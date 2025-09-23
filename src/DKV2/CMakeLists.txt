set (TARGET DKV2)

qt_add_executable( ${TARGET} WIN32
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
# usage of precompiled header
target_precompile_headers( ${TARGET} PRIVATE pch.h)
set_target_properties(${TARGET} PROPERTIES AUTOGEN_USE_PCH ON)
set_target_properties(${TARGET} PROPERTIES AUTOGEN_USE_PCH ON)

target_link_libraries(${TARGET} PRIVATE Qt6::Core)
target_link_libraries(${TARGET} PRIVATE Qt6::Widgets)
target_link_libraries(${TARGET} PRIVATE Qt6::Sql)
target_link_libraries(${TARGET} PRIVATE Qt6::PrintSupport)

set_target_properties(${TARGET} PROPERTIES
    WIN22_EXECUTABLE ON
)
