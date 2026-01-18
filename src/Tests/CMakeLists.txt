cmake_minimum_required(VERSION 3.21)

# Qt packages
find_package(Qt6 REQUIRED COMPONENTS Test)

# -------------------------------------------------
# Helper: one executable per suite (better Qt Creator integration + smaller logs)
# -------------------------------------------------
function(add_dkv2_test TARGET_NAME TEST_HEADER TEST_CLASS)
    qt_add_executable(${TARGET_NAME}
        test_main.cpp
        testhelper.cpp
        testhelper.h
        ${TEST_HEADER}           # optional, für IDE-Ansicht
        ${ARGN}                  # die .cpp Datei(en) der Suite
    )

    target_link_libraries(${TARGET_NAME} PRIVATE
        Qt6::Test
        dkv2_core
    )

    target_precompile_headers(${TARGET_NAME} PRIVATE
        ../DKV2/pch_core.h
    )
    set_target_properties(${TARGET_NAME} PROPERTIES AUTOGEN_USE_PCH ON)

    target_compile_definitions(${TARGET_NAME} PRIVATE
        GEN_TEST_HEADER="${TEST_HEADER}"
        GEN_TEST_CLASS=${TEST_CLASS}
    )

    if(MSVC)
        target_link_options(${TARGET_NAME} PRIVATE /SUBSYSTEM:CONSOLE)
    endif()

    add_test(
        NAME    ${TARGET_NAME}
        COMMAND $<TARGET_FILE:${TARGET_NAME}>)
    set_tests_properties(
        ${TARGET_NAME} PROPERTIES
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${TARGET_NAME}>)

endfunction()

# -------------------------------------------------
# Common test sources reused across suites
# (compiled into each test exe; OK and simple)
# -------------------------------------------------
set(TEST_COMMON
    test_main.cpp
    testhelper.cpp
    testhelper.h
)

add_dkv2_test(test_csv "test_csv.h" test_csv
    test_csv.cpp
)

add_dkv2_test(test_db "test_db.h" test_db
    test_db.cpp
)

add_dkv2_test(test_views "test_views.h" test_views
    test_views.cpp
)

add_dkv2_test(test_dkdbcopy "test_dkdbcopy.h" test_dkdbcopy
    test_dkdbcopy.cpp
)

add_dkv2_test(test_dkdbhelper "test_dkdbhelper.h" test_dkdbhelper
    test_dkdbhelper.cpp
)

add_dkv2_test(test_tableDataInserter "test_tabledatainserter.h" test_tableDataInserter
    test_tabledatainserter.cpp
)

add_dkv2_test(test_dbfield "test_dbfield.h" test_dbfield
    test_dbfield.cpp
)

add_dkv2_test(test_creditor "test_creditor.h" test_creditor
    test_creditor.cpp
)

add_dkv2_test(test_booking "test_booking.h" test_booking
    test_booking.cpp
)

add_dkv2_test(test_contract "test_contract.h" test_contract
    test_contract.cpp
)

add_dkv2_test(test_finance "test_finance.h" test_finance
    financaltimespan.cpp
    test_finance.cpp
)

add_dkv2_test(test_statistics "test_statistics.h" test_statistics
    test_statistics.cpp
)

add_dkv2_test(test_appConfig "test_appconfig.h" test_appConfig
    test_appconfig.cpp
)

add_dkv2_test(test_properties "test_properties.h" test_properties
    test_properties.cpp
)

