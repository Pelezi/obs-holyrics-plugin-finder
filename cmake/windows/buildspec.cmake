# CMake Windows build dependencies module

include_guard(GLOBAL)

include(buildspec_common)

# _check_dependencies_windows: Set up Windows slice for _check_dependencies
function(_check_dependencies_windows)
  # Set architecture - Try multiple sources in order of preference:
  # 1. OBS_PLUGIN_ARCHITECTURE (from presets - custom variable for Ninja builds)
  # 2. CMAKE_GENERATOR_PLATFORM (from presets or command line -A for VS generators)
  # 3. CMAKE_VS_PLATFORM_NAME (for Visual Studio generators)
  # 4. Environment variable VSCMD_ARG_TGT_ARCH (from VS Developer Command Prompt)
  # 5. CMAKE_SIZEOF_VOID_P (detected from compiler - least reliable for Ninja before first configure)
  
  if(OBS_PLUGIN_ARCHITECTURE)
    set(arch ${OBS_PLUGIN_ARCHITECTURE})
    message(DEBUG "Architecture from OBS_PLUGIN_ARCHITECTURE: ${arch}")
  elseif(CMAKE_GENERATOR_PLATFORM)
    set(arch ${CMAKE_GENERATOR_PLATFORM})
    message(DEBUG "Architecture from CMAKE_GENERATOR_PLATFORM: ${arch}")
  elseif(DEFINED ENV{VSCMD_ARG_TGT_ARCH})
    set(arch $ENV{VSCMD_ARG_TGT_ARCH})
    message(DEBUG "Architecture from VSCMD_ARG_TGT_ARCH: ${arch}")
  elseif(CMAKE_VS_PLATFORM_NAME)
    set(arch ${CMAKE_VS_PLATFORM_NAME})
    message(DEBUG "Architecture from CMAKE_VS_PLATFORM_NAME: ${arch}")
  else()
    # For non-Visual Studio generators (like Ninja), detect architecture from pointer size
    # This only works after the compiler has been detected
    if(CMAKE_SIZEOF_VOID_P)
      if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(arch x64)
      else()
        set(arch x86)
      endif()
      message(DEBUG "Architecture from CMAKE_SIZEOF_VOID_P: ${arch}")
    else()
      # CMAKE_SIZEOF_VOID_P not set yet - assume x64 as default for modern systems
      set(arch x64)
      message(DEBUG "Architecture defaulting to x64 (CMAKE_SIZEOF_VOID_P not yet set)")
    endif()
  endif()
  
  message(STATUS "Detected architecture: ${arch}")
  
  # Post-compiler detection check: verify the compiler matches the target architecture
  # This is a safety check in case the early check in bootstrap.cmake was bypassed
  if(CMAKE_SIZEOF_VOID_P)
    set(compiler_arch "unknown")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(compiler_arch "x64")
    else()
      set(compiler_arch "x86")
    endif()
    
    if(NOT compiler_arch STREQUAL arch)
      message(FATAL_ERROR 
        "Architecture mismatch detected!\n"
        "Target architecture is ${arch}, but compiler is ${compiler_arch}.\n\n"
        "To fix this issue, choose ONE of the following options:\n\n"
        "Option 1 (Recommended): Use the Visual Studio generator which works from any command prompt:\n"
        "  1. Delete the build directory: ${CMAKE_BINARY_DIR}\n"
        "  2. Run: cmake --preset windows-vs-x64\n\n"
        "Option 2: Run from the correct Native Tools Command Prompt:\n"
        "  1. Open the '${arch} Native Tools Command Prompt for VS 2022' from the Start Menu\n"
        "  2. Navigate to this directory\n"
        "  3. Delete the build directory: ${CMAKE_BINARY_DIR}\n"
        "  4. Run: cmake --preset windows-x64\n\n"
        "The Visual Studio generator (Option 1) is easier because it works from any command prompt."
      )
    endif()
  endif()
  
  set(platform windows-${arch})

  set(dependencies_dir "${CMAKE_CURRENT_SOURCE_DIR}/.deps")
  set(prebuilt_filename "windows-deps-VERSION-ARCH-REVISION.zip")
  set(prebuilt_destination "obs-deps-VERSION-ARCH")
  set(qt6_filename "windows-deps-qt6-VERSION-ARCH-REVISION.zip")
  set(qt6_destination "obs-deps-qt6-VERSION-ARCH")
  set(obs-studio_filename "VERSION.zip")
  set(obs-studio_destination "obs-studio-VERSION")
  set(dependencies_list prebuilt qt6 obs-studio)

  _check_dependencies()
endfunction()

_check_dependencies_windows()
