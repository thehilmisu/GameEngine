# FindSDL2.cmake
# Locate SDL2 library
# This module defines:
# SDL2_FOUND - if false, do not try to link to SDL2
# SDL2_LIBRARIES - SDL2 library to link against
# SDL2_INCLUDE_DIRS - where to find SDL.h

# Look for SDL2 using pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_SDL2 QUIET sdl2)
endif()

# Search for the header
find_path(SDL2_INCLUDE_DIR
    NAMES SDL.h
    HINTS
        ${PC_SDL2_INCLUDE_DIRS}
        ENV SDL2DIR
        ${SDL2_DIR}
    PATH_SUFFIXES
        SDL2
        include/SDL2
        include
    PATHS
        /usr/local
        /usr
        /opt/local
        /opt
        /opt/homebrew
)

# Search for the library
find_library(SDL2_LIBRARY
    NAMES SDL2
    HINTS
        ${PC_SDL2_LIBRARY_DIRS}
        ENV SDL2DIR
        ${SDL2_DIR}
    PATH_SUFFIXES
        lib
        lib64
    PATHS
        /usr/local
        /usr
        /opt/local
        /opt
        /opt/homebrew
)

if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY)
    set(SDL2_FOUND TRUE)
    set(SDL2_LIBRARIES ${SDL2_LIBRARY})
    set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
    
    # Get additional link flags from pkg-config
    if(PC_SDL2_LINK_LIBRARIES)
        list(APPEND SDL2_LIBRARIES ${PC_SDL2_LINK_LIBRARIES})
    endif()
endif()

# Handle the QUIETLY and REQUIRED arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
    REQUIRED_VARS SDL2_LIBRARIES SDL2_INCLUDE_DIRS
    VERSION_VAR PC_SDL2_VERSION
)

mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARY)

