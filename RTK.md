# MiniGCS 编码与架构规则

本文是 MiniGCS 仓库的长期开发约束。新任务、新窗口和后续修改均应先遵守本文；用户在当前任务中的明确要求优先于本文。

## 0. 复用优先（禁止重复生成）

新增功能前先搜索现有实现。凡已有可复用资产，必须扩展或引用，禁止在 C++、QML、配置中再造一份平行方案。

### 0.1 总原则

- **单一事实来源**：同一概念只保留一处定义，其它层只消费。
- **先查后写**：改类型、文案、阈值、命令、校验、UI 片段前，先 `rg` / 全局搜索是否已有。
- **扩展优于复制**：已有类型缺字段就扩展；已有组件差点布局就加属性/插槽；已有配置缺键就加键并做旧键兼容。
- **禁止平行实现**：不要为“差不多一样”的需求再开一套命名、目录或文件。

### 0.2 C++ / 公开 API

- 跨模块通用值类型放 `Inc/Common` + `Src/Common`（如 `QGpsPosition`、`QNEDPosition`、`QAttitude`、`QVelocity`）。禁止在 `Plat`/`AirLine`/`Test` 再定义等价结构体。
- 业务域类型放对应目录：`Plat`、`AirLine`、`Link`；不要把域模型塞进 `Common`，也不要把通用量散落在业务类扁平属性里（成组遥测用 Common 类型聚合）。
- 枚举、命令、结果码只在一处定义（如 `QAutopilot::ActionCommand`、`QDroneControlManager::Command`、`QAutoVehicleType`）；禁止 QML/其它 cpp 再用裸字符串或重复枚举。
- 协议适配（MAVSDK/MAVLink）只在 `Src/**/Private` 与 `Src/Extern`；公开 API 禁止再暴露协议类型、结果码、命令名、连接 URL 方言。
- 旧路径若保留，只允许薄转发头（如 `Inc/AirLine/QGpsPosition.h` → `Common/QGpsPosition.h`），不得维护两份实现。

### 0.3 QML / UI

- 可复用界面做成独立 QML 组件，禁止在多处复制粘贴同一控件树或同一套样式常量。
- `Main.qml` 只做壳与协调；具体能力进既有页面/组件，不新堆平行页面逻辑。
- 业务状态、命令名、机型判断、阈值数字不得在 QML 重复维护；通过 `AppConfig` / `DroneControl` / 核心对象属性读取。
- 展示文案：界面用 `qsTr()`；类型/状态名用类型文本目录；禁止在 QML 写死 PX4/ArduPilot/GPS 定位等映射表。

### 0.4 配置文件

- 可调参数只进 INI（经 `QGCSConfig` / `QTestGCSConfig`），禁止在 QML 与 C++ 各写一套魔法数字。
- QML 输入范围与 C++ 校验必须读同一配置键。
- 类型与状态中文（及同类展示映射）只维护 `Config/type_text_zh_CN.json`（兼容旧名 `mavsdk_zh_CN.json`），禁止在代码里再抄一份。
- 新增配置键：更新默认初始化、README，并兼容旧键；不要同时长期维护两套互不同步的新键语义。
- 演示专用配置只放 `QTestGCSConfig`；通用飞控行为留在 `QGCSConfig`。

### 0.5 判据（做之前自问）

1. 仓库里是否已有同名或同职责类型/组件/配置键？
2. 若有，能否加字段、参数或兼容键解决，而不是新建？
3. 若新建，是否放对了目录（Common vs 业务域 vs Test）？
4. C++、QML、配置三处是否会出现第二份真相？若会，先合并再改。

## 1. 默认工作方式

- 未经用户明确要求，不自动执行 CMake 配置、编译、安装或运行程序。
- 未经用户明确要求，不新增测试代码、测试工程或测试依赖。
- 修改后至少执行与改动匹配的静态检查，例如 `git diff --check`、JSON 解析、QML 静态检查和引用搜索。
- 不因“顺便整理”修改任务范围之外的代码，不覆盖用户已有的未提交改动。
- 只有用户明确要求时才提交、推送或创建分支；提交前确认改动范围和当前分支。

## 2. 总体分层

### 2.1 核心库

- `Inc/` 是公开 API，只放稳定的 MiniGCS 自有类型和接口。
- 跨模块复用的通用值类型（坐标、姿态、速度、原始 GPS 质量等）放在 `Inc/Common` 与 `Src/Common`；业务域类型仍放在 `Plat`、`AirLine`、`Link` 等目录。
- `Src/` 是核心实现；第三方 SDK 的适配、线程、订阅句柄和实现细节放在 `Src/**/Private` 或其他私有实现中。
- 公共头文件不得暴露 MAVSDK 类型、头文件、智能指针或回调签名。对外统一使用 Qt、标准库或 MiniGCS 自有类型。
- `QGroundControlStation` 负责地面站核心对象与平台生命周期，`QLinkManager` 负责链路，`QPlat/QAutopilot` 负责平台能力和状态。
- 使用 PIMPL 隔离 MAVSDK。新增 MAVSDK 插件能力时，公开接口放在 `QAutopilot` 等自有类，实现放在对应 Private 类。
- `XmlToMavSDK` 专门用于 APM/ArduPilot 扩展命令和扩展消息适配，不将它泛化为普通类型显示、UI 文案或通用配置工具。

### 2.2 Test 演示程序

- `Test/` 是演示和集成界面，不属于核心 SDK 公共 API。
- UI 专用状态、编组、命令编排、图标选择等放在 `Test` 层，不让核心库反向依赖 Test 的 QML、qrc 路径或资源。
- `QTestGCSConfig` 只扩展演示程序需要的配置；通用飞控行为参数放在 `QGCSConfig`。

依赖方向必须保持：

```text
QML/Test UI -> Test 控制层 -> MiniGCS 公共 API -> Private/MAVSDK
```

禁止出现核心库依赖 Test UI 或公开 API 依赖 MAVSDK 的反向关系。

## 3. QML 组织规则

- `Main.qml` 只负责顶层窗口、页面布局、菜单以及跨组件信号协调，不承载具体业务界面实现。
- 按功能拆分 QML，当前职责如下：
  - `DroneMap.qml`：地图、无人机标记、航线和航点显示。
  - `RouteEditor.qml`：航点编辑、航线数据和上传请求。
  - `AppComboBox.qml`：统一下拉高亮对比度的 ComboBox。
  - `SingleDroneControl.qml`：单机选择、状态和控制。
  - `GroupDroneControl.qml`：编组维护和编组控制。
  - `DroneControlPanel.qml`：单机/编组控制区域的组合。
  - `CommandConfirmDialog.qml`：危险控制命令确认。
  - `MissionStatusPanel.qml`：任务进度与暂停/返航快捷操作。
  - `FlightRecordPage.qml`：飞行记录查看。
  - `LinkConfigPage.qml` / `MapConfigPage.qml`：链路与地图设置。
  - `DroneStatusPage.qml`：单机详细状态。
- 新功能优先创建或扩展对应组件；不要继续把实现堆入 `Main.qml`。
- 组件通过属性和信号通信，不跨组件直接访问内部控件 ID。
- 新增 QML 文件后必须加入 `Test/CMakeLists.txt` 的 `QML_FILES`。
- 航点标记的层级必须高于航线；选中无人机使用红色状态，名称不得遮挡机型图标，并保留位置箭头。
- 避免使用会造成 `implicitWidth`、`implicitHeight` 绑定循环的尺寸绑定。文本输入框必须满足 NativeStyle 的最小隐式高度。

## 4. 类型、命令和显示文本

- 类型与状态显示文案不得写死在 C++ 或 QML 中，统一读取 `Config/type_text_zh_CN.json`（兼容旧文件 `mavsdk_zh_CN.json`）。
- 当前文本目录包含载具、飞控、GPS 定位、任务结果、固件版本、命令名称和机型图标映射；新增映射应继续放入目录文件。
- 核心层通过类型文本目录、`QAutoVehicleType` 或合适的自有接口读取类型文本，QML 不直接按底层协议枚举数值判断机型。
- 控制命令必须使用 `QDroneControlManager::Command` 统一定义。QML 不传递 `"arm"`、`"takeoff"` 等裸字符串，也不自行维护命令名称映射。
- 机型图标属于 UI 资源，由 Test 控制层查询配置并提供给 QML；不得成为核心 SDK 对 Test qrc 资源的依赖。
- 普通 QML 界面文案使用 `qsTr()`；C++ 业务错误和用户可见状态使用 `tr()` 或 `QCoreApplication::translate()`。
- Qt 翻译文案与类型文本目录用途不同：普通业务文案不要塞入类型文本文件。
- 公开 `Inc/` API 不得出现 MAVLink/MAVSDK 类型、结果码、命令名、连接 URL 方言或协议术语；协议适配仅允许存在于 `Src/**/Private` 与 `Src/Extern`。

## 5. 配置规则

- 可调行为参数不得散落为 QML/C++ 魔法数字，应通过 `QSettings` 配置并提供明确默认值。
- 通用配置由 `QGCSConfig` 管理；演示 UI 配置由 `QTestGCSConfig` 管理。
- 当前主要配置域：
  - `GCS/*`：地面站系统和组件 ID。
  - `Logging/*`：日志级别。
  - `MessageExtension/*`、`TypeText/*`、`Command/*`：消息扩展、类型文本目录与命令超时（兼容旧键 `MavMessage/*`、`Mavsdk/*`）。
  - `TimeSync/*`：时间同步。
  - `Motion/*`：无人机移动/静止判断阈值和连续采样数。
  - `Map/*`：地图插件、初始中心和缩放范围。
  - `Mission/*`：默认、最小和最大航点高度。
  - `Links/*`、`DroneGroups/*`、`Drones/*`：演示程序链路、编组和别名。
- QML 输入约束与 C++ 后端验证必须读取同一份配置，避免前后端范围不一致。
- 配置新增键时，同时更新默认初始化逻辑和 README 配置说明，并考虑旧配置迁移。
- 文本目录应支持相对/绝对路径和运行时重新读取，不在代码中复制一份备用中文映射。

## 6. 飞控状态与地图位置

- 地图只显示已获得且有效的 GPS 坐标；首次有效定位后可以自动居中。
- 静止状态下不要用每次 GPS 抖动更新地图标记。是否移动综合使用 `inAir`、NED 水平/垂直速度、启停阈值和连续采样数判断。
- 移动阈值从 `Motion/*` 读取，不为固定翼、多旋翼等直接写死另一套数字；如需机型差异，应扩展结构化配置。
- 飞行状态、位置和异步结果由核心对象发信号，QML 不推断 MAVSDK 第三方状态。

## 7. 航线与控制命令

- 航线上传和下载通过 `QAutopilot` 自有接口封装 MAVSDK Mission 插件。
- 上传与下载互斥，使用请求 ID 防止过期异步回调修改新请求状态。
- 单机与编组控制共用同一命令定义和分发路径；编组只向在线且可执行的成员发送。
- 危险命令和航线上传在真正下发前保留确认对话框。
- 航点经纬度、相对高度必须在 C++ 边界再次校验，不能只依赖 QML Validator。
- 异步回调捕获 QObject 时使用 `QPointer` 或等效生命周期保护；跨线程回到 QObject 时使用队列调用。

## 8. 日志规则

- 文件日志继续由 spdlog 管理（经 `QGCSConfig` 接管 Qt 日志）。
- 业务 warning 及以上与固件 warning 及以上通过 `QGCSConfig` 信号区分转发；固件日志不得再次进入业务日志造成重复。
- 固件日志级别判断使用具名常量或枚举，不在判断和 `switch` 中散落数字。
- 日志回调不得阻塞 MAVSDK 或 UI 线程。
- Test 演示工程不再维护内存日志面板；告警以文件日志与控制台为准。

## 9. 生命周期与线程安全

- 关闭程序时先销毁 QML 引擎和界面，再销毁控制单例、地面站、链路和 MAVSDK 资源。
- 析构前取消遥测和消息订阅，避免回调访问已析构对象。
- 容器删除时不得使用失效迭代器；异步回调不得保留裸 QObject 指针。
- 不在 UI 线程执行阻塞网络、等待或同步销毁操作。

## 10. 完成修改后的检查

在不编译的默认前提下，至少完成：

1. `git diff --check`。
2. 修改 JSON 后进行 JSON 解析验证。
3. 修改 QML 后执行 `qmllint`；单独 lint 时无法加载构建生成的 `MiniGCS` 模块属于环境警告，但不能忽略真实语法错误。
4. 搜索旧接口、裸命令字符串、重复映射、已删除组件引用，以及是否违反第 0 节“复用优先”。
5. 检查新增 QML、资源、源文件是否已加入 CMake 清单；Common 类型是否放在 `Inc/Common` 与 `Src/Common`。
6. 向用户明确说明没有编译或运行测试。

只有用户明确要求编译时，才执行与风险匹配的构建和运行验证。
