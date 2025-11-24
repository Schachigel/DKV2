cmake_minimum_required(VERSION 3.16)
project(dkv2 LANGUAGES CXX VERSION 0.20.0.11)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTORCC ON)

# Export compile commands for tooling (clangd, clang-tidy, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Git commit hash with error handling
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE GIT_RESULT
    ERROR_QUIET
)

if(GIT_RESULT EQUAL 0)
    add_compile_definitions(GIT_COMMIT="${GIT_COMMIT_HASH}")
    message(STATUS "Git commit hash: ${GIT_COMMIT_HASH}")
else()
    add_compile_definitions(GIT_COMMIT="unknown")
    message(WARNING "Git not found or not a git repository. Using 'unknown' for commit hash.")
endif()

# Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql PrintSupport Test)

qt_standard_project_setup()

# Platform-specific settings
if(APPLE)
    # Lower deployment target for broader compatibility (macOS 10.15 Catalina)
    # set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "Minimum macOS version")
    set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0 CACHE STRING "Minimum macOS version")

    message(STATUS "macOS deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()

# Compiler-specific warning flags
function(set_project_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4                 # High warning level
            /WX                 # Warnings as errors (disabled by default, enable with /WX)
            /permissive-        # Standards conformance
        )
    endif()
endfunction()

# Sanitizers for Debug builds (helps catch bugs)
function(enable_sanitizers target)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        if(MSVC)
            # MSVC AddressSanitizer requires specific runtime library settings
            # It's incompatible with /MDd, so we need to change to /MTd
            # However, this can cause issues with Qt which uses /MD(d)
            # So we'll make sanitizers optional on MSVC
            option(ENABLE_MSVC_ASAN "Enable AddressSanitizer on MSVC (requires /MT runtime)" OFF)

            if(ENABLE_MSVC_ASAN)
                target_compile_options(${target} PRIVATE
                    /fsanitize=address
                    /Zi  # Debug info
                )
                # Change runtime library to /MTd for Debug, /MT for Release
                set_target_properties(${target} PROPERTIES
                    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
                )
                target_link_options(${target} PRIVATE /fsanitize=address)
                message(STATUS "Enabled AddressSanitizer for ${target} (MSVC with /MT runtime)")
            else()
                message(STATUS "MSVC sanitizers disabled for ${target} (incompatible with Qt's /MD runtime)")
            endif()
        endif()
    endif()
endfunction()

# Subdirectories
add_subdirectory(src/DKV2)
add_subdirectory(src/Tests)
