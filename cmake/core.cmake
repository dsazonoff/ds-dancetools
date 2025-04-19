include_guard(GLOBAL)

# Trace a list of variables
# If variable doesn't exist it will be treated as a text
# If variable is a single value it will be shown as VAR = VALUE
# If variable is an array it will be shown as a list VAR = [ val1, val2, ... ]
# Usage:  echo(<var1> <var2>...)
function(echo)
    # Long prefixes are required to avoid vars name conflicts
    foreach (ds_echo_arg ${ARGV})
        set(ds_echo_name ${ds_echo_arg})
        set(ds_echo_value ${${ds_echo_arg}})
        list(LENGTH ds_echo_value ds_echo_count)
        if (NOT DEFINED ${ds_echo_name})
            message(STATUS "${ds_echo_name}")
        elseif (${ds_echo_count} LESS 2)
            message(STATUS "${ds_echo_name} = ${ds_echo_value}")
        else ()
            string(JOIN ", " ds_echo_text ${ds_echo_value})
            set(ds_echo_text "[ ${ds_echo_text} ]")
            message(STATUS "${ds_echo_name} = ${ds_echo_text}")
        endif ()
    endforeach ()
endfunction()

set(ds_project_root "${CMAKE_SOURCE_DIR}" CACHE STRING "")
set(ds_cmake_root "${CMAKE_CURRENT_LIST_DIR}" CACHE STRING "")
list(APPEND CMAKE_MODULE_PATH "${ds_cmake_root}")

include(variables)
include(targets)
include(helpers)
if (${DS_CLANGFORMAT_USE})
    include(clang-format)
endif ()

# Entry point for a whole project configuration. Should be used once in CMakeLists.txt
# If DS_TARGETS cache entry is set, only specified targets will be loaded,
# for example -DDS_TARGETS="ds-iliad;hermes"
# Usage: configure_project()
function(configure_project)
    cmake_parse_arguments(PARSE_ARGV 0 in "" "TARGETS_PATH" "")

    set(ds_targets_path "${ds_project_root}/targets")
    if (in_TARGETS_PATH)
        set(ds_targets_path "${ds_project_root}/${in_TARGETS_PATH}")
    endif ()

    if (NOT EXISTS "${ds_targets_path}")
        message(FATAL_ERROR "configure_project TARGETS_PATH should be specified")
    endif ()

    set(ds_targets_path "${ds_project_root}/${in_TARGETS_PATH}" CACHE STRING "")
    set(ds_targets "" CACHE INTERNAL "" FORCE)

    # Enumerating targets
    file(GLOB_RECURSE target_files LIST_DIRECTORIES false "${ds_targets_path}/**.cmake")
    foreach (path ${target_files})
        get_target_name("${path}" name OUT_PATH path)
        if (("${DS_TARGETS}" STREQUAL "") OR ("${name}" IN_LIST DS_TARGETS))
            list(APPEND ds_targets "${path}")
        endif ()
    endforeach ()
    list(SORT ds_targets)
    set(ds_targets ${ds_targets} CACHE INTERNAL "" FORCE)

    string(JOIN ", " targets_list ${ds_targets})
    set(targets_list "[ ${targets_list} ]")
    message(STATUS "Targets = ${targets_list}")
endfunction()

# Load target
function(load_target name)
    # Isolated scope for all variables in <target>.cmake
    set(path "${ds_targets_path}/${name}.cmake")
    if (NOT EXISTS "${path}")
        message(FATAL_ERROR "Target file ${path} not found.")
    endif ()
    include(${path})
endfunction()

# Loads all found targets
# Usage: load_targets() or load_targets([target1 target2...])
function(load_targets)
    set(targets_to_load ${ds_targets})
    if (NOT ${ARGC} EQUAL 0)
        set(targets_to_load ${ARGV})
    endif ()

    foreach (name ${targets_to_load})
        load_target(${name})
    endforeach ()

    if (${DS_CLANGFORMAT_VERIFY_ON_LOAD})
        verify_project(DIRS "${ds_project_root}")
    endif ()
endfunction()
