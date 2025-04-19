include_guard(GLOBAL)


# Gets a target name and relative path based on <target>.cmake path
# Usage: get_target_name(<in_target_path> <out_target_name> [OUT_PATH <out_target_path>])
function(get_target_name in_path out_name)
    cmake_parse_arguments(PARSE_ARGV 2 in "" "OUT_PATH" "")

    cmake_path(RELATIVE_PATH in_path BASE_DIRECTORY "${ds_targets_path}")
    get_filename_component(filename "${in_path}" NAME_WE)
    get_filename_component(dir "${in_path}" DIRECTORY)
    set(name "${filename}")
    if (NOT "${dir}" STREQUAL "")
        set(name "${dir}/${name}")
    endif ()
    if (in_OUT_PATH)
        set(${in_OUT_PATH} "${name}" PARENT_SCOPE)
    endif ()
    # To have name in format like tools-device-simulator
    #string(REPLACE "/" "-" name "${filename}")
    set(name "${filename}")
    set(${out_name} "${name}" PARENT_SCOPE)
endfunction()


# Begins a target definition. Typically used as a first function in targets/xxxx.cmake
# Default target name will be the file name without extension (cmake/targets/core/logic.cmake -> core-logic)
# Usage: begin_target(EXECUTABLE|LIBRARY|INTERFACE [NAME <target_name>])
function(begin_target)
    cmake_parse_arguments(PARSE_ARGV 0 in "EXECUTABLE;LIBRARY;INTERFACE" "NAME;DIR" "")

    # Handling args
    get_target_name("${path}" default_name OUT_PATH target_path)

    set(target_name "${default_name}")
    if (in_NAME)
        set(target_name "${in_NAME}")
    endif ()

    # Target definition
    if (${in_EXECUTABLE})
        add_executable(${target_name})
    elseif (${in_LIBRARY})
        add_library(${target_name} STATIC)
    elseif (${in_INTERFACE})
        add_library(${target_name} INTERFACE)
    else ()
        message(FATAL_ERROR "One of EXECUTABLE, LIBRARY or INTERFACE should be specified for ${target_path}")
    endif ()

    message(STATUS "Configuring target: ${target_name}")

    # Target compile options
    get_target_property(target_type ${target_name} TYPE)
    if (NOT "${target_type}" STREQUAL "INTERFACE_LIBRARY")
        target_compile_options(${target_name} PRIVATE
                $<$<CXX_COMPILER_ID:MSVC>:${ds_msvc_options}>
                $<$<CXX_COMPILER_ID:GNU>:${ds_gcc_options}>
                $<$<CXX_COMPILER_ID:Clang>:${ds_clang_options}>
                $<$<CXX_COMPILER_ID:AppleClang>:${ds_clang_options}>
                )
        target_compile_definitions(${target_name} PUBLIC ${ds_compile_definitions})
    endif ()

    if (NOT ${DS_USE_PCH})
        if (${target_type} STREQUAL "INTERFACE_LIBRARY")
            target_compile_definitions(${target_name} INTERFACE NO_PCH)
        else ()
            target_compile_definitions(${target_name} PUBLIC NO_PCH)
        endif ()
    endif ()

    if (DS_CLANGFORMAT_APPLY)
        reformat_target(${target_name})
    endif ()

    set(target_name ${target_name} PARENT_SCOPE)
    set(target_path ${target_path} PARENT_SCOPE)
endfunction()


# Ends a target definition. Typically used as a last function in targets/xxxx.cmake
# Usage: end_target()
function(end_target)
    unset(target_name PARENT_SCOPE)
    unset(target_path PARENT_SCOPE)
endfunction()


# Populate sources for the target.
# Default target is the current target in scope of begin_target/end_target.
# Default path is project_root/source/<target>
# Usage: populate_sources([[TARGET <target>] DIR <source_dir>] [EXCLUDE_REGEXP <exclude_regex>] [GLOB <type1> <type2>...])
function(populate_sources)
    cmake_parse_arguments(PARSE_ARGV 0 in "" "TARGET;DIR;EXCLUDE_REGEXP" "GLOB;PCH")

    # Target name
    set(target_name ${target_name})
    if (in_TARGET)
        set(target_name ${in_TARGET})
    endif ()
    if (NOT TARGET ${target_name})
        message(FATAL_ERROR "Use this function between begin_target and end_target. Or use TARGET <target> parameter. Target ${target_name} should exist")
    endif ()

    # Source dir
    if (in_TARGET)
        if (NOT in_DIR)
            message(FATAL_ERROR "DIR <path> should ne specified for a custom TARGET")
        endif ()
    endif ()
    if (in_DIR)
        set(dir "${ds_project_root}/${in_DIR}")
    else ()
        # Possible only when TARGET and DIR are not specified
        set(dir "${ds_project_root}/${ds_source_dir}/${target_path}")
    endif ()

    # File globs
    set(globs "*.c*" "*.h*")
    if (in_GLOB)
        set(globs ${in_GLOB})
    endif ()

    # Enumerating sources
    set(all_files)
    foreach (glob ${globs})
        set(path "${dir}/${glob}")
        file(GLOB_RECURSE files LIST_DIRECTORIES false "${path}")
        list(APPEND all_files ${files})
    endforeach ()
    if (in_EXCLUDE_REGEXP)
        list(FILTER all_files EXCLUDE REGEX ${in_EXCLUDE_REGEXP})
    endif ()

    source_group(TREE ${dir} FILES ${all_files})    # For IDE's like MSVS
    target_sources(${target_name} PRIVATE ${all_files})

    # Default include source dir
    get_target_property(target_type ${target_name} TYPE)
    if (${target_type} STREQUAL "INTERFACE_LIBRARY")
        if (NOT in_DIR)
            # For targets with custom directories include_directories should be manually specified
            target_include_directories(${target_name} INTERFACE "${ds_project_root}/${ds_source_dir}")
        endif ()
    else ()
        target_include_directories(${target_name} PRIVATE "${ds_project_root}/${ds_source_dir}")
    endif ()

    # Target-specific PCH
    set(pch_path "${dir}/pch.h")
    if (EXISTS ${pch_path})
        target_precompile_headers(${target_name} PUBLIC ${pch_path})
    endif ()

endfunction()
