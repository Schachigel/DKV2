# Project created by QtCreator 2019-09-04T21:33:31

QT += core
QT += gui widgets sql printsupport

TARGET = DKV2
RC_ICONS = "res/logo.ico"

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES *= QT_DEPRECATED_WARNINGS
DEFINES *= QT_USE_QSTRINGBUILDER
#DEFINES += Q_QDOC

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#CONFIG += c++11
CONFIG *= c++14


# many ways to set utf8 source!!
# this works !! both compilers
CONFIG += utf8_source
#MSVC only SETTINGS
#win32:msvc:QMAKE_CXXFLAGS += /utf-8
#win32:msvc:QMAKE_CXXFLAGS += /FS
# unknown to MSVC:
#QMAKE_CXXFLAGS+=-finput-charset=UTF-8

DEFINES += GIT_COMMIT='\\"$$system(git log --format="%h" -n 1)\\"'

VERSION = 0.0.0.10
VERSION_PE_HEADER = 0.10

QMAKE_TARGET_COMPANY = HoMSoft
QMAKE_TARGET_PRODUCT = DKV2 Direktkredit Verwaltung
QMAKE_TARGET_DESCRIPTION = DK Verwaltung f. MHS Projekte

RC_CODEPAGE = 850
RC_LANG = 0x0407


win32:LIBS += \
        -lVersion

SOURCES += \
        appconfig.cpp \
        booking.cpp \
        contract.cpp \
        creditor.cpp \
        csvwriter.cpp \
        dbfield.cpp \
        dbstatistics.cpp \
        dbstructure.cpp \
        dbtable.cpp \
        dkdbcopy.cpp \
        dkdbhelper.cpp \
        dkdbviews.cpp \
        helper.cpp \
        helperfile.cpp \
        helperfin.cpp \
        helpersql.cpp \
        investment.cpp \
        letterTemplate.cpp \
        letters.cpp \
        main.cpp \
        mainwindow.cpp \
        reporthtml.cpp \
        tabledatainserter.cpp \
        transaktionen.cpp \
        uicustomtoolbutton.cpp \
        uiitemformatter.cpp \
        wizactivatecontract.cpp \
        wizannualsettlement.cpp \
        wizcancelcontract.cpp \
        wizchangecontractvalue.cpp \
        wiznew.cpp \
        wiznewdatabase.cpp \
        wiznewinvestment.cpp \
        wizopenornewdatabase.cpp \
        wizterminatecontract.cpp

#use precompiled header
PRECOMPILED_HEADER  = pch.h

HEADERS += pch.h\
        appconfig.h \
        booking.h \
        contract.h \
        creditor.h \
        csvwriter.h \
        dbfield.h \
        dbstatistics.h \
        dbstructure.h \
        dbtable.h \
        dkdbcopy.h \
        dkdbhelper.h \
        dkdbviews.h \
        helper.h \
        helperfile.h \
        helperfin.h \
        helpersql.h \
        investment.h \
        letterTemplate.h \
        letters.h \
        mainwindow.h \
        pch.h \
        reporthtml.h \
        tabledatainserter.h \
        transaktionen.h \
        uicustomtoolbutton.h \
        uiitemformatter.h \
        wizactivatecontract.h \
        wizannualsettlement.h \
        wizcancelcontract.h \
        wizchangecontractvalue.h \
        wiznew.h \
        wiznewdatabase.h \
        wiznewinvestment.h \
        wizopenornewdatabase.h \
        wizterminatecontract.h

FORMS += \
        mainwindow.ui

#TRANSLATIONS += \
#    translations.ts \
#    qtbase_de.ts

DISTFILES += \
    ../DOCS/Zinsberechnungsmethode.txt \
    ../DOCS/Zinstage.xlsx \
    ../DOCS/remember.txt \
    ../letter.html \
    DOKU \
    remember.txt

RESOURCES += \
    resource.qrc
