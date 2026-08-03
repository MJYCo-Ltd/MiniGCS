cmake_minimum_required(VERSION 3.16)

if(WIN32)
    set(MAVSDK_DIR "${CMAKE_CURRENT_LIST_DIR}/Depends/mavsdk/lib/cmake/MAVSDK")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(spdlog_DIR "${CMAKE_CURRENT_LIST_DIR}/Depends/spdlog/x64-Debug/lib/cmake/spdlog")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(spdlog_DIR "${CMAKE_CURRENT_LIST_DIR}/Depends/spdlog/x64-Release/lib/cmake/spdlog")
    endif()
endif()

project(MiniGCS 
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DEBUG_POSTFIX d)

find_package(MAVSDK REQUIRED)
find_package(spdlog REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS
    Core
)

qt_standard_project_setup(REQUIRES 6.9)

# 设置源文件
set(MINIGCS_SOURCES
    Src/AirLine/QAirLine.cpp
    Src/AirLine/QAirLineManager.cpp
    Src/AirLine/QMissionPoint.cpp
    Src/Common/QGpsPosition.cpp
    Src/Common/QNEDPosition.cpp
    Src/Common/QAttitude.cpp
    Src/Common/QVelocity.cpp
    Src/Common/QRawGps.cpp
    Src/Plat/QAutopilotStatus.cpp
    Src/Plat/QAutopilotFixedwing.cpp
    Src/Extern/XmlToMavSDK.cpp
    Src/Plat/QAutopilot.cpp
    Src/Plat/QAutoVehicleType.cpp
    Src/Plat/QPlat.cpp
    Src/Plat/Private/QAutopilotPrivate_base.cpp
    Src/Plat/Private/QAutopilotPrivate_control.cpp
    Src/Plat/Private/QPlatPrivate.cpp
    Src/Private/QGroundControlStationPrivate.cpp
    Src/Private/QMavsdkTextCatalog.cpp
    Src/QGroundControlStation.cpp
    Src/Link/QDataLink.cpp
    Src/Link/QLinkManager.cpp
    Src/Link/Private/QLinkManagerPrivate.cpp
    Src/QGCSConfig.cpp
)

set(MINIGCS_HEADERS
    Inc/MiniGCSExport.h
    Inc/AirLine/QAirLine.h
    Inc/AirLine/QAirLineManager.h
    Inc/AirLine/QMissionPoint.h
    Inc/Common/QGpsPosition.h
    Inc/Common/QNEDPosition.h
    Inc/Common/QAttitude.h
    Inc/Common/QVelocity.h
    Inc/Common/QRawGps.h
    Src/Private/QGCSConfigInternal.h
    Inc/AirLine/QGpsPosition.h
    Inc/AirLine/QNEDPosition.h
    Inc/Plat/QAutopilotStatus.h
    Inc/Plat/QAutopilotFixedwing.h
    Inc/Link/QDataLink.h
    Inc/Link/QLinkManager.h
    Src/Link/Private/QLinkManagerPrivate.h
    Inc/Plat/QAutopilot.h
    Inc/Plat/QAutoVehicleType.h
    Inc/Plat/QPlat.h
    Src/Plat/Private/QAutopilotPrivate.h
    Src/Plat/Private/QPlatPrivate.h
    Src/Private/QGroundControlStationPrivate.h
    Src/Private/QMavsdkTextCatalog.h
    Inc/QGroundControlStation.h
    Inc/QGCSConfig.h
    Src/Extern/XmlToMavSDK.h
    Src/Private/QGCSLog.h
)

# 构建 MiniGCS 为动态库
qt_add_library(${PROJECT_NAME} SHARED
    ${MINIGCS_HEADERS}
    ${MINIGCS_SOURCES}
)

# 定义导出宏
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    MINIGCS_LIBRARY
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
    PUBLIC
    Qt6::Core
    PRIVATE
    MAVSDK::mavsdk
    spdlog::spdlog
)

# 设置导出头文件目录
target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Inc>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Src
)

# 安装配置
install(TARGETS ${PROJECT_NAME}
    EXPORT MiniGCSTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# 安装头文件
install(DIRECTORY Inc/
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h"
)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Config")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config/type_text_zh_CN.json"
    "${CMAKE_CURRENT_BINARY_DIR}/Config/type_text_zh_CN.json"
    COPYONLY
)
# 兼容旧配置文件名：与 type_text_zh_CN.json 同源，勿再维护第二份文案
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config/type_text_zh_CN.json"
    "${CMAKE_CURRENT_BINARY_DIR}/Config/mavsdk_zh_CN.json"
    COPYONLY
)

install(FILES
    Config/type_text_zh_CN.json
    DESTINATION bin/Config
)
install(FILES
    Config/type_text_zh_CN.json
    DESTINATION bin/Config
    RENAME mavsdk_zh_CN.json
)

# 安装 CMake 配置文件
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MiniGCSConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/MiniGCSConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MiniGCSConfig.cmake"
    INSTALL_DESTINATION lib/cmake/MiniGCS
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MiniGCSConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MiniGCSConfigVersion.cmake"
    DESTINATION lib/cmake/MiniGCS
)

install(EXPORT MiniGCSTargets
    FILE MiniGCSTargets.cmake
    NAMESPACE MiniGCS::
    DESTINATION lib/cmake/MiniGCS
)

# 可选：构建 QML 演示程序
option(MINIGCS_BUILD_DEMO "Build the MiniGCS QML demo" OFF)
if(MINIGCS_BUILD_DEMO)
    add_subdirectory(Test)
endif()
