cmake_minimum_required(VERSION 3.16)

if(WIN32)
    set(MAVSDK_DIR ${CMAKE_SOURCE_DIR}/Depends/mavsdk/lib/cmake/MAVSDK)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(spdlog_DIR ${CMAKE_SOURCE_DIR}/Depends/spdlog/x64-Debug/lib/cmake/spdlog)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(spdlog_DIR ${CMAKE_SOURCE_DIR}/Depends/spdlog/x64-Release/lib/cmake/spdlog)
    endif()
endif()

project(MiniGCS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MAVSDK REQUIRED)
find_package(spdlog REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Quick
    SerialPort
    Location
)

qt_standard_project_setup(REQUIRES 6.9)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
)

# 设置源文件
set(SOURCES
    main.cpp
    Src/AirLine/QAirLine.cpp
    Src/AirLine/QAirLineManager.cpp
    Src/AirLine/QDronePosition.cpp
    Src/Extern/XmlToMavSDK.cpp
    Src/Extern/AsyncSendMavLink.cpp
    Src/Link/QSerialDataLink.cpp
    Src/Plat/QAutopilot.cpp
    Src/Plat/QPlat.cpp
    Src/Plat/Private/QAutopilotPrivate.cpp
    Src/Plat/Private/QPlatPrivate.cpp
    Src/Private/QGroundControlStationPrivate.cpp
    Src/QGroundControlStation.cpp
    Src/QGCSConfig.cpp
)

set(HEADERS
    Inc/AirLine/QAirLine.h
    Inc/AirLine/QAirLineManager.h
    Inc/AirLine/QDronePosition.h
    Inc/Extern/XmlToMavSDK.h
    Inc/Extern/AsyncSendMavLink.h
    Inc/Link/QDataLink.h
    Inc/Link/QSerialDataLink.h
    Inc/Plat/QAutopilot.h
    Inc/Plat/QPlat.h
    Inc/Plat/Private/QAutopilotPrivate.h
    Inc/Plat/Private/QPlatPrivate.h
    Inc/Private/QGroundControlStationPrivate.h
    Inc/QGroundControlStation.h
    Inc/QGCSConfig.h
)

qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

qt_add_qml_module(${PROJECT_NAME}
    URI ${PROJECT_NAME}
    QML_FILES
        qml/Main.qml
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
    Qt6::Core
    Qt6::Quick
    Qt6::SerialPort
    Qt6::Location
    MAVSDK::mavsdk
    spdlog::spdlog
)
