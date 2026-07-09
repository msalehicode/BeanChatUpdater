cmake_minimum_required(VERSION 3.16)

project(BeanChatUpdater VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appBeanChatUpdater
    main.cpp
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
        SOURCES updater/downloader.h updater/downloader.cpp
        SOURCES updater/latestresponse.h updater/latestresponse.cpp
        SOURCES updater/filehasher.h updater/filehasher.cpp
        SOURCES updater/fileverifier.h updater/fileverifier.cpp

)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appBeanChatUpdater PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appBeanChatUpdater
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
