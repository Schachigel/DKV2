
QT += testlib sql printsupport

CONFIG += qt warn_on depend_includepath testcase
CONFIG += c++11
CONFIG += c++14

TEMPLATE = app

SOURCES +=  tst_db.cpp \
    ../DKV2/dkdbhelper.cpp \
    ../DKV2/finhelper.cpp \
    ../DKV2/kreditor.cpp \
    ../DKV2/letterTemplate.cpp \
    ../DKV2/sqlhelper.cpp \
    ../DKV2/vertrag.cpp \
    ../DKV2/csvwriter.cpp \
    ../DKV2/dbfield.cpp \
    ../DKV2/dbstructure.cpp \
    ../DKV2/dbtable.cpp \
    ../DKV2/filehelper.cpp \
    ../DKV2/helper.cpp \
    test_csv.cpp \
    test_dkdbhelper.cpp \
    test_finance.cpp \
    test_lettertemplate.cpp \
    test_main.cpp \
    test_properties.cpp \
    test_sqlhelper.cpp \
    testhelper.cpp

HEADERS += \
    ../DKV2/finhelper.h \
    test_csv.h \
    test_dkdbhelper.h \
    test_finance.h \
    test_lettertemplate.h \
    test_properties.h \
    test_sqlhelper.h \
    testhelper.h \
    tst_db.h

RESOURCES += \
    resource.qrc
