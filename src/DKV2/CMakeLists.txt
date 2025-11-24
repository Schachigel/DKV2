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

# Create executable
qt_add_executable(${TARGET} ${PLATFORM_GUI_FLAG} ${DKV2_SOURCES})

# Compile definitions
target_compile_definitions(${TARGET} PRIVATE
    $<$<CONFIG:Debug>:DKV2_DEBUG>
    QT_DEPRECATED_WARNINGS           # Warn about deprecated Qt features
    QT_DISABLE_DEPRECATED_BEFORE=0x060000  # Disable features deprecated before Qt 6.0.0
)

# Apply project warnings (defined in root CMakeLists.txt)
if(COMMAND set_project_warnings)
    set_project_warnings(${TARGET})
endif()

# Enable sanitizers for Debug builds (defined in root CMakeLists.txt)
if(COMMAND enable_sanitizers)
    enable_sanitizers(${TARGET})
endif()

# Precompiled headers
target_precompile_headers(${TARGET} PRIVATE pch.h)
set_target_properties(${TARGET} PROPERTIES 
    AUTOGEN_USE_PCH ON
    # Disable automatic MOC for files that don't need it (performance optimization)
    AUTOMOC ON
    AUTOUIC ON
)

# Link libraries
target_link_libraries(${TARGET} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
)

# Platform-specific properties
if(WIN32)
    set_target_properties(${TARGET} PROPERTIES 
        WIN32_EXECUTABLE ON
        # Add version info to executable
        VERSION ${PROJECT_VERSION}
    )
    
    # Windows-specific: Set subsystem to Windows (GUI app, not console)
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
        # Enable high DPI support on macOS
        MACOSX_BUNDLE_INFO_STRING "Direktkredit Verwaltung"
    )
    
    # Sign the app bundle for macOS (ad-hoc signing)
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND codesign --force --deep --sign - $<TARGET_BUNDLE_DIR:${TARGET}>
        COMMENT "Ad-hoc signing macOS bundle"
        VERBATIM
    )
else()
    # Linux-specific settings
    set_target_properties(${TARGET} PROPERTIES
        VERSION ${PROJECT_VERSION}
        OUTPUT_NAME "dkv2"
    )
endif()

# Installation rules with proper error handling
if(WIN32)
    install(TARGETS ${TARGET} 
        RUNTIME DESTINATION .
        COMPONENT Application
    )
    
    # Deploy Qt dependencies on Windows
    if(Qt6_FOUND)
        install(CODE "
            include(\"${Qt6_DIR}/Qt6CoreDeployment.cmake\")
            qt_deploy_runtime_dependencies(
                EXECUTABLE \"\${CMAKE_INSTALL_PREFIX}/${TARGET}.exe\"
                GENERATE_QT_CONF
                VERBOSE
            )
        " COMPONENT Application)
    endif()
    
elseif(APPLE)
    install(TARGETS ${TARGET} 
        BUNDLE DESTINATION .
        COMPONENT Application
    )
    
    # Deploy Qt dependencies on macOS
    if(Qt6_FOUND)
        install(CODE "
            include(\"${Qt6_DIR}/Qt6CoreDeployment.cmake\")
            qt_deploy_runtime_dependencies(
                EXECUTABLE \"\${CMAKE_INSTALL_PREFIX}/${TARGET}.app\"
                GENERATE_QT_CONF
                VERBOSE
            )
        " COMPONENT Application)
    endif()
    
else()
    # Linux installation
    install(TARGETS ${TARGET} 
        RUNTIME DESTINATION bin
        COMPONENT Application
    )
    
    # Install desktop file and icon (if they exist)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dkv2.desktop")
        install(FILES dkv2.desktop 
            DESTINATION share/applications
            COMPONENT Application
        )
    endif()
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dkv2.png")
        install(FILES dkv2.png 
            DESTINATION share/icons/hicolor/256x256/apps
            COMPONENT Application
        )
    endif()
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "DKV2 Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Qt Version: ${Qt6_VERSION}")
if(WIN32)
    message(STATUS "  Platform: Windows")
elseif(APPLE)
    message(STATUS "  Platform: macOS (deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET})")
else()
    message(STATUS "  Platform: Linux/Unix")
endif()
message(STATUS "")