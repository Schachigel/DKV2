set (TARGET Tests)

install(TARGETS DKV2
  BUNDLE DESTINATION .
  CONFIGURATIONS Debug Release RelWithDebInfo MinSizeRel
)

qt_add_executable(${TARGET}
    test_main.cpp
    resource.qrc
    ../DKV2/pch.h

    # eigene Testdateien
    test_appconfig.cpp test_appconfig.h
    test_booking.cpp test_booking.h
    test_contract.cpp test_contract.h
    test_creditor.cpp test_creditor.h
    test_csv.cpp test_csv.h
    test_db.cpp test_db.h
    test_dbtable.cpp test_dbtable.h
    test_dkdbcopy.cpp test_dkdbcopy.h
    test_dkdbhelper.cpp test_dkdbhelper.h
    test_finance.cpp test_finance.h
    test_properties.cpp test_properties.h
    test_sqlhelper.cpp test_sqlhelper.h
    test_statistics.cpp test_statistics.h
    test_tabledatainserter.cpp test_tabledatainserter.h
    test_views.cpp test_views.h
    testhelper.cpp testhelper.h
    financaltimespan.cpp financaltimespan.h

    # benötigte Quelldateien aus DKV2
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

target_compile_definitions(Tests PRIVATE
  $<$<CONFIG:Debug>:DKV2_TEST_DEBUG>
)

find_package(Qt6 REQUIRED COMPONENTS Test Widgets Sql PrintSupport)

target_precompile_headers(${TARGET} PRIVATE ../DKV2/pch.h)
set_target_properties(${TARGET} PROPERTIES AUTOGEN_USE_PCH ON)

target_link_libraries(${TARGET} PRIVATE
    Qt6::Test Qt6::Core Qt6::Widgets Qt6::Sql Qt6::PrintSupport
)

# explizit Konsolen-Subsystem unter Windows
if(MSVC)
    target_link_options(${TARGET} PRIVATE /SUBSYSTEM:CONSOLE)
endif()

# optional: Tests installierbar machen
install(TARGETS ${TARGET} RUNTIME DESTINATION tests)

