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
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "Minimum macOS version")
    message(STATUS "macOS deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()

# Compiler-specific warning flags
function(set_project_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4                 # High warning level
            /WX-                # Warnings as errors (disabled by default, enable with /WX)
            /permissive-        # Standards conformance
            /w14242             # Conversion warnings
            /w14254             # operator mismatch
            /w14263             # Member function does not override base class virtual
            /w14265             # Class has virtual functions, but destructor is not virtual
            /w14287             # Unsigned/negative constant mismatch
            /we4289             # Loop variable used outside loop
            /w14296             # Expression is always true/false
            /w14311             # Pointer truncation
            /w14545             # Expression evaluates to function missing argument list
            /w14546             # Function call before comma missing argument list
            /w14547             # Operator before comma has no effect
            /w14549             # Operator before comma has no effect
            /w14555             # Expression has no effect
            /w14619             # Pragma warning suppression
            /w14640             # Thread-unsafe static member initialization
            /w14826             # Conversion is sign-extended
            /w14905             # Wide string literal cast
            /w14906             # String literal cast
            /w14928             # Illegal copy-initialization
        )
    else()
        target_compile_options(${target} PRIVATE
            -Wall               # Enable most warnings
            -Wextra             # Enable extra warnings
            -Wpedantic          # Strict ISO C++ warnings
            -Wshadow            # Warn if variable shadows another
            -Wnon-virtual-dtor  # Warn if class with virtual functions has non-virtual destructor
            -Wold-style-cast    # Warn for C-style casts
            -Wcast-align        # Warn for potential alignment issues
            -Wunused            # Warn on unused variables
            -Woverloaded-virtual # Warn if you overload (not override) a virtual function
            -Wconversion        # Warn on type conversions that may lose data
            -Wsign-conversion   # Warn on sign conversions
            -Wdouble-promotion  # Warn if float is implicitly promoted to double
            -Wformat=2          # Warn on security issues around format strings
        )
        
        # GCC-specific warnings
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${target} PRIVATE
                -Wmisleading-indentation
                -Wduplicated-cond
                -Wduplicated-branches
                -Wlogical-op
                -Wnull-dereference
            )
        endif()
        
        # Clang-specific warnings
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${target} PRIVATE
                -Wmost
                -Wextra-semi
            )
        endif()
    endif()
endfunction()

# Sanitizers for Debug builds (helps catch bugs)
function(enable_sanitizers target)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        if(MSVC)
            # MSVC has limited sanitizer support
            target_compile_options(${target} PRIVATE /fsanitize=address)
            target_link_options(${target} PRIVATE /fsanitize=address)
            message(STATUS "Enabled AddressSanitizer for ${target} (MSVC)")
        else()
            # GCC and Clang support multiple sanitizers
            target_compile_options(${target} PRIVATE
                -fsanitize=address              # Memory errors
                -fsanitize=undefined            # Undefined behavior
                -fsanitize=leak                 # Memory leaks
                -fno-omit-frame-pointer         # Better stack traces
                -fno-optimize-sibling-calls     # Better debugging
            )
            target_link_options(${target} PRIVATE
                -fsanitize=address
                -fsanitize=undefined
                -fsanitize=leak
            )
            message(STATUS "Enabled sanitizers for ${target} (Address, UndefinedBehavior, Leak)")
        endif()
    endif()
endfunction()

# Subdirectories
add_subdirectory(src/DKV2)
add_subdirectory(src/Tests)