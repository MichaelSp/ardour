#!/usr/bin/env python3
from __future__ import annotations

import argparse
import ast
import os
import re
import sys
import subprocess
from dataclasses import dataclass, field
from pathlib import Path


SKIP_TARGET_RE = re.compile(r"(?:^run-tests$|test|help2man|luadoc|load-save-session|canvas_test|profile)", re.I)
REVISION_RE = re.compile(r'([0-9][0-9]*)\.([0-9][0-9]*)\-?([pr][rc]e?[0-9]*)?-?([0-9][0-9]*)?(-g([a-f0-9]+))?')
WINDOWS_SOURCE_RE = re.compile(r'(^|/)(windows_[^/]+\.(?:cc|c|mm)|msvc/)', re.I)


@dataclass
class Target:
    source_dir: Path
    varname: str
    kind: str = ""
    name: str = ""
    output_name: str = ""
    sources: list[str] = field(default_factory=list)
    includes: list[str] = field(default_factory=list)
    export_includes: list[str] = field(default_factory=list)
    use: list[str] = field(default_factory=list)
    uselib: list[str] = field(default_factory=list)
    defines: list[str] = field(default_factory=list)
    install_path: str = ""
    app_target: bool = False

    def cmake_name(self) -> str:
        return self.name or self.output_name or self.varname


def eval_simple_expr(node: ast.AST, vars: dict[str, object], env: dict[str, str]) -> object | None:
    if isinstance(node, ast.Constant):
        return node.value
    if isinstance(node, ast.Name):
        return vars.get(node.id)
    if isinstance(node, ast.Subscript):
        if isinstance(node.value, ast.Attribute) and isinstance(node.value.value, ast.Name):
            if node.value.value.id == "bld" and node.value.attr == "env":
                key = eval_simple_expr(node.slice, vars, env)
                if isinstance(key, str):
                    return env.get(key)
    if isinstance(node, ast.List):
        return [eval_simple_expr(elt, vars, env) for elt in node.elts]
    if isinstance(node, ast.Tuple):
        return [eval_simple_expr(elt, vars, env) for elt in node.elts]
    if isinstance(node, ast.BinOp) and isinstance(node.op, ast.Add):
        left = eval_simple_expr(node.left, vars, env)
        right = eval_simple_expr(node.right, vars, env)
        if isinstance(left, list) and isinstance(right, list):
            return left + right
        if isinstance(left, str) or isinstance(right, str):
            return f"{left or ''}{right or ''}"
    if isinstance(node, ast.Call) and isinstance(node.func, ast.Name) and node.func.id == "list" and node.args:
        value = eval_simple_expr(node.args[0], vars, env)
        if isinstance(value, list):
            return list(value)
    if isinstance(node, ast.JoinedStr):
        parts: list[str] = []
        for part in node.values:
            if isinstance(part, ast.Constant) and isinstance(part.value, str):
                parts.append(part.value)
        return "".join(parts)
    return None


def collect_top_level_vars(tree: ast.AST) -> dict[str, object]:
    vars: dict[str, object] = {}
    for node in getattr(tree, "body", []):
        if not isinstance(node, ast.Assign):
            continue
        if len(node.targets) != 1 or not isinstance(node.targets[0], ast.Name):
            continue
        value = eval_simple_expr(node.value, vars, {})
        if value is not None:
            vars[node.targets[0].id] = value
    return vars


def as_list(value: object | None) -> list[str]:
    if value is None:
        return []
    if isinstance(value, list):
        out: list[str] = []
        for item in value:
            if isinstance(item, str) and item:
                out.append(item)
        return out
    if isinstance(value, str):
        if "\n" in value:
            tokens = []
            for line in value.splitlines():
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                tokens.extend(line.split())
            return tokens
        return value.split()
    return []


def resolve_path(repo_root: Path, base: Path, item: str) -> str:
    if item.startswith("$"):
        return item
    return str((repo_root / base / item).resolve())


def is_platform_source(relpath: str) -> bool:
    if os.name != "nt" and WINDOWS_SOURCE_RE.search(relpath):
        return False
    if sys.platform == "darwin" and ("/x11/" in relpath or "-x11." in relpath):
        return False
    return True


def parse_wscript(repo_root: Path, path: Path, env: dict[str, str]) -> list[Target]:
    source = path.read_text(encoding="utf-8")
    tree = ast.parse(source, filename=str(path))
    vars = collect_top_level_vars(tree)
    build = next((node for node in tree.body if isinstance(node, ast.FunctionDef) and node.name == "build"), None)
    if build is None:
        return []

    targets: list[Target] = []

    def new_target(varname: str, call: ast.Call, local_vars: dict[str, object]) -> Target | None:
        kind = ""
        if isinstance(call.func, ast.Attribute):
            kind = call.func.attr
        elif isinstance(call.func, ast.Name):
            features = ""
            for kw in call.keywords:
                if kw.arg == "features":
                    features = str(eval_simple_expr(kw.value, local_vars, env) or "")
            if "program" in features:
                kind = "program"
            elif "shlib" in features:
                kind = "shlib"
            elif "stlib" in features:
                kind = "stlib"

        rec = Target(source_dir=path.parent.relative_to(repo_root), varname=varname, kind=kind)
        for kw in call.keywords:
            value = eval_simple_expr(kw.value, local_vars, env)
            if kw.arg == "source":
                rec.sources.extend(as_list(value))
            elif kw.arg == "target":
                rec.output_name = str(value or "")
            elif kw.arg == "name":
                rec.name = str(value or "")
            elif kw.arg == "includes":
                rec.includes.extend(as_list(value))
            elif kw.arg == "export_includes":
                rec.export_includes.extend(as_list(value))
            elif kw.arg == "use":
                rec.use.extend(as_list(value))
            elif kw.arg == "uselib":
                rec.uselib.extend(as_list(value))
            elif kw.arg == "defines":
                rec.defines.extend(as_list(value))
            elif kw.arg == "install_path":
                rec.install_path = "" if value is None else str(value)
        return rec

    def apply_attr(rec: Target, attr: str, value: object, op: str) -> None:
        items = as_list(value)
        if attr == "source":
            if op == "+=":
                rec.sources.extend(items)
            else:
                rec.sources = items
        elif attr in {"includes", "export_includes", "use", "uselib", "defines"}:
            cur = getattr(rec, attr)
            if op == "+=":
                cur.extend(items)
            else:
                setattr(rec, attr, items)
        elif attr == "target":
            rec.output_name = str(value or "")
        elif attr == "name":
            rec.name = str(value or "")
        elif attr == "install_path":
            rec.install_path = "" if value is None else str(value)

    def walk(nodes: list[ast.stmt], state: dict[str, Target], local_vars: dict[str, object]) -> None:
        for node in nodes:
            if isinstance(node, ast.Assign) and len(node.targets) == 1:
                target = node.targets[0]
                if isinstance(target, ast.Name) and isinstance(node.value, ast.Call):
                    rec = new_target(target.id, node.value, local_vars)
                    if rec:
                        targets.append(rec)
                        state[target.id] = rec
                    continue
                if isinstance(target, ast.Name):
                    value = eval_simple_expr(node.value, local_vars, env)
                    if value is not None:
                        local_vars[target.id] = value
                    continue

                if isinstance(target, ast.Attribute) and isinstance(target.value, ast.Name):
                    rec = state.get(target.value.id)
                    if rec is None:
                        continue
                    value = eval_simple_expr(node.value, local_vars, env)
                    if value is not None:
                        apply_attr(rec, target.attr, value, "=")
                    continue

            if isinstance(node, ast.AugAssign) and isinstance(node.target, ast.Attribute) and isinstance(node.target.value, ast.Name):
                rec = state.get(node.target.value.id)
                if rec is None:
                    continue
                value = eval_simple_expr(node.value, local_vars, env)
                if value is not None:
                    apply_attr(rec, node.target.attr, value, "+=")
                continue

            if isinstance(node, ast.If):
                body_state = state.copy()
                walk(node.body, body_state, local_vars.copy())
                orelse_state = state.copy()
                walk(node.orelse, orelse_state, local_vars.copy())
                if node.orelse:
                    state.update(orelse_state)
                else:
                    state.update(body_state)
                continue

            if isinstance(node, (ast.For, ast.While, ast.With, ast.Try)):
                subnodes = []
                if isinstance(node, ast.For):
                    iterable = eval_simple_expr(node.iter, local_vars, env)
                    items = as_list(iterable)
                    if items:
                        for item in items:
                            iter_vars = local_vars.copy()
                            if isinstance(node.target, ast.Name):
                                iter_vars[node.target.id] = item
                            walk(node.body, state, iter_vars)
                    walk(node.orelse, state, local_vars.copy())
                    continue
                elif isinstance(node, ast.While):
                    subnodes = node.body + node.orelse
                elif isinstance(node, ast.With):
                    subnodes = node.body
                elif isinstance(node, ast.Try):
                    subnodes = node.body + node.orelse + node.finalbody
                    for handler in node.handlers:
                        subnodes.extend(handler.body)
                walk(subnodes, state.copy(), local_vars.copy())

    walk(build.body, {}, vars)
    return targets


def parse_revision(root: Path) -> tuple[str, str, str, str]:
    raw = subprocess.check_output(
        ["git", "describe", "--tags", "--always", "--dirty", "--abbrev=8"],
        cwd=root,
        text=True,
    ).strip()
    match = REVISION_RE.match(raw)
    if not match:
        return ("0", "0", "0", raw)

    major = match.group(1)
    minor = match.group(2)
    revcount = match.group(4) or "0"
    if revcount == "0":
        release_version = f"{major}.{minor}.0"
    else:
        commit = match.group(6) or ""
        release_version = f"{major}.{minor}.{revcount}{('.' + commit) if commit else ''}"
    return major, minor, revcount, release_version


def cmake_escape(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def classify_target(target: Target) -> str:
    if target.kind == "program":
        return "executable"
    return "library"


def should_skip(target: Target) -> bool:
    name = target.cmake_name()
    if target.kind == "":
        return True
    if name == "obj":
        return True
    if not target.sources:
        return True
    if SKIP_TARGET_RE.search(name):
        return True
    return False


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo-root", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args()

    repo_root = Path(args.repo_root).resolve()
    major_version, minor_version, revcount, release_version = parse_revision(repo_root)
    env = {
        "MAJOR": major_version,
        "MINOR": minor_version,
        "VERSION": release_version,
        "PROGRAM_VERSION": major_version,
    }
    targets: list[Target] = []
    for wscript in sorted(repo_root.rglob("wscript")):
        targets.extend(parse_wscript(repo_root, wscript, env))

    deduped: list[Target] = []
    seen_names: set[str] = set()
    for target in targets:
        if should_skip(target):
            continue
        name = target.cmake_name()
        if name in seen_names:
            continue
        seen_names.add(name)
        deduped.append(target)
    targets = deduped

    main_app = next(
        (
            t
            for t in targets
            if t.source_dir.name == "gtk2_ardour"
            and classify_target(t) == "executable"
            and any(Path(src).name == "main.cc" for src in t.sources)
        ),
        None,
    )
    bundle_libs = [t.cmake_name() for t in targets if classify_target(t) == "library" and t.kind == "shlib"]
    bundle_execs = [t.cmake_name() for t in targets if classify_target(t) == "executable" and t is not main_app]

    lines: list[str] = []
    lines.append("# Generated by cmake/translate_wscripts.py")
    lines.append(f"set(ARDOUR_MAJOR_VERSION {major_version})")
    lines.append(f"set(ARDOUR_MINOR_VERSION {minor_version})")
    lines.append(f"set(ARDOUR_REVCOUNT {revcount})")
    lines.append(f"set(ARDOUR_RELEASE_VERSION {release_version})")
    lines.append(f"set(ARDOUR_BUNDLE_NAME Ardour{major_version})")
    lines.append(f"set(ARDOUR_BUNDLE_IDENTIFIER_BASE org.ardour)")
    lines.append("")
    lines.append("set(ARDOUR_TARGETS")
    for t in targets:
        lines.append(f"  {t.cmake_name()}")
    lines.append(")")
    lines.append("")

    if main_app is not None:
        lines.append(f"set(ARDOUR_MAIN_APP_TARGET {main_app.cmake_name()})")
    else:
        lines.append("set(ARDOUR_MAIN_APP_TARGET \"\")")
    lines.append("set(ARDOUR_BUNDLE_LIBRARY_TARGETS")
    for name in bundle_libs:
        lines.append(f"  {name}")
    lines.append(")")
    lines.append("set(ARDOUR_BUNDLE_RUNTIME_TARGETS")
    for name in bundle_execs:
        lines.append(f"  {name}")
    lines.append(")")
    lines.append("")

    for t in targets:
        cmake_name = t.cmake_name()
        srcs = []
        for source in t.sources:
            resolved = resolve_path(repo_root, t.source_dir, source)
            if Path(resolved).exists() and is_platform_source(str(Path(resolved).relative_to(repo_root).as_posix())):
                srcs.append(resolved)
        if not srcs:
            continue
        lines.append(f"add_{'executable' if classify_target(t) == 'executable' else 'library'}({cmake_name}")
        if classify_target(t) == "library":
            lib_kind = "STATIC"
            if t.kind == "shlib":
                lib_kind = "SHARED"
            lines[-1] = f"add_library({cmake_name} {lib_kind}"
        for src in srcs:
            lines.append(f"  \"{cmake_escape(src)}\"")
        lines.append(")")
        if t.output_name and t.output_name != cmake_name:
            lines.append(f"set_target_properties({cmake_name} PROPERTIES OUTPUT_NAME \"{cmake_escape(t.output_name)}\")")
        if t.export_includes or t.includes:
            includes = []
            for item in t.export_includes + t.includes:
                if item == ".":
                    includes.append(f"${{ARDOUR_SOURCE_DIR}}/{t.source_dir.as_posix()}")
                elif item.startswith(".."):
                    includes.append(f"${{ARDOUR_SOURCE_DIR}}/{(repo_root / t.source_dir / item).resolve().relative_to(repo_root).as_posix()}")
                else:
                    includes.append(f"${{ARDOUR_SOURCE_DIR}}/{(repo_root / t.source_dir / item).resolve().relative_to(repo_root).as_posix()}")
            if includes:
                lines.append(f"target_include_directories({cmake_name} PUBLIC")
                for inc in dict.fromkeys(includes):
                    lines.append(f"  \"{cmake_escape(inc)}\"")
                lines.append(")")
        if t.defines:
            lines.append(f"target_compile_definitions({cmake_name} PRIVATE")
            for define in dict.fromkeys(t.defines):
                lines.append(f"  \"{cmake_escape(define)}\"")
            lines.append(")")
        target_names = {x.cmake_name() for x in targets}
        internal_deps = [dep for dep in t.use if dep in target_names]
        external_deps: list[str] = []
        for dep in t.uselib:
            if not dep:
                continue
            if dep in {"OSX", "AUDIOUNITS", "COREAUDIO"}:
                external_deps.append("Ardour::AppleFrameworks")
            elif dep == "AUBIO4":
                external_deps.append("PkgConfig::AUBIO")
            elif dep == "BOOST":
                continue
            elif f"lib{dep.lower()}" in target_names:
                continue
            elif dep.startswith("LIB") and f"lib{dep[3:].lower()}" in target_names:
                continue
            else:
                external_deps.append(f"PkgConfig::{dep}")
        external_deps = list(dict.fromkeys(external_deps))
        if internal_deps or external_deps:
            include_deps = [dep for dep in external_deps if dep.startswith("PkgConfig::")]
            if include_deps:
                lines.append(f"target_include_directories({cmake_name} PUBLIC")
                for dep in include_deps:
                    lines.append(f"  \"$<TARGET_PROPERTY:{dep},INTERFACE_INCLUDE_DIRECTORIES>\"")
                lines.append(")")
            lines.append(f"target_link_libraries({cmake_name} PRIVATE")
            for dep in internal_deps:
                lines.append(f"  {dep}")
            for dep in external_deps:
                lines.append(f"  {dep}")
            lines.append(")")
        lines.append("")

    Path(args.out).write_text("\n".join(lines), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
