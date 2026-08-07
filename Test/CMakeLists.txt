cmake_minimum_required(VERSION 3.16)

project(Test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Quick
    SerialPort
    Location
    Network
)

qt_standard_project_setup(REQUIRES 6.9)

# 在同一个构建树中，直接使用 MiniGCS 目标
# 如果是从已安装的包中使用，则使用 find_package
if(TARGET MiniGCS)
    # 在同一个构建树中，直接使用目标（不带命名空间）
    message(STATUS "Using MiniGCS from build tree")
    set(MINIGCS_TARGET MiniGCS)
elseif(TARGET MiniGCS::MiniGCS)
    # 如果已经通过 find_package 加载，使用命名空间版本
    message(STATUS "Using MiniGCS from find_package (with namespace)")
    set(MINIGCS_TARGET MiniGCS::MiniGCS)
else()
    # 尝试通过 find_package 查找（用于独立构建 Test 时）
    find_package(MiniGCS REQUIRED)
    message(STATUS "Using MiniGCS from installed location")
    set(MINIGCS_TARGET MiniGCS::MiniGCS)
endif()

# 创建可执行文件
qt_add_executable(${PROJECT_NAME}
    main.cpp
    QTestGCSConfig.cpp
    QTestGCSConfig.h
    QDroneControlManager.cpp
    QDroneControlManager.h
    QFlightRecordStore.cpp
    QFlightRecordStore.h
)

# 添加 QML 模块
qt_add_qml_module(${PROJECT_NAME}
    URI MiniGCS
    QML_FILES
        qml/AppComboBox.qml
        qml/CommandConfirmDialog.qml
        qml/DroneControlPanel.qml
        qml/DroneMap.qml
        qml/DroneStatusPage.qml
        qml/FlightRecordPage.qml
        qml/GroupDroneControl.qml
        qml/LinkConfigPage.qml
        qml/Main.qml
        qml/MapConfigPage.qml
        qml/MissionStatusPanel.qml
        qml/RouteEditor.qml
        qml/SingleDroneControl.qml
    RESOURCES
        qml/images/boat.svg
        qml/images/fixed-wing.svg
        qml/images/helicopter.svg
        qml/images/multirotor.svg
        qml/images/rocket.svg
        qml/images/rover.svg
        qml/images/submarine.svg
        qml/images/unknown.svg
        qml/images/vtol.svg
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4828)
endif()

# 链接 MiniGCS 库
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${MINIGCS_TARGET}
    Qt6::Core
    Qt6::Quick
    Qt6::SerialPort
    Qt6::Location
    Qt6::Network
)

