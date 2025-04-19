include_guard(GLOBAL)


# This file contain predefined global variables that are used across the project like constants, compiler and linker
# flags, predefined macro etc


# Common options
option(DS_USE_PCH "Use precompiled headers" ON)
option(DS_CLANGFORMAT_VERIFY_ON_LOAD "Verify .clang-format rules on project load" OFF)
option(DS_CLANGFORMAT_APPLY "Apply .clang-format rules on build" OFF)
option(DS_CLANGFORMAT_ONLY_MODIFIED "Verify with clang-format only modified files" ON) # TODO: impement for all files
set(DS_CLANGFORMAT_USE FALSE CACHE INTERNAL "(internal) Use clang-format.cmake" FORCE)
set(DS_TARGETS "" CACHE STRING "Override list of targets")

if (${DS_CLANGFORMAT_VERIFY_ON_LOAD} OR ${DS_CLANGFORMAT_APPLY})
    set(DS_CLANGFORMAT_USE TRUE CACHE INTERNAL "(internal) Use clang-format.cmake" FORCE)
endif ()

# Compiler options
set(ds_msvc_options /W4 /WX /wd4702)
set(ds_clang_options -Wall -Wextra -Wpedantic -Werror
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-deprecated-declarations
        -Wno-reserved-id-macro
        -Wno-unsafe-buffer-usage
        -Wno-missing-prototypes
#        -Wno-switch-enum
#        -Wno-unused-function
#        -Wno-unused-macros
#        -Wno-unused-parameter
#        -Wno-unused-variable
        )
set(ds_gcc_options -Wall -Wextra -Wpedantic -Werror
        )

# Compiler definitions
set(ds_compile_definitions
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
        BOOST_BIND_GLOBAL_PLACEHOLDERS
        _CRT_SECURE_NO_WARNINGS
        )

# Platform specific fixes
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    list(APPEND ds_clang_options /EHs)
    list(APPEND ds_compile_definitions _WIN32_WINNT=0x0601)
endif ()

# Global variables
set(ds_source_dir "source")
