set(APP_TARGET DKV2)
set(CORE_TARGET dkv2_core)
set(UI_TARGET   dkv2_ui)

# --- Core (ohne Widgets) ---
set(DKV2_CORE_SOURCES
    # Alle NICHT-UI Klassen rein
    pch_core.h

    helper_core.cpp helper_core.h
    helperfile.cpp helperfile.h
    helperfin.cpp helperfin.h
    helpersql.cpp helpersql.h


    booking.cpp booking.h
    contract.cpp contract.h
    creditor.cpp creditor.h
    csvwriter.cpp csvwriter.h
    dbfield.cpp dbfield.h
    dbstructure.cpp dbstructure.h
    dbtable.cpp dbtable.h
    dkdbcopy.cpp dkdbcopy.h

    dkdbindices.cpp dkdbindices.h
    dkdbviews.cpp dkdbviews.h
    filewriter.cpp filewriter.h

    investment.cpp investment.h
    mustache.cpp mustache.h

    tabledatainserter.cpp tabledatainserter.h

    annualSettlement.cpp annualSettlement.h
)
add_library(${CORE_TARGET} STATIC
    ${DKV2_CORE_SOURCES}
    appconfig.h
    appconfig.cpp
    dkdbhelper.cpp
    dkdbhelper.h
)

target_precompile_headers(${CORE_TARGET} PRIVATE pch_core.h)
set_target_properties(${CORE_TARGET} PROPERTIES AUTOGEN_USE_PCH ON)

target_include_directories(${CORE_TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${CORE_TARGET} PUBLIC
    Qt6::Core
    Qt6::Sql
    Qt6::Gui
)

# Falls Core wirklich PrintSupport nutzt (eher selten), dann hier ergänzen:
target_link_libraries(${CORE_TARGET} PUBLIC Qt6::PrintSupport)

# --- UI (Widgets/Dialoge/Wizards) ---
set(DKV2_UI_SOURCES
    pch.h
    helper.cpp helper.h

    mainwindow.cpp mainwindow.h mainwindow.ui
    mainwindow_contractlist.cpp
    mainwindow_creditorlist.cpp
    mainwindow_mru.cpp

    dlgabout.cpp dlgabout.h
    dlgannualsettlement.cpp dlgannualsettlement.h
    dlgaskcontractlabel.cpp dlgaskcontractlabel.h
    dlgaskdate.cpp dlgaskdate.h
    dlgchangebooking.cpp dlgchangebooking.h
    dlgchangecontracttermination.cpp dlgchangecontracttermination.h
    dlgdisplaycolumns.cpp dlgdisplaycolumns.h
    dlginterestletters.cpp dlginterestletters.h

    wizactivatecontract.cpp wizactivatecontract.h
    wizcancelcontract.cpp wizcancelcontract.h
    wizchangecontractvalue.cpp wizchangecontractvalue.h
    wiznew.cpp wiznew.h
    wiznewdatabase.cpp wiznewdatabase.h
    wiznewinvestment.cpp wiznewinvestment.h
    wizopenornewdatabase.cpp wizopenornewdatabase.h
    wizterminatecontract.cpp wizterminatecontract.h
    transaktionen.cpp transaktionen.h

    uiitemformatter.cpp uiitemformatter.h
    resource.qrc
)

add_library(${UI_TARGET} STATIC
    ${DKV2_UI_SOURCES}
    opendatabase.cpp
    opendatabase.h


    ibanvalidator.h
    ibanvalidator.cpp
    uebersichten.h
    uebersichten.cpp
    busycursor.h


    contractsheadersortingadapter.h
    contractsheadersortingadapter.cpp
    dkv2version.h
    contracttablemodel.cpp
    contracttablemodel.h
)

target_precompile_headers(${UI_TARGET} PRIVATE pch.h)
set_target_properties(${UI_TARGET} PROPERTIES AUTOGEN_USE_PCH ON)

target_include_directories(${UI_TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${UI_TARGET} PUBLIC
    ${CORE_TARGET}
    Qt6::Widgets
    Qt6::PrintSupport
)

# --- Executable: nur main + Plattformwrapper ---
set(DKV2_APP_SOURCES
    main.cpp
)

if(WIN32)
    list(APPEND DKV2_APP_SOURCES DKV2.rc)
elseif(APPLE)
    list(APPEND DKV2_APP_SOURCES DKV2.icns Info.plist)
endif()

if(APPLE)
    set(PLATFORM_GUI_FLAG MACOSX_BUNDLE)
elseif(WIN32)
    set(PLATFORM_GUI_FLAG WIN32)
endif()

qt_add_executable(${APP_TARGET} ${PLATFORM_GUI_FLAG}
    ${DKV2_APP_SOURCES}
)

# Link: GUI (die wiederum Core zieht, wenn UI_TARGET PUBLIC gegen CORE_TARGET linkt)
target_link_libraries(${APP_TARGET} PRIVATE
    ${UI_TARGET}
)

# PCH: nach dem Anlegen des Targets
target_precompile_headers(${APP_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/pch.h
)

# Optional (meist nicht nötig, aber ok)
set_target_properties(${APP_TARGET} PROPERTIES AUTOGEN_USE_PCH ON)


# Installation/Bundle-Properties bleiben am Executable wie bisher.
