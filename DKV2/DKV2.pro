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
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += Q_QDOC

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
CONFIG += c++14

SOURCES += \
        activatecontractwiz.cpp \
        appconfig.cpp \
        askdatedlg.cpp \
        booking.cpp \
        changecontractvaluewiz.cpp \
        contract.cpp \
        creditor.cpp \
        csvwriter.cpp \
        customtoolbutton.cpp \
        dbfield.cpp \
        dbstructure.cpp \
        dbtable.cpp \
        dkdbhelper.cpp \
        filehelper.cpp \
        finhelper.cpp \
        frmjahresabschluss.cpp \
        helper.cpp \
        itemformatter.cpp \
        jahresabschluss.cpp \
        letterTemplate.cpp \
        letters.cpp \
        main.cpp \
        mainwindow.cpp \
        sqlhelper.cpp \
        tabledatainserter.cpp \
        transaktionen.cpp

HEADERS += \
        activatecontractwiz.h \
        appconfig.h \
        askdatedlg.h \
        booking.h \
        changecontractvaluewiz.h \
        contract.h \
        creditor.h \
        csvwriter.h \
        customtoolbutton.h \
        dbfield.h \
        dbstructure.h \
        dbtable.h \
        dkdbhelper.h \
        filehelper.h \
        financaltimespan.h \
        finhelper.h \
        frmjahresabschluss.h \
        helper.h \
        itemformatter.h \
        jahresabschluss.h \
        letterTemplate.h \
        letters.h \
        mainwindow.h \
        sqlhelper.h \
        tabledatainserter.h \
        transaktionen.h

FORMS += \
        askDateDlg.ui \
        frmjahresabschluss.ui \
        mainwindow.ui

DISTFILES += \
    ../DOCS/Zinsberechnungsmethode.txt \
    ../DOCS/Zinstage.xlsx \
    ../DOCS/remember.txt \
    ../letter.html \
    DOKU \
    remember.txt

RESOURCES += \
    resource.qrc
