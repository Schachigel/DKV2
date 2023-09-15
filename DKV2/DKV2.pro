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
CONFIG *= c++17


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
macx:QMAKE_INFO_PLIST = Info.plist

APP_TRANSLATIONS_FILES.files = $$[QT_INSTALL_TRANSLATIONS]/qtbase_de.qm
APP_TRANSLATIONS_FILES.path = Contents/MacOS/translations
QMAKE_BUNDLE_DATA += APP_TRANSLATIONS_FILES

# macx:QMAKE_PRE_LINK += set -x &&
macx:QMAKE_PRE_LINK += echo "starte pre link steps..."  &&
!exists($${PWD}/$${TARGET}.icns) {
   macx:QMAKE_PRE_LINK += mkdir -p $${OUT_PWD}/DKV2.iconset  &&
   macx:QMAKE_PRE_LINK += sips -z 16 16  $${PWD}/res/logo256.png --out $${OUT_PWD}/DKV2.iconset/icon_16x16.png  &&
   macx:QMAKE_PRE_LINK += sips -z 32 32  $${PWD}/res/logo256.png --out $${OUT_PWD}/DKV2.iconset/icon_16x16@2x.png  &&
   macx:QMAKE_PRE_LINK += sips -z 32 32  $${PWD}/res/logo256.png --out $${OUT_PWD}/DKV2.iconset/icon_32x32.png  &&
   macx:QMAKE_PRE_LINK += sips -z 64 64  $${PWD}/res/logo256.png --out $${OUT_PWD}/DKV2.iconset/icon_32x32@2x.png  &&
   macx:QMAKE_PRE_LINK += sips -z 64 64  $${PWD}/res/logo256.png --out $${OUT_PWD}/DKV2.iconset/icon_64x64.png  &&
   macx:QMAKE_PRE_LINK += sips -z 128 128  $${PWD}/res/logo256.png --out $${OUT_PWD}/DKV2.iconset/icon_64x64@2x.png  &&
   macx:QMAKE_PRE_LINK += sips -z 128 128  $${PWD}/res/logo256.png --out $${OUT_PWD}/DKV2.iconset/icon_128x128.png  &&
   macx:QMAKE_PRE_LINK += sips -z 128 128  $${PWD}/res/logo256.png --out $${OUT_PWD}/DKV2.iconset/icon_128x128@2x.png &&
   macx:QMAKE_PRE_LINK += sips -z 256 256  $${PWD}/res/logo256.png --out $${OUT_PWD}/DKV2.iconset/icon_256x256.png  &&
   macx:QMAKE_PRE_LINK += iconutil -c icns $${OUT_PWD}/DKV2.iconset  &&
   macx:QMAKE_PRE_LINK += rm -R $${OUT_PWD}/DKV2.iconset &&
   macx:QMAKE_PRE_LINK += cp $${OUT_PWD}/$${TARGET}.icns $${PWD}/ &&
}
macx:QMAKE_PRE_LINK += echo "... beende pre link steps"

# macx:QMAKE_POST_LINK += set -x &&
macx:QMAKE_POST_LINK += echo "starte post link steps..." &&
macx:QMAKE_POST_LINK += cp $${PWD}/$${TARGET}.icns $${OUT_PWD}/$${TARGET}.app/Contents/Resources &&
# macx:QMAKE_POST_LINK += mkdir -p $${OUT_PWD}/$${TARGET}.app/Contents/MacOS/translations &&
# macx:QMAKE_POST_LINK += cp $$[QT_INSTALL_TRANSLATIONS]/qtbase_de.qm $${OUT_PWD}/$${TARGET}.app/Contents/MacOS/translations/ &&
CONFIG( release, debug|release ){
   macx:QMAKE_POST_LINK += rm $${OUT_PWD}/$${TARGET}.dmg || true &&
   macx:QMAKE_POST_LINK += cd $${OUT_PWD} &&
   macx:QMAKE_POST_LINK += macdeployqt $${TARGET}.app -dmg &&
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
        contractsheadersortingadapter.cpp \
        contracttablemodel.cpp \
        creditor.cpp \
        csvwriter.cpp \
        dbfield.cpp \
        dbstructure.cpp \
        dbtable.cpp \
        dkdbcopy.cpp \
        dkdbhelper.cpp \
        dkdbindices.cpp \
        dkdbviews.cpp \
        dlgannualsettlement.cpp \
        dlgaskcontractlabel.cpp \
        dlgaskdate.cpp \
        dlgchangecontracttermination.cpp \
        dlgdisplaycolumns.cpp \
        dlginterestletters.cpp \
        filewriter.cpp \
        helper.cpp \
        helperfile.cpp \
        helperfin.cpp \
        helpersql.cpp \
        ibanvalidator.cpp \
        investment.cpp \
        main.cpp \
        mainwindow.cpp \
        mainwindow_contractlist.cpp \
        mainwindow_creditorlist.cpp \
        mainwindow_mru.cpp \
        mustache.cpp \
        opendatabase.cpp \
        tabledatainserter.cpp \
        transaktionen.cpp \
        uebersichten.cpp \
        uiitemformatter.cpp \
        wizactivatecontract.cpp \
        wizcancelcontract.cpp \
        wizchangecontractvalue.cpp \
        wiznew.cpp \
        wiznewdatabase.cpp \
        wiznewinvestment.cpp \
        wizopenornewdatabase.cpp \
        wizterminatecontract.cpp

#use precompiled header
PRECOMPILED_HEADER  = pch.h
CONFIG += precompile_header

HEADERS += pch.h\
        appconfig.h \
        booking.h \
        busycursor.h \
        contract.h \
        contractsheadersortingadapter.h \
        contracttablemodel.h \
        creditor.h \
        csvwriter.h \
        dbfield.h \
        dbstructure.h \
        dbtable.h \
        dkdbcopy.h \
        dkdbhelper.h \
        dkdbindices.h \
        dkdbviews.h \
        dkv2version.h \
        dlgannualsettlement.h \
        dlgaskcontractlabel.h \
        dlgaskdate.h \
        dlgchangecontracttermination.h \
        dlgdisplaycolumns.h \
        dlginterestletters.h \
        filewriter.h \
        helper.h \
        helperfile.h \
        helperfin.h \
        helpersql.h \
        ibanvalidator.h \
        investment.h \
        mainwindow.h \
        mustache.h \
        opendatabase.h \
        pch.h \
        tabledatainserter.h \
        transaktionen.h \
        uebersichten.h \
        uiitemformatter.h \
        wizactivatecontract.h \
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
