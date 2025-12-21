cmake_minimum_required(VERSION 3.16)

if(WIN32)
    set(MAVSDK_DIR ${CMAKE_SOURCE_DIR}/Depends/mavsdk/lib/cmake/MAVSDK)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(spdlog_DIR ${CMAKE_SOURCE_DIR}/Depends/spdlog/x64-Debug/lib/cmake/spdlog)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(spdlog_DIR ${CMAKE_SOURCE_DIR}/Depends/spdlog/x64-Release/lib/cmake/spdlog)
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
    Qml
    SerialPort
)

qt_standard_project_setup(REQUIRES 6.9)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
)

# 设置源文件
set(MINIGCS_SOURCES
    Src/AirLine/QAirLine.cpp
    Src/AirLine/QAirLineManager.cpp
    Src/AirLine/QGpsPosition.cpp
    Src/AirLine/QNEDPosition.cpp
    Src/Plat/QAutopilotStatus.cpp
    Src/Extern/XmlToMavSDK.cpp
    Src/Extern/AsyncSendMavLink.cpp
    Src/Plat/QAutopilot.cpp
    Src/Plat/QAutoVehicleType.cpp
    Src/Plat/QPlat.cpp
    Src/Plat/Private/QAutopilotPrivate_base.cpp
    Src/Plat/Private/QAutopilotPrivate_control.cpp
    Src/Plat/Private/QPlatPrivate.cpp
    Src/Private/QGroundControlStationPrivate.cpp
    Src/QGroundControlStation.cpp
    Src/QGCSConfig.cpp
)

set(MINIGCS_HEADERS
    Inc/MiniGCSExport.h
    Inc/AirLine/QAirLine.h
    Inc/AirLine/QAirLineManager.h
    Inc/AirLine/QGpsPosition.h
    Inc/AirLine/QNEDPosition.h
    Inc/Plat/QAutopilotStatus.h
    Inc/Extern/XmlToMavSDK.h
    Inc/Extern/AsyncSendMavLink.h
    Inc/Link/QDataLink.h
    Inc/Plat/QAutopilot.h
    Inc/Plat/QAutoVehicleType.h
    Inc/Plat/QPlat.h
    Inc/Plat/Private/QAutopilotPrivate.h
    Inc/Plat/Private/QPlatPrivate.h
    Inc/Private/QGroundControlStationPrivate.h
    Inc/QGroundControlStation.h
    Inc/QGCSConfig.h
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
    Qt6::Qml
    Qt6::SerialPort
    MAVSDK::mavsdk
    spdlog::spdlog
)

# 设置导出头文件目录
target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Inc>
    $<INSTALL_INTERFACE:include>
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
    PATTERN "Private" EXCLUDE
    PATTERN "Plat/Private" EXCLUDE
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

# 可选：包含 Test 子项目
option(BUILD_TEST "Build Test executable" ON)
if(BUILD_TEST)
    add_subdirectory(Test)
endif()
