# Project created by QtCreator 2019-09-04T21:33:31

QT += core
QT += gui widgets sql charts printsupport

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

# macx:ICON = $${TARGET}.icns
# macx:QMAKE_INFO_PLIST = Info.plist

APP_TRANSLATIONS_FILES.files = $$[QT_INSTALL_TRANSLATIONS]/qtbase_de.qm
APP_TRANSLATIONS_FILES.path = Contents/MacOS/translations
QMAKE_BUNDLE_DATA += APP_TRANSLATIONS_FILES

# macx:QMAKE_POST_LINK += set -x &&
macx:QMAKE_POST_LINK += echo "starte post link steps..."
# macx:QMAKE_POST_LINK += mkdir -p $${OUT_PWD}/$${TARGET}.app/Contents/MacOS/translations &&
# macx:QMAKE_POST_LINK += cp $$[QT_INSTALL_TRANSLATIONS]/qtbase_de.qm $${OUT_PWD}/$${TARGET}.app/Contents/MacOS/translations/ &&
CONFIG( release, debug|release ){
   macx:QMAKE_POST_LINK += rm $${OUT_PWD}/$${TARGET}.dmg || true &&
   macx:QMAKE_POST_LINK += macdeployqt $${OUT_PWD}/$${TARGET}.app -dmg &&
}
macx:QMAKE_POST_LINK += echo "... beende post link steps"

# RESOURCES += $${TARGET}.qrc
RC_FILE += $${TARGET}.rc

# win32:LIBS += \
#         -lVersion

SOURCES += \
        appconfig.cpp \
        booking.cpp \
        contract.cpp \
        contracttablemodel.cpp \
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
        ibanvalidator.cpp \
        investment.cpp \
        letterTemplate.cpp \
        letters.cpp \
        main.cpp \
        mainwindow.cpp \
        mainwindow_contractlist.cpp \
        reporthtml.cpp \
        tabledatainserter.cpp \
        transaktionen.cpp \
        uiitemformatter.cpp \
        wizEditContractTermination.cpp \
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
        contracttablemodel.h \
        creditor.h \
        csvwriter.h \
        dbfield.h \
        dbstatistics.h \
        dbstructure.h \
        dbtable.h \
        dkdbcopy.h \
        dkdbhelper.h \
        dkdbviews.h \
        dkv2version.h \
        helper.h \
        helperfile.h \
        helperfin.h \
        helpersql.h \
        ibanvalidator.h \
        investment.h \
        letterTemplate.h \
        letters.h \
        mainwindow.h \
        pch.h \
        reporthtml.h \
        tabledatainserter.h \
        transaktionen.h \
        uiitemformatter.h \
        wizEditContractTermination.h \
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
