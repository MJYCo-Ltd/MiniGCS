# MiniGCS

MiniGCS 是一个基于 **Qt6 + MAVSDK** 的地面控制站（GCS）C++ 共享库（版本 `1.0.0`）。它提供无人机/飞控的链路管理、状态采集、航线规划及自动驾驶控制等核心能力，可作为库集成到上层 GCS 应用程序中。

## 依赖项

| 依赖 | 版本要求 | 说明 |
|------|----------|------|
| [Qt6](https://www.qt.io/) | ≥ 6.9 | Core 模块（对象系统、信号槽、MOC） |
| [MAVSDK](https://mavsdk.mavlink.io/) | — | MAVLink 协议通信 |
| [spdlog](https://github.com/gabime/spdlog) | — | 高性能日志 |
| CMake | ≥ 3.16 | 构建系统 |
| C++ | 20 | 语言标准 |

> **Windows 预编译依赖**：仓库未包含 `Depends/`（见 `.gitignore`）。在 Windows 上构建前，需自行准备第三方库并放到项目根目录，目录布局需与 `CMakeLists.txt` 一致（见下文「准备 Depends」）。

---

## 准备 Depends（Windows）

在仓库根目录创建 `Depends/`，结构示例：

```
Depends/
├── mavsdk/
│   └── lib/cmake/MAVSDK/    # find_package(MAVSDK) 所需
└── spdlog/
    ├── x64-Debug/lib/cmake/spdlog/
    └── x64-Release/lib/cmake/spdlog/
```

非 Windows 平台需在系统中安装 MAVSDK、spdlog，并保证 CMake 能通过 `find_package` 找到它们（当前 `CMakeLists.txt` 仅在 `WIN32` 下自动设置上述路径）。

---

## 构建

### 前置条件

- 安装 **Qt 6.9+**（含 Core；运行 `Test` 还需 Quick、SerialPort、Location、Network）
- 设置环境变量 **`QTDIR`** 指向 Qt 工具链，例如 `C:/Qt/6.9.0/msvc2022_64`
- **MSVC 2022** 或兼容工具链（Windows 推荐）
- 已按上文准备好 **Depends**（Windows）或系统级 MAVSDK / spdlog

### 配置与编译

以 **Ninja + 单配置** 为例（`CMAKE_BUILD_TYPE` 会用于选择 Debug/Release 版 spdlog）：

```powershell
# Release
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Debug（产物为 MiniGCSd.dll / MiniGCSd.lib，见 CMAKE_DEBUG_POSTFIX）
cmake -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

使用 **Visual Studio 多配置生成器** 时，需在配置阶段指定 `-DCMAKE_BUILD_TYPE=Release` 或 `Debug`（与 spdlog 路径选择一致），构建时指定 `--config`：

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

| CMake 选项 | 默认值 | 说明 |
|------------|--------|------|
| `MINIGCS_BUILD_DEMO` | `OFF` | 是否构建 `Test/` 下的 QML 演示程序 |

构建产物默认位于 `build/`（或你指定的 `-B` 目录），例如
`build/MiniGCS.dll`；启用演示选项后才会生成 `Test`。

### 安装

```powershell
cmake --install build --prefix <安装目录>
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

在同一构建树中开发时，也可直接 `add_subdirectory(MiniGCS)` 并链接目标 `MiniGCS`（无需先安装）。

---

## 快速开始

```cpp
#include <QCoreApplication>
#include <QDebug>
#include "QGroundControlStation.h"
#include "QGCSConfig.h"
#include "Link/QLinkManager.h"
#include "Plat/QAutopilot.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QGCSConfig::instance()->init();

    QGroundControlStation gcs;
    gcs.Init();

    QObject::connect(&gcs, &QGroundControlStation::newPlatFind, [](QPlat *plat) {
        if (auto *ap = qobject_cast<QAutopilot *>(plat)) {
            qDebug() << "GPS lat:" << ap->gpsPosition().latitude();
        }
    });

    LinkParams params;
    params.port = 14550;
    gcs.linkManager()->addLink(LinkKind::UdpServer, params);

    return app.exec();
}
```

典型初始化顺序：`QGCSConfig::instance()->init()` → 创建 `QGroundControlStation` → `Init()` → 通过 `linkManager()` 添加链路。

---

## 模块说明

### 核心入口

| 类 | 头文件 | 说明 |
|----|--------|------|
| `QGroundControlStation` | `QGroundControlStation.h` | GCS 核心：链路管理器、飞控对象生命周期、`newPlatFind` 等信号 |
| `QGCSConfig` | `QGCSConfig.h` | 配置单例（INI）：系统/组件 ID、日志级别、MAV 扩展消息等 |

### 链路管理（Link）

| 类/结构 | 说明 |
|---------|------|
| `QLinkManager` | 统一管理通信链路 |
| `QDataLink` | 单条链路抽象，负责数据收发 |
| `LinkKind` | `TcpServer` / `TcpClient` / `UdpServer` / `UdpClient` / `Serial` / `Raw` |
| `LinkParams` | 链路参数（端口、主机名、串口名、波特率） |

```cpp
QLinkManager *lm = gcs.linkManager();

LinkParams params;
params.port = 14550;
QDataLink *link = lm->addLink(LinkKind::UdpServer, params);
if (link) {
    link->setAutoReconnect(true);
    link->setReconnectCount(5); // 0 表示无限重试
}
```

自动重连采用退避策略（1、2、4、8、15 秒，之后保持 15 秒）。可通过
`opened` 表示底层传输是否已注册成功，`reconnectAttempts` 表示当前重试次数。
飞控是否在线应观察 `QPlat::connected`，不要用链路的 `opened` 代替设备在线状态。

### 平台 / 飞控（Plat）

| 类 | 说明 |
|----|------|
| `QPlat` | 平台基类：固件版本、连接状态等 |
| `QAutopilot` | 自驾仪：位置、姿态、速度、飞行模式、解锁/起飞等 |
| `QAutopilotStatus` | 电池、健康与遥控等状态 |
| `QAutopilotFixedwing` | 固定翼扩展状态 |
| `QAutoVehicleType` | 载具与自驾仪类型枚举 |

### 通用类型（Common）

| 类 | 说明 |
|----|------|
| `QGpsPosition` | 地理坐标（经纬度、高度） |
| `QNEDPosition` | NED 局部位移 |
| `QAttitude` | 姿态欧拉角与航向 |
| `QVelocity` | NED 速度分量及水平/垂直速度 |

### 航线管理（AirLine）

| 类 | 说明 |
|----|------|
| `QAirLineManager` | 多条航线管理（支持 QML） |
| `QAirLine` | 单条航线及航点列表 |

`QAirLineManager::addAirLine()` 成功后接管航线对象所有权；移除或清空航线时，
对象会在 `airlineRemoved` 信号发出后通过 `deleteLater()` 销毁。

飞控任务不会在发现设备时自动下载。需要时显式调用：

```cpp
QObject::connect(autopilot, &QAutopilot::airLineDownloaded,
                 [](const QList<QGpsPosition> &waypoints) {
                     // waypoint.altitude() 为相对起飞点高度
                 });
autopilot->downloadAirLine();
```

任务下载只使用 MAVSDK `Mission` 路径，避免与 `MissionRaw` 并行请求。
可通过 `airLineDownloading` 属性观察下载状态；重复请求会被拒绝。下载结果会
自动忽略没有有效经纬度或相对高度的非航点任务项。

任务航线可通过同一 `Mission` 插件异步上传：

```cpp
QList<QGpsPosition> waypoints{
    QGpsPosition(114.502461, 38.045474, 30.0),
    QGpsPosition(114.503461, 38.045474, 30.0)
};
QObject::connect(autopilot, &QAutopilot::airLineUploaded,
                 []() { /* 上传成功 */ });
autopilot->uploadAirLine(waypoints);
```

可通过 `airLineUploading` 属性及 `airLineUploadFailed` 信号观察上传状态。
上传与下载互斥，防止对同一个 Mission 插件并发发起任务请求。

## 日志与配置

- 日志由 **spdlog** 输出，并通过 `QGCSConfig::qtLogHandler` 接管 Qt 的 `qDebug` / `qWarning` 等。
- 默认日志文件（相对**当前工作目录**）：`data/log/minigcs.log`（按日滚动，保留 7 天）。
- `QGCSConfig::warningLogMessage` 转发业务 warning 及以上日志；
  `firmwareWarningMessage` 单独转发飞控固件 warning 及以上日志。
- 配置文件路径：`<可执行文件目录>/Config/<applicationName>.ini`（`applicationName` 为空时使用 `MiniGCS.ini`）。

常用 INI 键（节名以代码为准；旧键仍兼容）：

| 键 | 默认值 | 说明 |
|----|--------|------|
| `GCS/SystemId` | `246` | 地面站身份 ID |
| `GCS/ComponentId` | `191` | 地面站组件 ID |
| `Logging/Level` | `debug` | `trace` / `debug` / `info` / `warn` / `error` / `critical` / `off` |
| `MessageExtension/File` | `ardupilotmega.xml` | 扩展命令表文件（相对配置目录）。兼容旧键 `MavMessage/Extension` |
| `TypeText/File` | `type_text_zh_CN.json` | 类型/状态显示文本目录。兼容旧键 `Mavsdk/TypeTextFile` 与旧文件名 `mavsdk_zh_CN.json` |
| `Command/AckTimeoutMs` | `5000` | 扩展命令确认超时（1000–60000 ms）。兼容旧键 `Mavsdk/CommandAckTimeoutMs` |
| `TimeSync/Enabled` | `true` | 是否启用时间同步 |
| `Motion/StartHorizontalSpeedMS` | `0.7` | 判定开始移动的水平速度阈值（m/s） |
| `Motion/StartVerticalSpeedMS` | `0.5` | 判定开始移动的垂直速度阈值（m/s） |
| `Motion/StopHorizontalSpeedMS` | `0.25` | 判定停止移动的水平速度阈值（m/s） |
| `Motion/StopVerticalSpeedMS` | `0.2` | 判定停止移动的垂直速度阈值（m/s） |
| `Motion/StartSampleCount` | `2` | 开始移动所需连续采样数 |
| `Motion/StopSampleCount` | `5` | 停止移动所需连续采样数 |
| `Telemetry/PositionHz` 等 | 见默认 | 遥测订阅频率（Hz），含 Position / GpsInfo / Battery / Attitude / Health / Home 等 |

载具类型、飞控类型、机型图标、控制命令名称、GPS 定位状态、固件版本类型和任务结果均从
`Config/type_text_zh_CN.json`（或兼容的旧文件）读取，不在 C++ 中硬编码。可以复制该文件制作其他
语言版本，再通过 `TypeText/File` 指向新文件；程序会在文件更新时间或
大小变化后重新加载。

公开 API 仅暴露业务接口（平台、航线、链路 Kind/Params、业务命令与结果）。协议适配细节保留在 `Src/**/Private`。

可通过继承 `QGCSConfig` 并在首次 `instance()` 前调用 `QGCSConfig::setInstance()` 注入自定义配置（`Test` 工程中的 `QTestGCSConfig` 即如此，并额外支持多链路配置）。

---

## Test 演示程序

演示程序默认不构建。配置 `-DMINIGCS_BUILD_DEMO=ON` 后才会加入构建；
`Test` 为 **Qt Quick** 示例，依赖 Qt 模块：Core、Quick、SerialPort、Location、Network。

```powershell
cmake -B build -DMINIGCS_BUILD_DEMO=ON
cmake --build build --target Test
# 运行前将 MiniGCS.dll、Qt 运行时与 MAVSDK 依赖置于 PATH 或 exe 同目录
./build/Test.exe
```

程序从 `QTestGCSConfig` 读取链路列表并自动 `addLink`，QML 界面见 `Test/qml/Main.qml`。
界面支持按平台 ID 配置无人机别名、创建编组并维护成员，也可以向
单机或编组中的在线成员发送解锁、上锁、起飞、降落、返航和任务下载命令。
无人机别名与编组配置保存在演示程序的 INI 配置文件中。
地图插件、初始中心、缩放范围以及航点默认/最小/最大高度也可通过演示程序
INI 文件中的 `Map/*` 与 `Mission/*` 配置项调整。

---

## 目录结构

```
MiniGCS/
├── CMakeLists.txt
├── MiniGCSConfig.cmake.in     # 安装后的 CMake 包配置模板
├── Depends/                   # Windows 第三方库（本地准备，不入库）
├── Inc/                       # 公开头文件
│   ├── QGroundControlStation.h
│   ├── QGCSConfig.h
│   ├── MiniGCSExport.h
│   ├── AirLine/
│   ├── Common/
│   ├── Link/
│   ├── Plat/
├── Src/                       # 实现及 MAVSDK/spdlog 内部适配
└── Test/                      # QML 演示与 QTestGCSConfig
    ├── CMakeLists.txt
    ├── main.cpp
    └── qml/Main.qml
```

---

## 许可证

[MIT License](LICENSE) — Copyright (c) 2025 杨天宇
