cmake_minimum_required(VERSION 3.21)
project(ClientOrders VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)
qt_add_executable(ClientOrders
    src/main.cpp
    src/mainwindow.h src/mainwindow.cpp
    src/database.h src/database.cpp
    src/orderdialog.h src/orderdialog.cpp
)
target_link_libraries(ClientOrders PRIVATE Qt6::Widgets Qt6::Sql)
