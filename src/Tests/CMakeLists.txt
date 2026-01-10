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

# Create test executable
qt_add_executable(${TARGET} ${TEST_SOURCES} ${DKV2_TEST_SOURCES}
    xtest_tolearn.h xtest_tolearn.cpp)

# Compile definitions
target_compile_definitions(${TARGET} PRIVATE
    $<$<CONFIG:Debug>:DKV2_TEST_DEBUG>
)

# Precompiled headers
target_precompile_headers(${TARGET} PRIVATE ../DKV2/pch.h)
set_target_properties(${TARGET} PROPERTIES AUTOGEN_USE_PCH ON)

# Link libraries
target_link_libraries(${TARGET} PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
)

# Windows-specific: Force console subsystem
if(MSVC)
    target_link_options(${TARGET} PRIVATE /SUBSYSTEM:CONSOLE)
endif()

# Installation
install(TARGETS ${TARGET}
    RUNTIME DESTINATION tests
    BUNDLE DESTINATION .
    CONFIGURATIONS Debug Release RelWithDebInfo MinSizeRel
)
