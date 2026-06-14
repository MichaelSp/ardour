include_guard(GLOBAL)

function(ardour_stage_macos_bundle)
  if(NOT DEFINED ARDOUR_MAIN_APP_TARGET)
    message(FATAL_ERROR "ARDOUR_MAIN_APP_TARGET is not set")
  endif()
  if(NOT DEFINED ARDOUR_BUNDLE_NAME)
    set(ARDOUR_BUNDLE_NAME "Ardour")
  endif()

  set(bundle_name "Ardour")
  if(DEFINED ARDOUR_BUNDLE_NAME AND NOT ARDOUR_BUNDLE_NAME STREQUAL "")
    set(bundle_name "${ARDOUR_BUNDLE_NAME}")
  endif()

  set(bundle_dir "${CMAKE_BINARY_DIR}/${bundle_name}.app")
  set(contents_dir "${bundle_dir}/Contents")
  set(macos_dir "${contents_dir}/MacOS")
  set(resources_dir "${contents_dir}/Resources")
  set(resource_icons_dir "${resources_dir}/icons")
  set(resource_assets_dir "${resources_dir}/resources")
  set(lib_dir "${contents_dir}/lib")
  set(stamp_file "${CMAKE_BINARY_DIR}/${bundle_name}.app.stamp")

  add_custom_command(
    OUTPUT "${stamp_file}"
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${bundle_dir}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${macos_dir}" "${resources_dir}" "${resource_icons_dir}" "${resource_assets_dir}" "${lib_dir}"
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${ARDOUR_MAIN_APP_TARGET}>" "${macos_dir}/${bundle_name}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/cmake/Info.plist" "${contents_dir}/Info.plist"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/tools/osx_packaging/Ardour.icns" "${resources_dir}/appIcon.icns"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/tools/osx_packaging/typeArdour.icns" "${resources_dir}/typeIcon.icns"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/gtk2_ardour/ArdourMono.ttf" "${resources_dir}/ArdourMono.ttf"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/gtk2_ardour/ArdourSans.ttf" "${resources_dir}/ArdourSans.ttf"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/system_config" "${resources_dir}/system_config"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/share/templates" "${resources_dir}/templates"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/share/rdf" "${resources_dir}/rdf"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/share/midi_maps" "${resources_dir}/midi_maps"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/share/mcp" "${resources_dir}/mcp"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/share/osc" "${resources_dir}/osc"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/share/patchfiles" "${resources_dir}/patchfiles"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/share/plugin_metadata" "${resources_dir}/plugin_metadata"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/share/scripts" "${resources_dir}/scripts"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/share/web_surfaces" "${resources_dir}/web_surfaces"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/share/media" "${resources_dir}/media"
    DEPENDS "${ARDOUR_MAIN_APP_TARGET}"
  )

  file(GLOB ardour_bundle_icons CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/gtk2_ardour/icons/*.png")
  foreach(icon IN LISTS ardour_bundle_icons)
    get_filename_component(icon_name "${icon}" NAME)
    add_custom_command(
      OUTPUT "${stamp_file}"
      COMMAND "${CMAKE_COMMAND}" -E copy "${icon}" "${resource_icons_dir}/${icon_name}"
      APPEND
      DEPENDS "${icon}"
    )
  endforeach()

  file(GLOB ardour_bundle_cursor_dirs CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/gtk2_ardour/icons/cursor_*")
  foreach(cursor_dir IN LISTS ardour_bundle_cursor_dirs)
    get_filename_component(cursor_name "${cursor_dir}" NAME)
    add_custom_command(
      OUTPUT "${stamp_file}"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${cursor_dir}" "${resource_icons_dir}/${cursor_name}"
      APPEND
      DEPENDS "${cursor_dir}"
    )
  endforeach()

  file(GLOB ardour_bundle_resources CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/gtk2_ardour/resources/Ardour-*")
  foreach(resource IN LISTS ardour_bundle_resources)
    get_filename_component(resource_name "${resource}" NAME)
    add_custom_command(
      OUTPUT "${stamp_file}"
      COMMAND "${CMAKE_COMMAND}" -E copy "${resource}" "${resource_assets_dir}/${resource_name}"
      APPEND
      DEPENDS "${resource}"
    )
  endforeach()

  foreach(lib IN LISTS ARDOUR_BUNDLE_LIBRARY_TARGETS)
    add_custom_command(
      OUTPUT "${stamp_file}"
      COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${lib}>" "${lib_dir}/"
      APPEND
      DEPENDS "${lib}"
    )
  endforeach()

  foreach(exe IN LISTS ARDOUR_BUNDLE_RUNTIME_TARGETS)
    add_custom_command(
      OUTPUT "${stamp_file}"
      COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${exe}>" "${macos_dir}/"
      APPEND
      DEPENDS "${exe}"
    )
  endforeach()

  add_custom_target(${bundle_name}_bundle ALL DEPENDS "${stamp_file}")
  set(ARDOUR_BUNDLE_OUTPUT "${bundle_dir}" PARENT_SCOPE)
endfunction()
