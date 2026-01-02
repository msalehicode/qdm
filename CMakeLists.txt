cmake_minimum_required(VERSION 3.16)

project(qmldownloadManager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Network)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appqmldownloadManager
    main.cpp
)

qt_add_qml_module(appqmldownloadManager
    URI qmldownloadManager
    VERSION 1.0
    QML_FILES
        Main.qml
        QML_FILES
        SOURCES downloadmanager.h downloadmanager.cpp
        SOURCES downloaditem.h downloaditem.cpp
        SOURCES javahelper.h javahelper.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appqmldownloadManager PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appiconandroidapp
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/android
    QT_ANDROID_PACKAGE_NAME "org.qtproject.qmldownloadmanager"
    QT_ANDROID_APP_NAME "QML DM"
    # QT_ANDROID_APP_ICON "@mipmap/ic_launcher"
)

target_link_libraries(appqmldownloadManager
    PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Network
)

include(GNUInstallDirs)
install(TARGETS appqmldownloadManager
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(ANDROID)
    install(FILES
        ${CMAKE_SOURCE_DIR}/android/libs/arm64-v8a/libssl.so
        ${CMAKE_SOURCE_DIR}/android/libs/arm64-v8a/libcrypto.so
        DESTINATION lib/${ANDROID_ABI}
    )
endif()
