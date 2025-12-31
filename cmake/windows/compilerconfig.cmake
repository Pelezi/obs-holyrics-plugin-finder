# CMake Windows compiler configuration module

include_guard(GLOBAL)

include(compiler_common)

set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT ProgramDatabase)

# Set MSVC runtime library policy
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

# Set default runtime library to dynamic
if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Set Windows SDK version for non-Visual Studio generators
if(NOT CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
  if(CMAKE_SYSTEM_VERSION)
    set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION ${CMAKE_SYSTEM_VERSION})
  endif()
  
  # For non-Visual Studio generators, try to detect the Windows SDK version
  if(NOT CMAKE_GENERATOR MATCHES "Visual Studio")
    # Try to find WindowsSDKs directory
    file(GLOB _sdk_dirs 
      "C:/Program Files (x86)/Windows Kits/10/Include/10.0.*"
      "C:/Program Files/Windows Kits/10/Include/10.0.*"
    )
    
    if(_sdk_dirs)
      # Sort to get the latest version
      list(SORT _sdk_dirs COMPARE NATURAL ORDER DESCENDING)
      list(GET _sdk_dirs 0 _latest_sdk)
      
      # Extract version number from path
      string(REGEX MATCH "10\\.0\\.[0-9.]+" _sdk_version "${_latest_sdk}")
      
      if(_sdk_version)
        set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION ${_sdk_version})
        message(STATUS "Detected Windows SDK version: ${_sdk_version}")
      endif()
    endif()
  endif()
endif()

message(DEBUG "Current Windows API version: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION_MAXIMUM)
  message(DEBUG "Maximum Windows API version: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION_MAXIMUM}")
endif()

if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION VERSION_LESS 10.0.20348)
  message(
    FATAL_ERROR
    "OBS requires Windows 10 SDK version 10.0.20348.0 or more recent.\n"
    "Please download and install the most recent Windows platform SDK."
  )
endif()

set(_obs_msvc_c_options /MP /Zc:__cplusplus /Zc:preprocessor)
set(_obs_msvc_cpp_options /MP /Zc:__cplusplus /Zc:preprocessor)

if(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
  list(APPEND _obs_msvc_cpp_options /Zc:char8_t-)
endif()

add_compile_options(
  /W3
  /utf-8
  /Brepro
  /permissive-
  "$<$<COMPILE_LANG_AND_ID:C,MSVC>:${_obs_msvc_c_options}>"
  "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:${_obs_msvc_cpp_options}>"
  "$<$<COMPILE_LANG_AND_ID:C,Clang>:${_obs_clang_c_options}>"
  "$<$<COMPILE_LANG_AND_ID:CXX,Clang>:${_obs_clang_cxx_options}>"
  $<$<NOT:$<CONFIG:Debug>>:/Gy>
  $<$<NOT:$<CONFIG:Debug>>:/GL>
  $<$<NOT:$<CONFIG:Debug>>:/Oi>
  $<$<CONFIG:Debug>:/MDd>
  $<$<NOT:$<CONFIG:Debug>>:/MD>
)

add_link_options(
  $<$<NOT:$<CONFIG:Debug>>:/OPT:REF>
  $<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>
  $<$<NOT:$<CONFIG:Debug>>:/LTCG>
  $<$<NOT:$<CONFIG:Debug>>:/INCREMENTAL:NO>
  /DEBUG
  /Brepro
)

if(CMAKE_COMPILE_WARNING_AS_ERROR)
  add_link_options(/WX)
endif()
