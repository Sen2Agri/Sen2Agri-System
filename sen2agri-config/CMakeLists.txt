cmake_minimum_required(VERSION 2.8)

project(sen2agri-config)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pipe")

find_package(Qt5Widgets REQUIRED)

set(UI_FORMS maindialog.ui)

qt5_wrap_ui(UI_HEADERS ${UI_FORMS})

add_executable(${CMAKE_PROJECT_NAME} main.cpp maindialog.cpp configmodel.cpp ${UI_FORMS})
target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Widgets)
