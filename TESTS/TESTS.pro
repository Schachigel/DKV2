# Project created by me

QT += core testlib
QT += sql printsupport

TARGET = tests

TEMPLATE = app

CONFIG += qt warn_on testcase
# CONFIG += depend_includepath # by default anyways

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES *= QT_USE_QSTRINGBUILDER
#DEFINES += Q_QDOC

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#CONFIG += c++11
CONFIG += c++14
#win32:QMAKE_CXXFLAGS += /utf-8
#win32:QMAKE_CXXFLAGS += /FS
CONFIG += utf8_source

DEFINES += GIT_COMMIT='\\"$$system(git log --format="%h" -n 1)\\"'

SOURCES +=  \
    ../DKV2/appconfig.cpp \
    ../DKV2/booking.cpp \
    ../DKV2/contract.cpp \
    ../DKV2/creditor.cpp \
    ../DKV2/dkdbcopy.cpp \
    ../DKV2/dkdbhelper.cpp \
    ../DKV2/dkdbviews.cpp \
    ../DKV2/helperfin.cpp \
    ../DKV2/ibanvalidator.cpp \
    ../DKV2/investment.cpp \
    ../DKV2/helpersql.cpp \
    ../DKV2/csvwriter.cpp \
    ../DKV2/dbfield.cpp \
    ../DKV2/dbstructure.cpp \
    ../DKV2/dbtable.cpp \
    ../DKV2/helperfile.cpp \
    ../DKV2/helper.cpp \
    ../DKV2/letters.cpp \
    ../DKV2/lettersnippets.cpp \
    ../DKV2/tabledatainserter.cpp \
    financaltimespan.cpp \
    test_appconfig.cpp \
    test_booking.cpp \
    test_contract.cpp \
    test_creditor.cpp \
    test_csv.cpp \
    test_db.cpp \
    test_dkdbcopy.cpp \
    test_dkdbhelper.cpp \
    test_finance.cpp \
    test_lettersnippet.cpp \
    test_main.cpp \
    test_properties.cpp \
    test_sqlhelper.cpp \
    test_statistics.cpp \
    test_tabledatainserter.cpp \
    test_views.cpp \
    testhelper.cpp

#use precompiled header
PRECOMPILED_HEADER  = ../DKV2/pch.h

HEADERS += \
    ../DKV2/ibanvalidator.h \
    ../DKV2/letters.h \
    financaltimespan.h \
    test_appconfig.h \
    test_booking.h \
    test_contract.h \
    test_creditor.h \
    test_csv.h \
    test_db.h \
    test_dkdbcopy.h \
    test_dkdbhelper.h \
    test_finance.h \
    test_lettersnippet.h \
    test_properties.h \
    test_sqlhelper.h \
    test_statistics.h \
    test_tabledatainserter.h \
    test_views.h \
    testhelper.h

RESOURCES += \
    resource.qrc
