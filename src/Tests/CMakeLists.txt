set(TARGET Tests)

# Test sources
set(TEST_SOURCES
    test_main.cpp
    resource.qrc
    
    # Test files
    test_appconfig.cpp
    test_appconfig.h
    test_booking.cpp
    test_booking.h
    test_contract.cpp
    test_contract.h
    test_creditor.cpp
    test_creditor.h
    test_csv.cpp
    test_csv.h
    test_db.cpp
    test_db.h
    test_dbtable.cpp
    test_dbtable.h
    test_dkdbcopy.cpp
    test_dkdbcopy.h
    test_dkdbhelper.cpp
    test_dkdbhelper.h
    test_finance.cpp
    test_finance.h
    test_properties.cpp
    test_properties.h
    test_sqlhelper.cpp
    test_sqlhelper.h
    test_statistics.cpp
    test_statistics.h
    test_tabledatainserter.cpp
    test_tabledatainserter.h
    testhelper.cpp
    testhelper.h
    financaltimespan.cpp
    financaltimespan.h
)

# DKV2 source files needed for tests
set(DKV2_TEST_SOURCES
    ../DKV2/helper.cpp
    ../DKV2/appconfig.cpp
    ../DKV2/booking.cpp
    ../DKV2/contract.cpp
    ../DKV2/creditor.cpp
    ../DKV2/dbtable.cpp
    ../DKV2/dbfield.cpp
    ../DKV2/dbstructure.cpp
    ../DKV2/helperfile.cpp
    ../DKV2/helpersql.cpp
    ../DKV2/dkdbhelper.cpp
    ../DKV2/dkdbcopy.cpp
    ../DKV2/dkdbindices.cpp
    ../DKV2/dkdbviews.cpp
    ../DKV2/csvwriter.cpp
    ../DKV2/investment.cpp
    ../DKV2/helperfin.cpp
    ../DKV2/ibanvalidator.cpp
    ../DKV2/tabledatainserter.cpp
)

# Verify that all source files exist
foreach(src_file ${DKV2_TEST_SOURCES})
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src_file}")
        message(WARNING "Source file not found: ${src_file}")
    endif()
endforeach()

# Create test executable
qt_add_executable(${TARGET} ${TEST_SOURCES} ${DKV2_TEST_SOURCES})

# Compile definitions
target_compile_definitions(${TARGET} PRIVATE
    $<$<CONFIG:Debug>:DKV2_TEST_DEBUG>
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
)

# Apply project warnings (defined in root CMakeLists.txt)
if(COMMAND set_project_warnings)
    set_project_warnings(${TARGET})
endif()

# Enable sanitizers for Debug builds (defined in root CMakeLists.txt)
if(COMMAND enable_sanitizers)
    enable_sanitizers(${TARGET})
endif()

# Precompiled headers
target_precompile_headers(${TARGET} PRIVATE ../DKV2/pch.h)
set_target_properties(${TARGET} PROPERTIES 
    AUTOGEN_USE_PCH ON
    AUTOMOC ON
    AUTOUIC ON
)

# Link libraries
target_link_libraries(${TARGET} PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
)

# Windows-specific: Force console subsystem for test output
if(WIN32)
    if(MSVC)
        target_link_options(${TARGET} PRIVATE /SUBSYSTEM:CONSOLE)
    endif()
    set_target_properties(${TARGET} PROPERTIES
        WIN32_EXECUTABLE OFF  # Ensure console window appears
    )
endif()

# Enable Qt Test features
set_target_properties(${TARGET} PROPERTIES
    # Ensure tests can write temporary files
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Create output directory for tests
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# Enable CTest integration
enable_testing()

# Add test to CTest
add_test(
    NAME ${TARGET}
    COMMAND ${TARGET}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Set test properties
set_tests_properties(${TARGET} PROPERTIES
    TIMEOUT 300  # 5 minutes timeout for all tests
    LABELS "unit"
)

# Platform-specific test environment
if(WIN32)
    # Add Qt DLL path to test environment on Windows
    set_tests_properties(${TARGET} PROPERTIES
        ENVIRONMENT "PATH=${Qt6_DIR}/../../../bin;$ENV{PATH}"
    )
elseif(APPLE)
    # Set DYLD_LIBRARY_PATH on macOS
    set_tests_properties(${TARGET} PROPERTIES
        ENVIRONMENT "DYLD_LIBRARY_PATH=${Qt6_DIR}/../../../lib:$ENV{DYLD_LIBRARY_PATH}"
    )
else()
    # Set LD_LIBRARY_PATH on Linux
    set_tests_properties(${TARGET} PROPERTIES
        ENVIRONMENT "LD_LIBRARY_PATH=${Qt6_DIR}/../../../lib:$ENV{LD_LIBRARY_PATH}"
    )
endif()

# Installation (optional - useful for packaging tests separately)
install(TARGETS ${TARGET}
    RUNTIME DESTINATION tests
    COMPONENT Tests
    CONFIGURATIONS Debug Release RelWithDebInfo MinSizeRel
    OPTIONAL  # Don't fail if tests are not built
)

# Install test data files (if any exist)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/testdata")
    install(DIRECTORY testdata
        DESTINATION tests
        COMPONENT Tests
        OPTIONAL
    )
endif()

# Custom target to run tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS ${TARGET}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests..."
    VERBATIM
)

# Print test configuration
message(STATUS "")
message(STATUS "Tests Configuration:")
message(STATUS "  Test executable: ${TARGET}")
message(STATUS "  Test output directory: ${CMAKE_BINARY_DIR}/tests")
message(STATUS "  Number of test source files: ${CMAKE_LIST_LENGTH TEST_SOURCES}")
message(STATUS "  Number of DKV2 source files linked: ${CMAKE_LIST_LENGTH DKV2_TEST_SOURCES}")
message(STATUS "  Run tests with: cmake --build . --target run_tests")
message(STATUS "  Or with CTest: ctest --output-on-failure")
message(STATUS "")