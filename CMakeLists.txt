cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project("ds-dancetools" LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_PREFIX_PATH "./_thirdparty/${CMAKE_SYSTEM_PROCESSOR}")

# Options

# Project configuration
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_NO_WARN_NEW_VERSIONS TRUE)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# Load project
include(cmake/core.cmake)
configure_project(TARGETS_PATH "targets")
load_targets()
