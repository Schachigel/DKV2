set (TARGET Tests)

qt_add_executable( ${TARGET}
    test_main.cpp

    resource.qrc
    ../DKV2/pch.h
    ../DKV2/helper.cpp
    financaltimespan.cpp
    financaltimespan.h
    ../DKV2/dkdbhelper.cpp
    ../DKV2/appconfig.cpp
    test_appconfig.cpp
    test_appconfig.h
    ../DKV2/booking.cpp
    test_booking.cpp
    test_booking.h
    ../DKV2/contract.cpp
    test_contract.cpp
    test_contract.h
    ../DKV2/creditor.cpp
    test_creditor.cpp
    test_creditor.h
    test_csv.cpp
    test_csv.h
    test_db.cpp
    test_db.h
    ../DKV2/dbtable.cpp
    ../DKV2/dbfield.cpp
    ../DKV2/dbstructure.cpp
    test_dbtable.cpp
    test_dbtable.h
    ../DKV2/helperfile.cpp
    ../DKV2/helpersql.cpp
    ../DKV2/dkdbhelper.cpp
    ../DKV2/dkdbcopy.cpp
    ../DKV2/dkdbindices.cpp
    ../DKV2/dkdbviews.cpp
    test_dkdbcopy.cpp
    test_dkdbcopy.h
    ../DKV2/csvwriter.cpp
    test_dkdbhelper.cpp
    test_dkdbhelper.h
    ../DKV2/investment.cpp
    ../DKV2/helperfin.cpp
    ../DKV2/ibanvalidator.cpp
    test_finance.cpp
    test_finance.h
    test_properties.cpp
    test_properties.h
    test_sqlhelper.cpp
    test_sqlhelper.h
    test_statistics.cpp
    test_statistics.h
    ../DKV2/tabledatainserter.cpp
    test_tabledatainserter.cpp
    test_tabledatainserter.h
    test_views.cpp
    test_views.h
    testhelper.cpp
    testhelper.h
)

find_package(Qt6 REQUIRED COMPONENTS Test)

# usage of precompiled header
target_precompile_headers(${TARGET} PRIVATE ../DKV2/pch.h)
set_target_properties(${TARGET} PROPERTIES AUTOGEN_USE_PCH ON)
# If you want to be explicit on MSVC:
if (MSVC)
  target_link_options(Tests PRIVATE /SUBSYSTEM:CONSOLE)
endif()

target_link_libraries(${TARGET} PRIVATE Qt6::Test)
target_link_libraries(${TARGET} PRIVATE Qt6::Core)
target_link_libraries(${TARGET} PRIVATE Qt6::Widgets)
target_link_libraries(${TARGET} PRIVATE Qt6::Sql)
target_link_libraries(${TARGET} PRIVATE Qt6::PrintSupport)
