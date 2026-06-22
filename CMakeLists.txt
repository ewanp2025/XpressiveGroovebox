cmake_minimum_required(VERSION 3.16)

project(XpressiveGroovebox VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Xml Multimedia)

qt_add_executable(XpressiveGroovebox
    main.cpp
    mainwindow.cpp
    mainwindow.h
    synthengine.cpp
    synthengine.h
    xpressive_parser.cpp
    xpressive_parser.h
    instrument.cpp
    instrument.h
    effect.cpp
    effect.h
    pianoroll.cpp
    pianoroll.h
    automation.cpp
    automation.h
)

target_link_libraries(XpressiveGroovebox PRIVATE Qt6::Widgets Qt6::Xml Qt6::Multimedia)

if(ANDROID)
    set_target_properties(XpressiveGroovebox PROPERTIES
        QT_ANDROID_PACKAGE_NAME "com.xpressive.groovebox"
        QT_ANDROID_APP_NAME "Xpressive Groovebox"
        QT_ANDROID_MIN_SDK_VERSION 28
        QT_ANDROID_VERSION_NAME "1.0.0"
        QT_ANDROID_VERSION_CODE "1"
        QT_ANDROID_TARGET_SDK_VERSION 34
        QT_ANDROID_COMPILE_SDK_VERSION 34
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )
endif()