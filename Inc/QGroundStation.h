#ifndef QGROUNDSTATION_H
#define QGROUNDSTATION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QTimer>
#include <memory>

// 前向声明
class QGroundStationPrivate;

/**
 * @brief 连接方式枚举
 */
enum class ConnectionType {
    Serial,     ///< 串口连接
    TCP,        ///< TCP连接
    UDP         ///< UDP连接
};

/**
 * @brief 飞控信息结构体
 */
struct VehicleInfo {
    uint8_t systemId;                    ///< 系统ID
    uint8_t componentId;                 ///< 组件ID
    QString autopilotType;               ///< 自动驾驶仪类型
    QString vehicleType;                 ///< 载具类型
    QString firmwareVersion;             ///< 固件版本
    QString hardwareVersion;             ///< 硬件版本
    QString softwareVersion;             ///< 软件版本
    bool isConnected;                    ///< 是否已连接
    bool hasCamera;                      ///< 是否有相机
    std::vector<uint8_t> componentIds;       ///< 组件ID列表
    qint64 lastHeartbeatTime;           ///< 最后心跳时间戳（毫秒）
    bool isHeartbeatActive;              ///< 心跳是否活跃
};

/**
 * @brief 连接配置结构体
 */
struct ConnectionConfig {
    int heartbeatTimeoutMs;              ///< 心跳超时时间（毫秒），默认5000ms
    bool autoReconnect;                 ///< 是否自动重连，默认true
    int reconnectIntervalMs;            ///< 重连间隔时间（毫秒），默认3000ms
    int maxReconnectAttempts;            ///< 最大重连尝试次数，默认-1（无限）
    
    ConnectionConfig() 
        : heartbeatTimeoutMs(5000)
        , autoReconnect(true)
        , reconnectIntervalMs(3000)
        , maxReconnectAttempts(-1)
    {}
};

/**
 * @brief QGroundStation类 - 地面站控制类
 * 
 * 该类继承自QObject，支持串口、TCP、UDP连接飞控，
 * 并提供飞控信息查询功能
 */
class QGroundStation : public QObject
{
    Q_OBJECT

public:
    explicit QGroundStation(QObject *parent = nullptr);
    ~QGroundStation();

    /**
     * @brief 连接到飞控
     * @param connectionType 连接类型
     * @param connectionString 连接字符串
     * @return 连接是否成功
     */
    bool connectToVehicle(ConnectionType connectionType, const QString &connectionString);

    /**
     * @brief 断开连接
     */
    void disconnect();

    /**
     * @brief 获取所有已连接的飞控系统ID
     * @return 飞控系统ID列表
     */
    QVector<uint8_t> getConnectedSystemIds() const;

    /**
     * @brief 获取指定系统的飞控信息
     * @param systemId 系统ID
     * @return 飞控信息
     */
    VehicleInfo getVehicleInfo(uint8_t systemId) const;

    /**
     * @brief 获取所有已连接飞控的信息
     * @return 飞控信息列表
     */
    QVector<VehicleInfo> getAllVehicleInfo() const;

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief 获取连接的系统数量
     * @return 系统数量
     */
    int getSystemCount() const;

    /**
     * @brief 获取指定系统的原始句柄（用于与XmlToMavSDK等类集成）
     * @param systemId 系统ID
     * @return 系统句柄指针，如果未找到则返回nullptr
     */
    void* getSystemHandle(uint8_t systemId) const;

    /**
     * @brief 设置连接配置
     * @param config 连接配置
     */
    void setConnectionConfig(const ConnectionConfig &config);

    /**
     * @brief 获取当前连接配置
     * @return 连接配置
     */
    ConnectionConfig getConnectionConfig() const;

    /**
     * @brief 设置心跳超时时间
     * @param timeoutMs 超时时间（毫秒）
     */
    void setHeartbeatTimeout(int timeoutMs);

    /**
     * @brief 设置是否自动重连
     * @param autoReconnect 是否自动重连
     */
    void setAutoReconnect(bool autoReconnect);

    /**
     * @brief 手动重连指定系统
     * @param systemId 系统ID
     * @return 重连是否成功
     */
    bool reconnectSystem(uint8_t systemId);

signals:
    /**
     * @brief 新飞控接入信号
     * @param systemId 系统ID
     * @param vehicleInfo 飞控信息
     */
    void vehicleConnected(uint8_t systemId, const VehicleInfo &vehicleInfo);

    /**
     * @brief 飞控掉线信号（心跳超时）
     * @param systemId 系统ID
     * @param reason 掉线原因
     */
    void vehicleDisconnected(uint8_t systemId, const QString &reason);

    /**
     * @brief 通信链路断开信号
     * @param systemId 系统ID
     * @param error 错误描述
     */
    void communicationLost(uint8_t systemId, const QString &error);

    /**
     * @brief 连接错误信号
     * @param error 错误描述
     */
    void connectionError(const QString &error);

    /**
     * @brief 飞控信息更新信号
     * @param vehicleInfo 飞控信息
     */
    void vehicleInfoUpdated(const VehicleInfo &vehicleInfo);

    /**
     * @brief 心跳状态变化信号
     * @param systemId 系统ID
     * @param isActive 心跳是否活跃
     */
    void heartbeatStatusChanged(uint8_t systemId, bool isActive);

    /**
     * @brief 重连尝试信号
     * @param systemId 系统ID
     * @param attemptCount 尝试次数
     */
    void reconnectAttempted(uint8_t systemId, int attemptCount);

    /**
     * @brief 重连成功信号
     * @param systemId 系统ID
     */
    void reconnected(uint8_t systemId);

    /**
     * @brief 重连失败信号
     * @param systemId 系统ID
     * @param error 错误描述
     */
    void reconnectFailed(uint8_t systemId, const QString &error);

private slots:
    /**
     * @brief 定期检查系统状态
     */
    void checkSystemStatus();

private:
    /**
     * @brief 获取系统信息
     * @param system 系统指针
     * @return 飞控信息
     */
    VehicleInfo extractVehicleInfo(void* system) const;

    std::unique_ptr<QGroundStationPrivate> d_ptr;    ///< 私有实现指针
};

#endif // QGROUNDSTATION_H
