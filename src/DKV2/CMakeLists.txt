set(TARGET DKV2)

# Plattform-Flag (GUI: kein Konsolenfenster unter Windows / macOS)
if(APPLE)
    set(PLATFORM_GUI_FLAG MACOSX_BUNDLE)
elseif(WIN32)
    set(PLATFORM_GUI_FLAG WIN32)
endif()

qt_add_executable(${TARGET} ${PLATFORM_GUI_FLAG}
    CMakeLists.txt
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    mainwindow_contractlist.cpp
    mainwindow_creditorlist.cpp
    mainwindow_mru.cpp
    
    DKV2.rc
    dkv2version.h
    resource.qrc

    pch.h

    appconfig.cpp
    appconfig.h
    booking.cpp
    booking.h
    busycursor.h
    contract.cpp
    contract.h
    contractsheadersortingadapter.cpp
    contractsheadersortingadapter.h
    contracttablemodel.cpp
    contracttablemodel.h
    creditor.cpp
    creditor.h
    csvwriter.cpp
    csvwriter.h
    dbfield.cpp
    dbfield.h
    dbstructure.cpp
    dbstructure.h
    dbtable.cpp
    dbtable.h
    dkdbcopy.cpp
    dkdbcopy.h
    dkdbhelper.cpp
    dkdbhelper.h
    dkdbindices.cpp
    dkdbindices.h
    dkdbviews.cpp
    dkdbviews.h
    DKV2.icns
    dlgabout.cpp
    dlgabout.h
    dlgannualsettlement.cpp
    dlgannualsettlement.h
    dlgaskcontractlabel.cpp
    dlgaskcontractlabel.h
    dlgaskdate.cpp
    dlgaskdate.h
    dlgchangebooking.cpp
    dlgchangebooking.h
    dlgchangecontracttermination.cpp
    dlgchangecontracttermination.h
    dlgdisplaycolumns.cpp
    dlgdisplaycolumns.h
    dlginterestletters.cpp
    dlginterestletters.h
    filewriter.cpp
    filewriter.h
    helper.cpp
    helper.h
    helperfile.cpp
    helperfile.h
    helperfin.cpp
    helperfin.h
    helpersql.cpp
    helpersql.h
    ibanvalidator.cpp
    ibanvalidator.h
    Info.plist
    investment.cpp
    investment.h
    mustache.cpp
    mustache.h
    opendatabase.cpp
    opendatabase.h
    tabledatainserter.cpp
    tabledatainserter.h
    transaktionen.cpp
    transaktionen.h
    uebersichten.cpp
    uebersichten.h
    uiitemformatter.cpp
    uiitemformatter.h
    wizactivatecontract.cpp
    wizactivatecontract.h
    wizcancelcontract.cpp
    wizcancelcontract.h
    wizchangecontractvalue.cpp
    wizchangecontractvalue.h
    wiznew.cpp
    wiznew.h
    wiznewdatabase.cpp
    wiznewdatabase.h
    wiznewinvestment.cpp
    wiznewinvestment.h
    wizopenornewdatabase.cpp
    wizopenornewdatabase.h
    wizterminatecontract.cpp
    wizterminatecontract.h
)

target_compile_definitions(DKV2 PRIVATE
  $<$<CONFIG:Debug>:DKV2_DEBUG>
)

if(WIN32)
    install(TARGETS DKV2 RUNTIME DESTINATION .)
elseif(APPLE)
    install(TARGETS DKV2 BUNDLE DESTINATION .)
else()
    install(TARGETS DKV2 RUNTIME DESTINATION bin)
endif()


target_precompile_headers(${TARGET} PRIVATE pch.h)
set_target_properties(${TARGET} PROPERTIES AUTOGEN_USE_PCH ON)

target_link_libraries(${TARGET} PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Sql Qt6::PrintSupport
)

# Plattform-spezifische Eigenschaften
if(WIN32)
    set_target_properties(${TARGET} PROPERTIES WIN32_EXECUTABLE ON)
elseif(APPLE)
    set_target_properties(${TARGET} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        MACOSX_BUNDLE_BUNDLE_NAME "DKV2"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.dkv2"
    )
    install(TARGETS ${TARGET} BUNDLE DESTINATION .)
else()
    install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
endif()
