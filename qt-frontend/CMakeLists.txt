cmake_minimum_required(VERSION 3.16)
project(CINEBASE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    Network
    Concurrent
    REQUIRED
)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/apimanager.h
    src/apimanager.cpp
    src/videolibrary.h
    src/videolibrary.cpp
)

add_executable(CINEBASE ${PROJECT_SOURCES})

target_link_libraries(CINEBASE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

set_target_properties(CINEBASE PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    BUILD_RPATH "/usr/lib/x86_64-linux-gnu:/usr/lib:/lib/x86_64-linux-gnu:/lib"
    INSTALL_RPATH "/usr/lib/x86_64-linux-gnu:/usr/lib:/lib/x86_64-linux-gnu:/lib"
)
