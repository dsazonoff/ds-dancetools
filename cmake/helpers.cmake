include_guard(GLOBAL)


# List for wrappers for find_package + target_link_libraries + target_include_directories

function(package_add_openssl)
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        # Patch for the latest OpenSSL and Windows
        # Probably it will be fixed in next cmake releases
        if (EXISTS "${VCPKG_ROOT}")
            set(OPENSSL_ROOT_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}")
        endif ()
    endif ()

    find_package(OpenSSL REQUIRED)
    foreach (arg ${ARGV})
        target_link_libraries(${arg} PUBLIC OpenSSL::SSL OpenSSL::Crypto)
    endforeach ()
endfunction()

function(package_add_boost)
    if(POLICY CMP0167)
        cmake_policy(SET CMP0167 NEW)
    endif()
    find_package(Iconv REQUIRED)
    find_package(ICU REQUIRED COMPONENTS uc dt in io)
    find_package(Boost COMPONENTS system program_options exception locale thread CONFIG REQUIRED)
    foreach (arg ${ARGV})
        target_link_libraries(${arg} PUBLIC ${Boost_LIBRARIES} Threads::Threads ${Iconv_LIBRARIES} ${ICU_LIBRARIES})
        target_include_directories(${arg} SYSTEM PUBLIC "${Boost_INCLUDE_DIR}")
    endforeach ()
endfunction()

function(package_add_sqlite_orm)
    find_package(SQLite3 REQUIRED)
    find_package(SqliteOrm CONFIG REQUIRED)
    foreach (arg ${ARGV})
        target_link_libraries(${arg} PUBLIC sqlite_orm::sqlite_orm)
    endforeach ()
endfunction()

function(package_add_fmt)
    find_package(fmt CONFIG REQUIRED)
    foreach (arg ${ARGV})
        target_link_libraries(${arg} PUBLIC fmt::fmt-header-only)
    endforeach ()
endfunction()

function(package_add_magic_enum)
    find_package(magic_enum CONFIG REQUIRED)
    foreach (arg ${ARGV})
        target_link_libraries(${arg} PUBLIC magic_enum::magic_enum)
    endforeach ()
endfunction()

function(package_add_taskflow)
    find_package(Taskflow CONFIG REQUIRED)
    foreach (arg ${ARGV})
        target_link_libraries(${arg} PUBLIC Taskflow::Taskflow)
    endforeach ()
endfunction()


# Find packages and attaches them to a target
# Automatically creates and attaches pch to a target
# PCH header is calculated based on PCH <name> value. File "<name>.h" should exist.
# Usage: configure_packages_and_pch([TARGET <target>] [PCH_DIR <dir>] DEPENDENCIES <dep1> <dep2>...)
# Example: configure_packages_and_pch(DEPENDENCIES openssl boost stl flatbuffers json)
function(configure_packages_and_pch)
    cmake_parse_arguments(PARSE_ARGV 0 in "" "TARGET;PCH_DIR" "DEPENDENCIES")

    # Parse arguments
    set(target_name ${target_name})
    if (in_TARGET)
        set(target_name ${in_TARGET})
    endif ()
    if (NOT TARGET ${target_name})
        message(FATAL_ERROR "Should be called in scope of begin_target()/end_target() or TARGET must point to an existing target")
    endif ()

    set(pch_dir "${ds_project_root}/${ds_source_dir}/pch")
    if (in_PCH_DIR)
        set(pch_dir "${ds_project_root}/${in_PCH_DIR}")
    endif ()

    get_target_property(target_type ${target_name} TYPE)

    # Load dependencies
    foreach (name ${in_DEPENDENCIES})
        # If pch is ON and exists, use it
        if (DS_USE_PCH)
            set(pch_path "${pch_dir}/${name}-pch.h")
            if (EXISTS ${pch_path})
                if (NOT ${target_type} STREQUAL "INTERFACE_LIBRARY")
                    target_precompile_headers(${target_name} PUBLIC ${pch_path})
                endif ()
            endif ()
        endif ()

        if (${name} STREQUAL stl)
            # Nothing to for STL
        elseif (${name} STREQUAL openssl)
            package_add_openssl(${target_name})
        elseif (${name} STREQUAL boost)
            package_add_boost(${target_name})
        elseif (${name} STREQUAL sqlite_orm)
            package_add_sqlite_orm(${target_name})
        elseif (${name} STREQUAL fmt)
            package_add_fmt(${target_name})
        elseif (${name} STREQUAL magic_enum)
            package_add_magic_enum(${target_name})
        elseif (${name} STREQUAL taskflow)
            package_add_taskflow(${target_name})
        else ()
            message(FATAL_ERROR "No helper exists to load dependencies. Please implement package_add_${name} function in helpers.cmake")
        endif ()
    endforeach ()
endfunction()

function(add_version_define target macro_name)
    set(version_string 0.0)
    set(commit_count 0)

    find_package(Git QUIET)
    if (Git_FOUND)
        # Get version from latest tag
        execute_process(
                COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE result
                OUTPUT_VARIABLE last_tag
        )
        if (${result} EQUAL 0)
            string(REGEX MATCH "[0-9]+.[0-9]+" version_from_git ${last_tag})
            if (NOT ${version_from_git} STREQUAL "")
                set(version_string ${version_from_git})
            endif ()
        else ()
            message(WARNING "Could not detect a version for ${target}. Please, check that git tags are set.")
        endif ()

        # Get commit count since last tag
        execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-list ${version_string}..HEAD --count
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE result
                OUTPUT_VARIABLE commit_count
        )
        if (${result} EQUAL 0)
            string(REGEX MATCH "[0-9]+" commit_count ${commit_count})
            set(version_string "${version_string}.${commit_count}")
        endif ()
    endif ()

    set(version "${version_string}")
    target_compile_definitions(${target} PUBLIC ${macro_name}=\"${version}\")
    set(${macro_name} ${version} PARENT_SCOPE)

    set_target_properties(${target} PROPERTIES git_based_version "${version}")

    message(STATUS "Version for ${target}: ${macro_name} = ${version}")
endfunction()
