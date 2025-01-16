cmake_minimum_required(VERSION 3.5)

project(Lab7-ClientQtKiosk)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network Charts)

set(SOURCES
    main.cpp
    mainwindow.cpp
)

set(HEADERS
    mainwindow.h
)

set(UI_FILES
    mainwindow.ui
)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})

add_executable(main ${SOURCES} ${HEADERS} ${UI_HEADERS})

target_link_libraries(main Qt5::Widgets Qt5::Network Qt5::Charts)

set(CMAKE_PREFIX_PATH "/path/to/qt5")