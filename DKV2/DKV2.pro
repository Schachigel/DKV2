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

win32:QMAKE_CXXFLAGS += /utf-8
win32:QMAKE_CXXFLAGS += /FS

win32:VERSION = 0.0.0.2 # major.minor.patch.build
else:VERSION = 0.0.0.2    # major.minor.patch
VERSION_PE_HEADER = 0.2
VERSION = 0.0.0.2
QMAKE_TARGET_COMPANY = HoMSoft
QMAKE_TARGET_PRODUCT = DKV2 Direktkredit Verwaltung
QMAKE_TARGET_DESCRIPTION = DK Verwaltung f. MHS Projekte
RC_CODEPAGE = 850
RC_LANG = 0x0407


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
        dkdbhelper.cpp \
        helper.cpp \
        helperfile.cpp \
        helperfin.cpp \
        helpersql.cpp \
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
        wizterminatecontract.cpp

HEADERS += \
        appconfig.h \
        booking.h \
        contract.h \
        creditor.h \
        csvwriter.h \
        dbfield.h \
        dbstatistics.h \
        dbstructure.h \
        dbtable.h \
        dkdbhelper.h \
        helper.h \
        helperfile.h \
        helperfin.h \
        helpersql.h \
        letterTemplate.h \
        letters.h \
        mainwindow.h \
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
