include_guard(GLOBAL)

set(codestyle_path "${CMAKE_SOURCE_DIR}/.clang-format" CACHE FILEPATH ".clang-format file")
set(default_branch "origin/master" CACHE STRING "Default git branch to compare for clang-format checks")

# clang-format app
find_program(
        clang_format
        NAMES "clang-format"
        DOC "Path to clang-format"
        REQUIRED
)
message(STATUS "clang format = ${clang_format}")

# git
find_package(Git REQUIRED)

# clang formatting functions

# Reformat list of files with clang-format utility, recursively
# Usage: apply_codestyle([SOURCE_DIR <dir>] [CODESTYLE <.clang-format file>] [FILES <files...>])
function(apply_codestyle)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE_DIR;CODESTYLE" "FILES")
    set(source_dir "${CMAKE_SOURCE_DIR}")
    if (arg_SOURCE_DIR)
        set(source_dir "${arg_SOURCE_DIR}")
    endif ()
    set(codestyle "${source_dir}/.clang-format")
    if (arg_CODESTYLE)
        set(codestyle "${arg_CODESTYLE}")
    endif ()
    if (NOT arg_FILES)
        return()
    endif ()

    execute_process(
            COMMAND ${clang_format} -i --style=file:${codestyle} ${arg_FILES}
            WORKING_DIRECTORY ${source_dir}
            RESULTS_VARIABLE result
    )
endfunction()

# Compare two files
# Usage: compare_files(result "1.cpp" "2.cpp")
# "result" will be 0 if files are equal
function(compare_files out_result path1 path2)
    # Can be speedup with different compare mechanism
    #    execute_process(
    #            COMMAND ${GIT_EXECUTABLE} diff --exit-code --quiet --no-index "${path1}" "${path2}"
    #            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    #            RESULT_VARIABLE result
    #    )
    execute_process(
            COMMAND ${CMAKE_COMMAND} -E compare_files "${path1}" "${path2}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE result
    )
    set(${out_result} ${result} PARENT_SCOPE)
endfunction()

# Verify codestyle for a list of files
# Source files are copied to the build/<config>/clang-format and compared with compare_files function
# Usage: verify_codestyle([SOURCE_DIR <dir>] [TEMP_DIR <dir>] OUTPUT <result> FILES <files>)
# "result" will contain a list of source files that doesn't match the codestyle
function(verify_codestyle)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE_DIR;TEMP_DIR;OUTPUT" "FILES")
    set(source_dir "${CMAKE_SOURCE_DIR}")
    if (arg_SOURCE_DIR)
        set(source_dir "${arg_SOURCE_DIR}")
    endif ()
    set(temp_dir "${CMAKE_BINARY_DIR}/clang-format")
    if (arg_TEMP_DIR)
        set(temp_dir "${arg_TEMP_DIR}")
    endif ()
    if (NOT arg_OUTPUT)
        message(FATAL_ERROR "OUTPUT <var> should be specified")
    endif ()

    set(modified_files)
    foreach (path ${arg_FILES})
        # Format file to a temp dir
        set(full_path "${path}")
        cmake_path(HAS_ROOT_PATH full_path is_absolute)
        if (NOT ${is_absolute})
            set(full_path "${source_dir}/${path}")
        endif ()
        cmake_path(RELATIVE_PATH full_path BASE_DIRECTORY "${source_dir}" OUTPUT_VARIABLE relpath) # TODO: fix relative path for files outside of ${source_dir}
        set(temp_path "${temp_dir}/${relpath}")
        configure_file("${full_path}" "${temp_path}" COPYONLY)
        apply_codestyle(SOURCE_DIR "${source_dir}" CODESTYLE "${codestyle_path}" FILES "${temp_path}")
        # Compare
        compare_files(result "${full_path}" "${temp_path}")
        if (NOT ${result} EQUAL 0)
            list(APPEND modified_files ${full_path})
        endif ()
    endforeach ()
    set(${arg_OUTPUT} ${modified_files} PARENT_SCOPE)
endfunction()

# Gets a list of modified files (diff) in the current git branch
# Diff is calculated relatively to origin/master (or BRANCH argument)
# So it's useful to do a pull+rebase before running this check
# Usage: get_modified_files([SOURCE_DIR <dir>] [BRANCH <branch>] OUTPUT files [EXTENSIONS ".c;.cpp;.h"])
# EXTENSIONS - a list of file extensions to check. It's useful to ignore non-cpp files
function(get_modified_files)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE_DIR;BRANCH;OUTPUT" "EXTENSIONS")
    set(source_dir "${CMAKE_SOURCE_DIR}")
    if (arg_SOURCE_DIR)
        set(source_dir "${arg_SOURCE_DIR}")
    endif ()
    set(branch ${default_branch})
    if (arg_BRANCH)
        set(branch ${arg_BRANCH})
    endif ()
    if (NOT arg_OUTPUT)
        message(FATAL_ERROR "OUTPUT <var> should be specified")
    endif ()
    set(extensions .c .cc .cpp .h .hpp .tcc)
    if (arg_EXTENSIONS)
        set(extensions ${arg_EXTENSIONS})
    endif ()

    execute_process(
            #COMMAND ${GIT_EXECUTABLE} diff --name-only --merge-base ${branch} # TODO: use --merge-base with git v.2.38+, not supported in Ubuntu 20
            COMMAND ${GIT_EXECUTABLE} diff --name-only ${branch}
            WORKING_DIRECTORY ${source_dir}
            OUTPUT_VARIABLE output
            RESULT_VARIABLE result
    )

    string(REPLACE "\n" ";" output ${output})
    set(files)
    foreach (file ${output})
        set(full_path "${source_dir}/${file}")
        # Include only files (not dirs) with specified extension
        cmake_path(GET file EXTENSION LAST_ONLY ext)
        list(FIND extensions "${ext}" ext_id)
        if (NOT IS_DIRECTORY "${full_path}"
                AND EXISTS "${full_path}"
                AND NOT ${ext_id} EQUAL -1
                )
            list(APPEND files "${full_path}")
        endif ()
    endforeach ()
    set(${arg_OUTPUT} ${files} PARENT_SCOPE)
endfunction()

# Verifies the codestyle in the list of directories recursively
# Diff is calculated relatively to origin/master (or BRANCH argument)
# Usage: verify_project(DIRS <dirs...> [BRANCH <branch>] [TEMP_DIR <dir>] [EXTENSIONS <file extensions...>])
function(verify_project)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "BRANCH;TEMP_DIR" "DIRS;EXTENSIONS")
    set(branch ${default_branch})
    if (arg_BRANCH)
        set(branch ${arg_BRANCH})
    endif ()
    set(temp_dir "${CMAKE_BINARY_DIR}/clang-format")
    if (arg_TEMP_DIR)
        set(temp_dir "${arg_TEMP_DIR}")
    endif ()
    if (NOT arg_DIRS)
        message(FATAL_ERROR "DIRS <directories> should be specified")
    endif ()
    set(ext ${arg_EXTENSIONS})
    if (arg_EXTENSIONS)
        set(ext ${arg_EXTENSIONS})
    endif ()

    foreach (dir ${arg_DIRS})
        message(STATUS "Verifying code style for: ${dir}")
        get_modified_files(SOURCE_DIR "${dir}" BRANCH ${branch} OUTPUT files)
        verify_codestyle(SOURCE_DIR "${dir}" TEMP_DIR "${CMAKE_BINARY_DIR}" OUTPUT failed FILES ${files})
        set(failed_list)
        foreach (file ${failed})
            cmake_path(RELATIVE_PATH file BASE_DIRECTORY "${dir}")
            list(APPEND failed_list "${file}")
        endforeach ()
        list(LENGTH failed_list count)
        if (NOT ${count} EQUAL 0)
            # Use one extra space at each new line to override default cmake formatting for warnings and errors
            string(JOIN "\n " text ${failed_list})
            message(FATAL_ERROR " Code style verification failed. List of files:\n ${text}")
        endif ()
    endforeach ()
endfunction()

# Creates a reformat target and add it as dependency
# Reformatting will happen as a pre-build step, before a target build
# Usage: reformat_target(<target>)
function(reformat_target target)
    set(format clang-format)
    if (NOT TARGET ${format})
        add_custom_target(${format}
                ${CMAKE_COMMAND} -Dpath=${CMAKE_SOURCE_DIR} -Dcodestyle=${codestyle_path} -Dbranch=${default_branch} -P ${ds_cmake_root}/../scripts/clang-reformat.cmake
                VERBATIM
                COMMENT "Reformatting source code"
                )
    endif ()
    add_dependencies(${target} ${format})
endfunction()
