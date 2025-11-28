cmake_minimum_required(VERSION 3.16)
project(dkv2 LANGUAGES CXX VERSION 0.20.0.11)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

# Git commit hash
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_compile_definitions(GIT_COMMIT="${GIT_COMMIT_HASH}")

# Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql PrintSupport Test)

qt_standard_project_setup()

# Platform-specific settings
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0 CACHE STRING "Minimum macOS version")
endif()

# Subdirectories
add_subdirectory(src/DKV2)
add_subdirectory(src/Tests)
