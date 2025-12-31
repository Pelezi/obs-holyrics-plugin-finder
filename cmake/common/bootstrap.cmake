# Plugin bootstrap module

include_guard(GLOBAL)

# Early environment check for Windows Ninja builds
if(WIN32 AND CMAKE_GENERATOR MATCHES "Ninja" AND DEFINED OBS_PLUGIN_ARCHITECTURE)
  # Check if we have Visual Studio environment info
  if(DEFINED ENV{VSCMD_ARG_TGT_ARCH})
    set(_env_arch $ENV{VSCMD_ARG_TGT_ARCH})
    if(NOT _env_arch STREQUAL OBS_PLUGIN_ARCHITECTURE)
      message(FATAL_ERROR 
        "Architecture mismatch detected!\n"
        "Target architecture is ${OBS_PLUGIN_ARCHITECTURE}, but Visual Studio environment is set to ${_env_arch}.\n\n"
        "To fix this issue, choose ONE of the following options:\n\n"
        "Option 1 (Recommended): Use the Visual Studio generator which works from any command prompt:\n"
        "  cmake --preset windows-vs-x64\n\n"
        "Option 2: Run from the correct Native Tools Command Prompt:\n"
        "  1. Open the '${OBS_PLUGIN_ARCHITECTURE} Native Tools Command Prompt for VS 2022' from the Start Menu\n"
        "  2. Navigate to this directory\n"
        "  3. Delete the build directory if it exists: ${CMAKE_BINARY_DIR}\n"
        "  4. Run: cmake --preset windows-x64\n\n"
        "The 'windows-x64' Ninja preset requires the correct Visual Studio environment to be set up."
      )
    endif()
  elseif(NOT DEFINED ENV{VSCMD_ARG_HOST_ARCH})
    # No Visual Studio environment detected at all
    message(WARNING
      "Using Ninja generator but no Visual Studio environment detected.\n"
      "This may cause build issues if the compiler cannot be found.\n\n"
      "Consider using one of these options:\n"
      "Option 1 (Recommended): Use the Visual Studio generator:\n"
      "  cmake --preset windows-vs-x64\n\n"
      "Option 2: Run from the '${OBS_PLUGIN_ARCHITECTURE} Native Tools Command Prompt for VS 2022'\n"
    )
  endif()
endif()

# Set MSVC runtime library policy before project() call
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

# Set default MSVC runtime library for Windows builds
if(WIN32 AND NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC runtime library" FORCE)
endif()

# Map fallback configurations for optimized build configurations
# gersemi: off
set(
  CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO
    RelWithDebInfo
    Release
    MinSizeRel
    None
    ""
)
set(
  CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL
    MinSizeRel
    Release
    RelWithDebInfo
    None
    ""
)
set(
  CMAKE_MAP_IMPORTED_CONFIG_RELEASE
    Release
    RelWithDebInfo
    MinSizeRel
    None
    ""
)
# gersemi: on

# Prohibit in-source builds
if("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  message(
    FATAL_ERROR
    "In-source builds are not supported. "
    "Specify a build directory via 'cmake -S <SOURCE DIRECTORY> -B <BUILD_DIRECTORY>' instead."
  )
  file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/CMakeCache.txt" "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles")
endif()

# Add common module directories to default search path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/common")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/buildspec.json" buildspec)

string(JSON _name GET ${buildspec} name)
string(JSON _website GET ${buildspec} website)
string(JSON _author GET ${buildspec} author)
string(JSON _email GET ${buildspec} email)
string(JSON _version GET ${buildspec} version)
string(JSON _bundleId GET ${buildspec} platformConfig macos bundleId)

set(PLUGIN_AUTHOR ${_author})
set(PLUGIN_WEBSITE ${_website})
set(PLUGIN_EMAIL ${_email})
set(PLUGIN_VERSION ${_version})
set(MACOS_BUNDLEID ${_bundleId})

string(REPLACE "." ";" _version_canonical "${_version}")
list(GET _version_canonical 0 PLUGIN_VERSION_MAJOR)
list(GET _version_canonical 1 PLUGIN_VERSION_MINOR)
list(GET _version_canonical 2 PLUGIN_VERSION_PATCH)
unset(_version_canonical)

include(buildnumber)
include(osconfig)

# Allow selection of common build types via UI
if(NOT CMAKE_GENERATOR MATCHES "(Xcode|Visual Studio .+)")
  if(NOT CMAKE_BUILD_TYPE)
    set(
      CMAKE_BUILD_TYPE
      "RelWithDebInfo"
      CACHE STRING
      "OBS build type [Release, RelWithDebInfo, Debug, MinSizeRel]"
      FORCE
    )
    set_property(
      CACHE CMAKE_BUILD_TYPE
      PROPERTY STRINGS Release RelWithDebInfo Debug MinSizeRel
    )
  endif()
endif()

# Disable exports automatically going into the CMake package registry
set(CMAKE_EXPORT_PACKAGE_REGISTRY FALSE)
# Enable default inclusion of targets' source and binary directory
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
