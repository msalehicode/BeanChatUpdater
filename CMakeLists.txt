cmake_minimum_required(VERSION 3.16)

project(BeanChatUpdater VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(UPDATER_VERSION 1.0.0)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appBeanChatUpdater
    main.cpp
    app.rc #windows icon for .exe
)


qt_add_resources(appBeanChatUpdater resources
    PREFIX "/"
    FILES
        icons/BeanChat.png
        icons/BeanChat.ico
)


qt_add_qml_module(appBeanChatUpdater
    URI BeanChatUpdater
    VERSION 1.0
    QML_FILES
        ui/Main.qml



    SOURCES
        updater/updater.h updater/updater.cpp
        updater/arguments.h updater/arguments.cpp
        updater/manifest.h updater/manifest.cpp
        updater/manifestfile.h
        updater/downloader.h updater/downloader.cpp
        updater/latestresponse.h updater/latestresponse.cpp
        updater/filehasher.h updater/filehasher.cpp
        updater/fileverifier.h updater/fileverifier.cpp

)


set_target_properties(appBeanChatUpdater PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appBeanChatUpdater
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appBeanChatUpdater
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)




# ==========================================================
# Windows deployment
# ==========================================================
if(WIN32)
    get_target_property(WINDEPLOYQT_EXECUTABLE Qt6::windeployqt IMPORTED_LOCATION)

    set(DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy/BeanChatUpdater-${UPDATER_VERSION}-Windows")

    add_custom_command(TARGET appBeanChatUpdater POST_BUILD

        # recreate deployment directory
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${DEPLOY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_DIR}"

        # copy executable
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:appBeanChatUpdater>"
            "${DEPLOY_DIR}"

        # deploy Qt
        COMMAND
            "${WINDEPLOYQT_EXECUTABLE}"
            --release
            --qmldir "${CMAKE_SOURCE_DIR}"
            "${DEPLOY_DIR}/$<TARGET_FILE_NAME:appBeanChatUpdater>"


        COMMENT "Deploying BeanChatUpdater..."
    )

endif()


# to deploy
qt_generate_deploy_app_script(
    TARGET appBeanChatUpdater
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

