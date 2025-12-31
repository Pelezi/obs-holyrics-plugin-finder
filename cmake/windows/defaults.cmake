# CMake Windows defaults module

include_guard(GLOBAL)

# Enable find_package targets to become globally available targets
set(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL TRUE)

include(buildspec)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(
    CMAKE_INSTALL_PREFIX
    "$ENV{ALLUSERSPROFILE}/obs-studio/plugins"
    CACHE STRING
    "Default plugin installation directory"
    FORCE
  )
endif()

# Try to find libobs from system installation first
find_package(libobs QUIET)

# If not found, look for OBS Studio installation and create import targets
if(NOT TARGET OBS::libobs)
  # Set architecture for paths
  if(OBS_PLUGIN_ARCHITECTURE)
    set(_arch ${OBS_PLUGIN_ARCHITECTURE})
  elseif(CMAKE_VS_PLATFORM_NAME)
    set(_arch ${CMAKE_VS_PLATFORM_NAME})
  elseif(CMAKE_GENERATOR_PLATFORM)
    set(_arch ${CMAKE_GENERATOR_PLATFORM})
  else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(_arch x64)
    else()
      set(_arch x86)
    endif()
  endif()

  # Find OBS Studio installation
  set(_obs_install_dir "C:/Program Files/obs-studio")
  
  if(EXISTS "${_obs_install_dir}")
    # Find OBS Studio source directory for headers
    file(GLOB _obs_source_dirs "${CMAKE_CURRENT_SOURCE_DIR}/.deps/obs-studio-*")
    if(_obs_source_dirs)
      list(GET _obs_source_dirs 0 _obs_source_dir)
      set(obs_INCLUDE_DIR "${_obs_source_dir}/libobs")
      
      # Set bin directory based on architecture
      if(_arch STREQUAL "x64")
        set(_obs_bin_dir "${_obs_install_dir}/bin/64bit")
        set(_arch_flag "X64")
      else()
        set(_obs_bin_dir "${_obs_install_dir}/bin/32bit")
        set(_arch_flag "X86")
      endif()
      
      # Find obs.dll
      find_file(
        obs_DLL
        NAMES obs.dll
        PATHS "${_obs_bin_dir}"
        NO_DEFAULT_PATH
      )
      
      # Try to find obs.lib
      find_file(
        obs_LIB
        NAMES obs.lib
        PATHS "${_obs_bin_dir}"
        NO_DEFAULT_PATH
      )
      
      # If .lib not found, generate it using dumpbin and lib
      if(NOT obs_LIB AND obs_DLL)
        set(_generated_lib_dir "${CMAKE_CURRENT_BINARY_DIR}/generated_libs")
        file(MAKE_DIRECTORY "${_generated_lib_dir}")
        set(obs_LIB "${_generated_lib_dir}/obs.lib")
        
        if(NOT EXISTS "${obs_LIB}")
          message(STATUS "Generating import library for obs.dll...")
          
          find_program(DUMPBIN_EXECUTABLE dumpbin)
          find_program(LIB_EXECUTABLE lib)
          
          if(DUMPBIN_EXECUTABLE AND LIB_EXECUTABLE)
            set(_def_file "${_generated_lib_dir}/obs.def")
            set(_exports_file "${_generated_lib_dir}/obs_exports.txt")
            
            # Export symbols using dumpbin
            execute_process(
              COMMAND "${DUMPBIN_EXECUTABLE}" /EXPORTS "${obs_DLL}"
              OUTPUT_FILE "${_exports_file}"
              ERROR_VARIABLE _dumpbin_error
              RESULT_VARIABLE _dumpbin_result
            )
            
            if(_dumpbin_result EQUAL 0 AND EXISTS "${_exports_file}")
              # Parse the exports file and create a .def file
              file(STRINGS "${_exports_file}" _export_lines)
              
              set(_def_content "LIBRARY obs\nEXPORTS\n")
              set(_in_exports_section FALSE)
              set(_export_count 0)
              
              foreach(_line IN LISTS _export_lines)
                # Look for the exports section header
                if(_line MATCHES "ordinal hint")
                  set(_in_exports_section TRUE)
                  continue()
                endif()
                
                # Skip until we find the exports section
                if(NOT _in_exports_section)
                  continue()
                endif()
                
                # Stop if we hit the summary section or blank line after exports
                if(_line MATCHES "^[ \t]*Summary" OR (_export_count GREATER 0 AND _line MATCHES "^[ \t]*$"))
                  break()
                endif()
                
                # Parse export lines - handle multiple formats
                # Format: ordinal hint RVA name
                # The name can be at various positions, try to extract the last non-whitespace token
                if(_line MATCHES "^[ \t]*[0-9]+[ \t]+[0-9A-Fa-f]+[ \t]+[0-9A-Fa-f]+[ \t]+(.+)$")
                  set(_export_name "${CMAKE_MATCH_1}")
                  # Remove any trailing whitespace or parenthetical info
                  string(REGEX REPLACE "[ \t]+.*$" "" _export_name "${_export_name}")
                  string(STRIP "${_export_name}" _export_name)
                  
                  # Skip certain patterns (imports, data exports, decorators)
                  if(NOT _export_name MATCHES "^\\[" AND 
                     NOT _export_name STREQUAL "" AND
                     NOT _export_name MATCHES "^[0-9]")
                    string(APPEND _def_content "    ${_export_name}\n")
                    math(EXPR _export_count "${_export_count} + 1")
                  endif()
                endif()
              endforeach()
              
              if(_export_count GREATER 0)
                file(WRITE "${_def_file}" "${_def_content}")
                message(STATUS "Generated .def file with ${_export_count} exports")
                
                # Generate import library from .def file
                execute_process(
                  COMMAND "${LIB_EXECUTABLE}" /DEF:${_def_file} /OUT:${obs_LIB} /MACHINE:${_arch_flag}
                  WORKING_DIRECTORY "${_generated_lib_dir}"
                  OUTPUT_VARIABLE _lib_output
                  ERROR_VARIABLE _lib_error
                  RESULT_VARIABLE _lib_result
                )
                
                if(_lib_result EQUAL 0 AND EXISTS "${obs_LIB}")
                  message(STATUS "Successfully generated import library: ${obs_LIB}")
                else()
                  message(WARNING "Failed to generate import library from .def file\nOutput: ${_lib_output}\nError: ${_lib_error}")
                  set(obs_LIB "")
                endif()
              else()
                message(WARNING "No exports found in dumpbin output - check ${_exports_file}")
                set(obs_LIB "")
              endif()
            else()
              message(WARNING "Failed to dump exports from obs.dll\nResult: ${_dumpbin_result}\nError: ${_dumpbin_error}")
              set(obs_LIB "")
            endif()
          else()
            message(WARNING "dumpbin or lib.exe not found - cannot generate import library")
            set(obs_LIB "")
          endif()
        endif()
      endif()
      
      if(obs_DLL AND obs_LIB AND obs_INCLUDE_DIR)
        # Create imported DLL target for libobs
        add_library(OBS::libobs SHARED IMPORTED)
        set_target_properties(
          OBS::libobs
          PROPERTIES
            IMPORTED_LOCATION "${obs_DLL}"
            IMPORTED_IMPLIB "${obs_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${obs_INCLUDE_DIR}"
        )
        message(STATUS "Found libobs DLL: ${obs_DLL}")
        message(STATUS "  Import library: ${obs_LIB}")
        message(STATUS "  Headers: ${obs_INCLUDE_DIR}")
      else()
        message(FATAL_ERROR 
          "Could not find OBS Studio installation or headers.\n"
          "OBS Installation: ${_obs_install_dir}\n"
          "OBS DLL: ${obs_DLL}\n"
          "OBS LIB: ${obs_LIB}\n"
          "OBS Headers: ${obs_INCLUDE_DIR}\n"
          "Please ensure:\n"
          "  1. OBS Studio is installed, and\n"
          "  2. OBS Studio sources are downloaded (run CMake again)\n"
        )
      endif()

      # Handle obs-frontend-api if needed
      if(ENABLE_FRONTEND_API)
        # Try both possible paths for frontend API
        set(obs-frontend-api_INCLUDE_DIR "${_obs_source_dir}/frontend/api")
        if(NOT EXISTS "${obs-frontend-api_INCLUDE_DIR}")
          set(obs-frontend-api_INCLUDE_DIR "${_obs_source_dir}/UI/obs-frontend-api")
        endif()
        
        find_file(
          obs-frontend-api_DLL
          NAMES obs-frontend-api.dll
          PATHS "${_obs_bin_dir}"
          NO_DEFAULT_PATH
        )
        
        find_file(
          obs-frontend-api_LIB
          NAMES obs-frontend-api.lib
          PATHS "${_obs_bin_dir}"
          NO_DEFAULT_PATH
        )
        
        # If .lib not found, generate it
        if(NOT obs-frontend-api_LIB AND obs-frontend-api_DLL)
          set(_generated_lib_dir "${CMAKE_CURRENT_BINARY_DIR}/generated_libs")
          set(obs-frontend-api_LIB "${_generated_lib_dir}/obs-frontend-api.lib")
          
          if(NOT EXISTS "${obs-frontend-api_LIB}")
            message(STATUS "Generating import library for obs-frontend-api.dll...")
            
            find_program(DUMPBIN_EXECUTABLE dumpbin)
            find_program(LIB_EXECUTABLE lib)
            
            if(DUMPBIN_EXECUTABLE AND LIB_EXECUTABLE)
              set(_def_file "${_generated_lib_dir}/obs-frontend-api.def")
              set(_exports_file "${_generated_lib_dir}/obs-frontend-api_exports.txt")
              
              execute_process(
                COMMAND "${DUMPBIN_EXECUTABLE}" /EXPORTS "${obs-frontend-api_DLL}"
                OUTPUT_FILE "${_exports_file}"
                ERROR_VARIABLE _dumpbin_error
                RESULT_VARIABLE _dumpbin_result
              )
              
              if(_dumpbin_result EQUAL 0 AND EXISTS "${_exports_file}")
                file(STRINGS "${_exports_file}" _export_lines)
                
                set(_def_content "LIBRARY obs-frontend-api\nEXPORTS\n")
                set(_in_exports_section FALSE)
                set(_export_count 0)
                
                foreach(_line IN LISTS _export_lines)
                  if(_line MATCHES "ordinal hint")
                    set(_in_exports_section TRUE)
                    continue()
                  endif()
                  
                  if(NOT _in_exports_section)
                    continue()
                  endif()
                  
                  if(_line MATCHES "^[ \t]*Summary" OR (_export_count GREATER 0 AND _line MATCHES "^[ \t]*$"))
                    break()
                  endif()
                  
                  if(_line MATCHES "^[ \t]*[0-9]+[ \t]+[0-9A-Fa-f]+[ \t]+[0-9A-Fa-f]+[ \t]+(.+)$")
                    set(_export_name "${CMAKE_MATCH_1}")
                    string(REGEX REPLACE "[ \t]+.*$" "" _export_name "${_export_name}")
                    string(STRIP "${_export_name}" _export_name)
                    
                    if(NOT _export_name MATCHES "^\\[" AND 
                       NOT _export_name STREQUAL "" AND
                       NOT _export_name MATCHES "^[0-9]")
                      string(APPEND _def_content "    ${_export_name}\n")
                      math(EXPR _export_count "${_export_count} + 1")
                    endif()
                  endif()
                endforeach()
                
                if(_export_count GREATER 0)
                  file(WRITE "${_def_file}" "${_def_content}")
                  message(STATUS "Generated .def file with ${_export_count} exports")
                  
                  execute_process(
                    COMMAND "${LIB_EXECUTABLE}" /DEF:${_def_file} /OUT:${obs-frontend-api_LIB} /MACHINE:${_arch_flag}
                    WORKING_DIRECTORY "${_generated_lib_dir}"
                    OUTPUT_VARIABLE _lib_output
                    ERROR_VARIABLE _lib_error
                    RESULT_VARIABLE _lib_result
                  )
                  
                  if(_lib_result EQUAL 0 AND EXISTS "${obs-frontend-api_LIB}")
                    message(STATUS "Successfully generated import library: ${obs-frontend-api_LIB}")
                  else()
                    message(WARNING "Failed to generate import library for obs-frontend-api\nOutput: ${_lib_output}\nError: ${_lib_error}")
                    set(obs-frontend-api_LIB "")
                  endif()
                else()
                  message(WARNING "No exports found in dumpbin output for obs-frontend-api")
                  set(obs-frontend-api_LIB "")
                endif()
              else()
                message(WARNING "Failed to dump exports from obs-frontend-api.dll\nResult: ${_dumpbin_result}\nError: ${_dumpbin_error}")
                set(obs-frontend-api_LIB "")
              endif()
            else()
              message(WARNING "dumpbin or lib.exe not found")
              set(obs-frontend-api_LIB "")
            endif()
          endif()
        endif()

        if(obs-frontend-api_DLL AND obs-frontend-api_LIB AND EXISTS "${obs-frontend-api_INCLUDE_DIR}")
          if(NOT TARGET OBS::obs-frontend-api)
            add_library(OBS::obs-frontend-api SHARED IMPORTED)
            set_target_properties(
              OBS::obs-frontend-api
              PROPERTIES
                IMPORTED_LOCATION "${obs-frontend-api_DLL}"
                IMPORTED_IMPLIB "${obs-frontend-api_LIB}"
                INTERFACE_INCLUDE_DIRECTORIES "${obs-frontend-api_INCLUDE_DIR}"
            )
            message(STATUS "Found obs-frontend-api DLL: ${obs-frontend-api_DLL}")
            message(STATUS "  Import library: ${obs-frontend-api_LIB}")
            message(STATUS "  Headers: ${obs-frontend-api_INCLUDE_DIR}")
          endif()
        endif()
      endif()
    else()
      message(FATAL_ERROR 
        "Could not find OBS Studio sources.\n"
        "Please run CMake configuration to download dependencies first.\n"
      )
    endif()
  else()
    message(FATAL_ERROR 
      "OBS Studio is not installed at: ${_obs_install_dir}\n"
      "Please install OBS Studio before building plugins.\n"
    )
  endif()

  # Override find_package to prevent re-searching for libobs and obs-frontend-api
  macro(find_package)
    if(NOT "${ARGV0}" STREQUAL libobs AND NOT "${ARGV0}" STREQUAL obs-frontend-api)
      _find_package(${ARGV})
    endif()
  endmacro()
endif()
