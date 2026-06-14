include_guard(GLOBAL)

find_path(ARDOUR_BOOST_INCLUDE_DIR boost/tokenizer.hpp
  PATHS /opt/homebrew/include /usr/local/include
)
if(ARDOUR_BOOST_INCLUDE_DIR)
  include_directories(SYSTEM "${ARDOUR_BOOST_INCLUDE_DIR}")
endif()

function(ardour_import_pkg prefix module)
  set(options REQUIRED OPTIONAL)
  cmake_parse_arguments(ARG "${options}" "" "" ${ARGN})

  if(ARG_REQUIRED AND NOT ARDOUR_ALLOW_MISSING_DEPS)
    pkg_check_modules(${prefix} REQUIRED IMPORTED_TARGET ${module})
    return()
  endif()

  pkg_check_modules(${prefix} QUIET IMPORTED_TARGET ${module})
  if(NOT TARGET PkgConfig::${prefix})
    add_library(PkgConfig::${prefix} INTERFACE IMPORTED)
    if(ARG_REQUIRED AND NOT ARDOUR_ALLOW_MISSING_DEPS)
      message(FATAL_ERROR "Missing required pkg-config dependency: ${module}")
    endif()
  endif()
endfunction()

function(ardour_import_pkg_any prefix)
  set(options REQUIRED OPTIONAL)
  cmake_parse_arguments(ARG "${options}" "" "" ${ARGN})

  foreach(module IN LISTS ARG_UNPARSED_ARGUMENTS)
    pkg_check_modules(${prefix} QUIET IMPORTED_TARGET ${module})
    if(TARGET PkgConfig::${prefix})
      return()
    endif()
  endforeach()

  if(NOT TARGET PkgConfig::${prefix})
    add_library(PkgConfig::${prefix} INTERFACE IMPORTED)
    if(ARG_REQUIRED AND NOT ARDOUR_ALLOW_MISSING_DEPS)
      message(FATAL_ERROR "Missing required pkg-config dependency: ${ARG_UNPARSED_ARGUMENTS}")
    endif()
  endif()
endfunction()

ardour_import_pkg(GLIB glib-2.0 REQUIRED)
ardour_import_pkg(GTHREAD gthread-2.0 REQUIRED)
ardour_import_pkg_any(GLIBMM glibmm-2.68 glibmm-2.4 REQUIRED)
ardour_import_pkg_any(GIOMM giomm-2.68 giomm-2.4 REQUIRED)
ardour_import_pkg_any(SIGCPP sigc++-3.0 sigc++-2.0 REQUIRED)
ardour_import_pkg(UUID uuid OPTIONAL)
ardour_import_pkg(JACK jack OPTIONAL)
ardour_import_pkg(PULSEAUDIO libpulse OPTIONAL)
ardour_import_pkg(USB libusb-1.0 OPTIONAL)
ardour_import_pkg(GIO gio-2.0 OPTIONAL)
ardour_import_pkg(GOBJECT gobject-2.0 OPTIONAL)
ardour_import_pkg(QMDSP qm-dsp OPTIONAL)
ardour_import_pkg(CAIROMM cairomm-1.0 OPTIONAL)
ardour_import_pkg(PANGOMM pangomm-2.48 OPTIONAL)
ardour_import_pkg(PANGOCAIRO pangocairo OPTIONAL)
ardour_import_pkg(PANGO pango OPTIONAL)
ardour_import_pkg(CAIRO cairo OPTIONAL)
ardour_import_pkg(LIBMM libmm OPTIONAL)
ardour_import_pkg(GMODULE gmodule-2.0 OPTIONAL)
ardour_import_pkg(GTKMM gtkmm-2.4 OPTIONAL)
ardour_import_pkg(READLINE readline OPTIONAL)
ardour_import_pkg(SNDFILE sndfile REQUIRED)
ardour_import_pkg(CURL libcurl REQUIRED)
ardour_import_pkg(ARCHIVE libarchive REQUIRED)
ardour_import_pkg(LO liblo REQUIRED)
ardour_import_pkg(TAGLIB taglib REQUIRED)
ardour_import_pkg(VAMPSDK vamp-sdk REQUIRED)
ardour_import_pkg(VAMPHOSTSDK vamp-hostsdk REQUIRED)
ardour_import_pkg(RUBBERBAND rubberband REQUIRED)
ardour_import_pkg(AUBIO aubio REQUIRED)
ardour_import_pkg(XML libxml-2.0 REQUIRED)
ardour_import_pkg(LRDF lrdf OPTIONAL)
ardour_import_pkg(SAMPLERATE samplerate REQUIRED)
ardour_import_pkg(LV2 lv2 REQUIRED)
ardour_import_pkg(SERD serd-0 REQUIRED)
ardour_import_pkg(SORD sord-0 REQUIRED)
ardour_import_pkg(SRATOM sratom-0 REQUIRED)
ardour_import_pkg(LILV lilv-0 REQUIRED)
ardour_import_pkg(FFTW3F fftw3f REQUIRED)
ardour_import_pkg(FLAC flac REQUIRED)
ardour_import_pkg(OGG ogg REQUIRED)
ardour_import_pkg(LIBPNG libpng OPTIONAL)
ardour_import_pkg(LIBJPEG libjpeg OPTIONAL)
ardour_import_pkg(DBUS dbus-1 OPTIONAL)
ardour_import_pkg(CWIID cwiid OPTIONAL)
ardour_import_pkg(UDEV libudev OPTIONAL)
ardour_import_pkg(LIBFLUIDSYNTH fluidsynth OPTIONAL)
ardour_import_pkg(SOUNDTOUCH soundtouch OPTIONAL)
ardour_import_pkg(OPENSSL openssl OPTIONAL)
ardour_import_pkg(WEBSOCKETS websockets OPTIONAL)
ardour_import_pkg(LIBAAF libaaf OPTIONAL)
ardour_import_pkg(FONTCONFIG fontconfig REQUIRED)
ardour_import_pkg(PANGOFT2 pangoft2 OPTIONAL)
ardour_import_pkg(X11 x11 OPTIONAL)
ardour_import_pkg(XEXT xext OPTIONAL)
ardour_import_pkg(XINERAMA xinerama OPTIONAL)
ardour_import_pkg(RANDR xrandr OPTIONAL)
ardour_import_pkg(ALSA alsa OPTIONAL)

foreach(dummy IN ITEMS DL GDI32 OLE COREAUDIO)
  if(NOT TARGET PkgConfig::${dummy})
    add_library(PkgConfig::${dummy} INTERFACE IMPORTED)
  endif()
endforeach()

foreach(dummy IN ITEMS GIO-WINDOWS)
  if(NOT TARGET PkgConfig::${dummy})
    add_library(PkgConfig::${dummy} INTERFACE IMPORTED)
  endif()
endforeach()

foreach(dummy IN ITEMS GIO-UNIX AUBIO4)
  if(NOT TARGET PkgConfig::${dummy})
    add_library(PkgConfig::${dummy} INTERFACE IMPORTED)
  endif()
endforeach()

add_library(Ardour::AppleFrameworks INTERFACE IMPORTED)
if(APPLE)
  target_link_libraries(Ardour::AppleFrameworks INTERFACE
    "-framework AppKit"
    "-framework CoreAudio"
    "-framework CoreAudioKit"
    "-framework CoreFoundation"
    "-framework CoreServices"
    "-framework AudioToolbox"
    "-framework AudioUnit"
    "-framework Cocoa"
    "-framework Accelerate"
  )
endif()

set(ARDOUR_PKG_TARGETS
  PkgConfig::GLIB
  PkgConfig::GTHREAD
  PkgConfig::GLIBMM
  PkgConfig::GIOMM
  PkgConfig::SIGCPP
  PkgConfig::SNDFILE
  PkgConfig::CURL
  PkgConfig::ARCHIVE
  PkgConfig::LO
  PkgConfig::TAGLIB
  PkgConfig::VAMPSDK
  PkgConfig::VAMPHOSTSDK
  PkgConfig::RUBBERBAND
  PkgConfig::AUBIO
  PkgConfig::XML
  PkgConfig::LRDF
  PkgConfig::SAMPLERATE
  PkgConfig::LV2
  PkgConfig::SERD
  PkgConfig::SORD
  PkgConfig::SRATOM
  PkgConfig::LILV
  PkgConfig::FFTW3F
  PkgConfig::FLAC
  PkgConfig::OGG
  PkgConfig::FONTCONFIG
  PkgConfig::PANGOFT2
  PkgConfig::X11
  PkgConfig::ALSA
  PkgConfig::PULSE
  PkgConfig::LIBUSB
  Ardour::AppleFrameworks
)
