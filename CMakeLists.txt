cmake_minimum_required(VERSION 3.16)

if(WIN32)
    set(MAVSDK_DIR ${CMAKE_SOURCE_DIR}/Depends/lib/cmake/MAVSDK)
endif()

project(MiniGCS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MAVSDK REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS
    Core
    SerialPort
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc/Link
)

# 设置源文件
set(SOURCES
    main.cpp
    Src/Extern/XmlToMavSDK.cpp
    Src/Link/QSerialDataLink.cpp
    Src/QVehicle.cpp
    Src/QGroundControlStation.cpp
    Src/Private/QGroundControlStationPrivate.cpp
    Src/Private/QVehiclePrivate.cpp
)

set(HEADERS
    Inc/Extern/XmlToMavSDK.h
    Inc/Link/QDataLink.h
    Inc/Link/QSerialDataLink.h
    Inc/QGroundControlStation.h
    Inc/QVehicle.h
    Inc/Private/QGroundControlStationPrivate.h
    Inc/Private/QVehiclePrivate.h
)

add_executable(${PROJECT_NAME} 
    ${SOURCES}
    ${HEADERS}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4828)
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    MAVSDK::mavsdk
    Qt6::Core
    Qt6::SerialPort
)
