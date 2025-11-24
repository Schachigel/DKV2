set(TARGET DKV2)

# Source files
set(DKV2_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    mainwindow_contractlist.cpp
    mainwindow_creditorlist.cpp
    mainwindow_mru.cpp

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

# Platform-specific sources
if(WIN32)
    list(APPEND DKV2_SOURCES DKV2.rc)
elseif(APPLE)
    list(APPEND DKV2_SOURCES DKV2.icns Info.plist)
endif()

# Platform-specific GUI flag
if(APPLE)
    set(PLATFORM_GUI_FLAG MACOSX_BUNDLE)
elseif(WIN32)
    set(PLATFORM_GUI_FLAG WIN32)
endif()

qt_add_executable(${TARGET} ${PLATFORM_GUI_FLAG} ${DKV2_SOURCES})

target_compile_definitions(${TARGET} PRIVATE
    $<$<CONFIG:Debug>:DKV2_DEBUG>
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
)

if(COMMAND set_project_warnings)
    set_project_warnings(${TARGET})
endif()

if(COMMAND enable_sanitizers)
    enable_sanitizers(${TARGET})
endif()

target_precompile_headers(${TARGET} PRIVATE pch.h)
set_target_properties(${TARGET} PROPERTIES
    AUTOGEN_USE_PCH ON
    AUTOMOC ON
    AUTOUIC ON
)

target_link_libraries(${TARGET} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
)

# Platform-specific settings
if(WIN32)
    set_target_properties(${TARGET} PROPERTIES
        WIN32_EXECUTABLE ON
        VERSION ${PROJECT_VERSION}
    )
    if(MSVC)
        target_link_options(${TARGET} PRIVATE /SUBSYSTEM:WINDOWS)
    endif()

elseif(APPLE)
    set_target_properties(${TARGET} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        MACOSX_BUNDLE_BUNDLE_NAME "DKV2"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.dkv2"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_INFO_STRING "Direktkredit Verwaltung"
    )

    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND codesign --force --deep --sign - $<TARGET_BUNDLE_DIR:${TARGET}>
        COMMENT "Ad-hoc signing macOS bundle"
        VERBATIM
    )

else()
    set_target_properties(${TARGET} PROPERTIES
        VERSION ${PROJECT_VERSION}
        OUTPUT_NAME "dkv2"
    )
endif()

# Installation Rules
if(WIN32)
    install(TARGETS ${TARGET}
        RUNTIME DESTINATION .
    )

elseif(APPLE)
    install(TARGETS ${TARGET}
        BUNDLE DESTINATION .
    )

    # <<< FIX: macOS-Deployment erfolgt extern im Workflow >>>
    message(STATUS "Note: macOS Qt deployment is handled via macdeployqt in GitHub Actions.")

else()
    install(TARGETS ${TARGET}
        RUNTIME DESTINATION bin
    )
endif()

