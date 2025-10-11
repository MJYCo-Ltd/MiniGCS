cmake_minimum_required(VERSION 3.16)

if(WIN32)
    set(MAVSDK_DIR ${CMAKE_SOURCE_DIR}/Depends/lib/cmake/MAVSDK)
endif()

project(MiniGSC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MAVSDK REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS
    Core
)

add_executable(${PROJECT_NAME} 
    main.cpp
    Src/Extern/XmlToMavSDK.cpp
    Src/QGroundStation.cpp
    Src/Private/QGroundStationPrivate.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4828)
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    MAVSDK::mavsdk
    Qt6::Core
)
