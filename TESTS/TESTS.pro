QT += testlib
QT += sql

CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES +=  tst_db.cpp \
    ../DKV2/dkdbhelper.cpp \
    ../DKV2/finhelper.cpp \
    ../DKV2/kreditor.cpp \
    ../DKV2/sqlhelper.cpp \
    ../DKV2/vertrag.cpp \
    ../dkv2/dbfield.cpp \
    ../dkv2/dbstructure.cpp \
    ../dkv2/dbtable.cpp \
    ../dkv2/filehelper.cpp \
    ../dkv2/helper.cpp \
    test_dkdbhelper.cpp \
    test_finance.cpp \
    test_main.cpp \
    testhelper.cpp

HEADERS += \
    test_dkdbhelper.h \
    test_finance.h \
    testhelper.h \
    tst_db.h
