cmake_minimum_required(VERSION 3.24)

project(Ardour VERSION 1.0.0 LANGUAGES C CXX OBJCXX ASM)

include(GNUInstallDirs)

set(ARDOUR_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
set(ARDOUR_BINARY_DIR "${CMAKE_BINARY_DIR}")
set(ARDOUR_PROGRAM_NAME "Ardour")

option(ARDOUR_ALLOW_MISSING_DEPS "Configure even if some optional pkg-config deps are missing" ON)
option(ARDOUR_BUILD_MAC_BUNDLE "Stage a macOS .app bundle in the build tree" ON)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(PkgConfig REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/libs/pbd")
include_directories(BEFORE "${CMAKE_SOURCE_DIR}/libs/tk")
include_directories(BEFORE "${CMAKE_SOURCE_DIR}/libs/tk/ydk")
include_directories(BEFORE "${CMAKE_SOURCE_DIR}/libs/tk/ydk-pixbuf")
include_directories(BEFORE "${CMAKE_SOURCE_DIR}/libs/tk/ztk")
include_directories(BEFORE "${CMAKE_SOURCE_DIR}/libs/temporal")

if(APPLE)
  add_compile_definitions(DARWIN)
add_link_options(-Xlinker -headerpad_max_install_names -Wl,-flat_namespace -Wl,-undefined,dynamic_lookup)
endif()

add_compile_definitions(PROGRAM_NAME=\"${ARDOUR_PROGRAM_NAME}\")

include(cmake/Dependencies.cmake)

set(ARDOUR_CONFIG_DIR "${CMAKE_BINARY_DIR}/cmake/config")
if(UNIX AND NOT WIN32)
  set(HAVE_UNISTD_H 1)
  set(HAVE_EXECINFO_H 1)
endif()
file(MAKE_DIRECTORY "${ARDOUR_CONFIG_DIR}/libs/pbd")
configure_file(
  "${CMAKE_SOURCE_DIR}/cmake/libpbd-config.h.in"
  "${ARDOUR_CONFIG_DIR}/libs/pbd/libpbd-config.h"
  @ONLY
)

set(ARDOUR_GENERATED_DIR "${CMAKE_BINARY_DIR}/cmake")
file(MAKE_DIRECTORY "${ARDOUR_GENERATED_DIR}")

execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/cmake/translate_wscripts.py"
          --repo-root "${CMAKE_SOURCE_DIR}"
          --out "${ARDOUR_GENERATED_DIR}/targets.cmake"
  RESULT_VARIABLE _ardour_translate_rc
  OUTPUT_VARIABLE _ardour_translate_stdout
  ERROR_VARIABLE _ardour_translate_stderr
)
if(NOT _ardour_translate_rc EQUAL 0)
  message(FATAL_ERROR "wscript translation failed:\n${_ardour_translate_stdout}\n${_ardour_translate_stderr}")
endif()

include("${ARDOUR_GENERATED_DIR}/targets.cmake")

if(TARGET libpbd)
  target_include_directories(libpbd PRIVATE "${ARDOUR_CONFIG_DIR}/libs/pbd")
endif()

set(EXECUTABLE "${ARDOUR_BUNDLE_NAME}")
set(BUNDLENAME "${ARDOUR_BUNDLE_NAME}")
set(IDBASE "${ARDOUR_BUNDLE_IDENTIFIER_BASE}")
set(IDSUFFIX "${ARDOUR_BUNDLE_NAME}")
set(VERSION "${ARDOUR_RELEASE_VERSION}")
set(INFOSTRING "${ARDOUR_BUNDLE_NAME} ${ARDOUR_RELEASE_VERSION}")
set(ENV "<key>LSEnvironment</key><dict><key>PATH</key><string>/usr/local/bin:/opt/bin:/usr/bin:/bin:/usr/sbin:/sbin</string><key>DYLIB_FALLBACK_LIBRARY_PATH</key><string>/usr/local/lib:/opt/lib</string><key>ARDOUR_BUNDLED</key><string>true</string></dict>")

configure_file(
  "${CMAKE_SOURCE_DIR}/tools/osx_packaging/Info.plist.in"
  "${ARDOUR_GENERATED_DIR}/Info.plist"
  @ONLY
)

if(APPLE AND ARDOUR_BUILD_MAC_BUNDLE)
  include(cmake/Bundle.cmake)
  ardour_stage_macos_bundle()
endif()
