# MiniGCS

MiniGCS 是一个基于 **Qt6 + MAVSDK** 的地面控制站（GCS）C++ 共享库，版本 `1.0.0`。它提供无人机/飞控的链路管理、状态采集、航线规划及自动驾驶控制等核心功能，可作为库集成到上层 GCS 应用程序中。

## 依赖项

| 依赖 | 版本要求 | 说明 |
|------|----------|------|
| [Qt6](https://www.qt.io/) | ≥ 6.9 | Core 模块（对象系统、信号槽） |
| [MAVSDK](https://mavsdk.mavlink.io/) | — | MAVLink 协议通信 |
| [spdlog](https://github.com/gabime/spdlog) | — | 高性能日志库 |
| CMake | ≥ 3.16 | 构建系统 |
| C++ | 20 | 语言标准 |

> Windows 平台预编译依赖已放置在 `Depends/` 目录中（Debug / Release 各一份）。

---

## 构建

### 前置条件

- 安装 Qt6（并设置环境变量 `QTDIR` 指向 Qt 工具链目录，例如 `C:/Qt/6.9.0/msvc2022_64`）
- MSVC 2022 或更高版本（Windows）

### 使用 CMake Presets

```bash
# Debug 构建
cmake --preset debug
cmake --build --preset debug

# Release 构建
cmake --preset release
cmake --build --preset release
```

构建产物（DLL / `.lib`）默认输出到 `out/build/<preset>/` 下。

### 安装

```bash
cmake --install out/build/release --prefix <安装目录>
```

安装后目录结构：

```
<prefix>/
  bin/          # MiniGCS.dll
  lib/          # 导入库 + cmake/MiniGCS/
  include/      # 公开头文件（不含 Private 子目录）
```

---

## 集成到其他 CMake 项目

安装完成后，在下游项目的 `CMakeLists.txt` 中：

```cmake
find_package(MiniGCS REQUIRED)

target_link_libraries(MyApp PRIVATE MiniGCS::MiniGCS)
```

---

## 模块说明

### 核心入口

| 类 | 头文件 | 说明 |
|----|--------|------|
| `QGroundControlStation` | `QGroundControlStation.h` | GCS 核心类，管理链路与飞控对象生命周期 |
| `QGCSConfig` | `QGCSConfig.h` | 配置单例，管理系统 ID、日志等全局配置 |

### 链路管理（Link）

| 类/结构 | 说明 |
|---------|------|
| `QLinkManager` | 统一管理所有通信链路，支持 TCP Server/Client、UDP Server/Client、串口、Raw 六种类型 |
| `QDataLink` | 单条链路抽象，负责数据收发 |
| `LinkKind` | 链路类型枚举 |
| `LinkParams` | 链路参数结构体（端口、主机名、串口名、波特率） |

```cpp
QLinkManager *lm = gcs->linkManager();

// 添加 UDP 服务器链路，监听 14550 端口
LinkParams params;
params.port = 14550;
lm->addLink(LinkKind::UdpServer, params);
```

### 平台 / 飞控（Plat）

| 类 | 说明 |
|----|------|
| `QPlat` | 平台基类，封装固件版本、连接状态等基础信息 |
| `QAutopilot` | 自动驾驶仪类，继承 `QPlat`，提供 GPS/NED 位置、姿态、航向、飞行模式等实时状态，以及解锁、起飞、模式切换等控制接口 |
| `QAutopilotStatus` | 飞控状态数据（电池、飞行模式、解锁状态等） |
| `QAutopilotFixedwing` | 固定翼专用状态扩展 |
| `QAutoVehicleType` | 飞行器类型与自驾仪类型枚举 |

```cpp
// 监听新飞控发现
connect(gcs, &QGroundControlStation::newPlatFind, [](QPlat *plat) {
    auto *ap = qobject_cast<QAutopilot *>(plat);
    if (ap) {
        qDebug() << "GPS:" << ap->gpsPosition().latitude();
    }
});
```

### 航线管理（AirLine）

| 类 | 说明 |
|----|------|
| `QAirLineManager` | 管理多条航线（增删查，暴露给 QML） |
| `QAirLine` | 单条航线，包含航点列表 |
| `QGpsPosition` | GPS 坐标（经纬度 + 高度） |
| `QNEDPosition` | NED 坐标（北东地） |

### 外部接口（Extern）

| 类 | 说明 |
|----|------|
| `XmlToMavSDK` | 将 XML 格式任务文件转换为 MAVSDK 任务项 |
| `AsyncSendMavLink` | 异步 MAVLink 消息发送队列 |

---

## 日志

MiniGCS 使用 **spdlog** 输出日志，同时接管 Qt 的 `qDebug/qWarning` 等消息。日志文件默认路径：

```
out/build/<preset>/data/log/minigcs_<日期>.log
```

可通过 `QGCSConfig` 进行日志级别与路径配置。

---

## 目录结构

```
MiniGCS/
├── CMakeLists.txt
├── CMakePresets.json
├── MiniGCSConfig.cmake.in     # CMake 安装配置模板
├── Depends/                   # 预编译第三方依赖
│   ├── mavsdk/
│   └── spdlog/
├── Inc/                       # 公开头文件
│   ├── QGroundControlStation.h
│   ├── QGCSConfig.h
│   ├── QGCSLog.h
│   ├── MiniGCSExport.h        # 导出宏定义
│   ├── AirLine/
│   ├── Link/
│   ├── Plat/
│   └── Extern/
├── Src/                       # 实现源文件
└── Test/                      # 测试程序
    ├── main.cpp
    └── qml/Main.qml
```

---

## 许可证

见 [LICENSE](LICENSE) 文件。
