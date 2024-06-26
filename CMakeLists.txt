﻿cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Must be included before a project() call
include(ds-cmake/cmake/vcpkg.cmake)

project("ds-dancetools" LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Options

# Project configuration
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_NO_WARN_NEW_VERSIONS TRUE)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# Load project
include(ds-cmake/cmake/core.cmake)
configure_project(TARGETS_PATH "targets")
load_targets()
