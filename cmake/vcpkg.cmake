# vcpkg package manager configuration
# https://github.com/microsoft/vcpkg/blob/master/docs/users/integration.md#with-cmake

# Try to find vcpkg

if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    cmake_path(NORMAL_PATH CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" CACHE PATH "")
    set(VCPKG_ROOT "$ENV{VCPKG_ROOT}" CACHE PATH "")
endif ()

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(vcpkg_hints
            "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/"
            "/opt/vcpkg"
            )
    set(vcpkg_toolchain "scripts/buildsystems/vcpkg.cmake")

    foreach (path ${vcpkg_hints})
        cmake_path(NORMAL_PATH path)
        set(full_path "${path}/${vcpkg_toolchain}")
        cmake_path(NORMAL_PATH full_path)
        if (EXISTS "${full_path}" AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
            set(CMAKE_TOOLCHAIN_FILE "${full_path}" CACHE STRING "")
            set(VCPKG_ROOT "${path}" CACHE PATH "")
        endif ()
    endforeach ()
    set(CMAKE_TOOLCHAIN_FILE "" CACHE STRING "")
    if (NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
        message(WARNING "Could not locate vcpkg.cmake file. Compilation may fail.
Ignore this warning if you are manually managing thirdparty dependencies.")
    endif ()
endif ()

if (DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif ()
if (NOT DEFINED VCPKG_TARGET_TRIPLET)
    if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
        set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
    elseif ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
        execute_process(COMMAND uname -m OUTPUT_VARIABLE CMAKE_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
        if ("${CMAKE_ARCH}" STREQUAL "aarch64")
            set(VCPKG_TARGET_TRIPLET "arm64-linux" CACHE STRING "")
            set(ENV{VCPKG_FORCE_SYSTEM_BINARIES} 1)
        else ()
            set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "")
        endif ()
    elseif ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Darwin")
        execute_process(COMMAND uname -m OUTPUT_VARIABLE CMAKE_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
        if ("${CMAKE_ARCH}" STREQUAL "arm64")
            set(VCPKG_TARGET_TRIPLET "arm64-osx" CACHE STRING "")
        else ()
            set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "")
        endif ()
    endif ()
endif ()

# Don't use manifest, because all libraries should installed at bootstrap step
set(VCPKG_MANIFEST_MODE OFF CACHE INTERNAL "Use vcpkg manifest mode" FORCE)

message(STATUS "VCPKG_ROOT = ${VCPKG_ROOT}")
message(STATUS "VCPKG_TARGET_TRIPLET = ${VCPKG_TARGET_TRIPLET}")
message(STATUS "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
