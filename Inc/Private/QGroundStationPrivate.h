#ifndef QGROUNDSTATIONPRIVATE_H
#define QGROUNDSTATIONPRIVATE_H

#include <QTimer>
#include <QVector>
#include <QMap>
#include <QDateTime>
#include <memory>
#include <mavsdk/mavsdk.h>
#include <mavsdk/system.h>

// 前向声明
struct ConnectionConfig;
struct VehicleInfo;

/**
 * @brief QGroundStation的私有实现类
 * 
 * 该类封装了QGroundStation的所有MAVSDK相关实现细节，
 * 使用PIMPL模式隐藏实现细节
 */
class QGroundStationPrivate
{
public:
    QGroundStationPrivate();
    ~QGroundStationPrivate();

    /**
     * @brief 初始化MAVSDK
     */
    void initializeMavsdk();

    /**
     * @brief 设置连接错误处理
     * @param parent QGroundStation实例指针，用于信号发射
     */
    void setupConnectionErrorHandling(QObject* parent);

    /**
     * @brief 连接到飞控
     * @param connectionString 连接字符串
     * @return 连接是否成功
     */
    bool connectToVehicle(const QString &connectionString);

    /**
     * @brief 断开所有连接
     */
    void disconnect();

    /**
     * @brief 获取所有已连接的系统
     * @return 系统列表
     */
    QVector<std::shared_ptr<mavsdk::System>> getConnectedSystems() const;

    /**
     * @brief 获取指定系统的指针
     * @param systemId 系统ID
     * @return 系统指针，如果未找到则返回nullptr
     */
    mavsdk::System* getSystem(uint8_t systemId) const;

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief 获取系统数量
     * @return 系统数量
     */
    int getSystemCount() const;

    /**
     * @brief 设置状态检查定时器
     * @param timer 定时器指针
     */
    void setStatusTimer(QTimer* timer);

    /**
     * @brief 获取状态检查定时器
     * @return 定时器指针
     */
    QTimer* getStatusTimer() const;

    /**
     * @brief 获取已连接的系统列表（用于状态检查）
     * @return 已连接的系统列表
     */
    QVector<std::shared_ptr<mavsdk::System>>& getConnectedSystemsList();

    /**
     * @brief 检查系统状态变化
     * @param parent QGroundStation实例指针，用于信号发射
     */
    void checkSystemStatus(QObject* parent);

    /**
     * @brief 根据连接类型生成连接字符串
     * @param connectionType 连接类型
     * @param connectionString 连接字符串
     * @return 完整的连接字符串
     */
    QString generateConnectionString(int connectionType, const QString &connectionString) const;

    /**
     * @brief 设置连接配置
     * @param config 连接配置
     */
    void setConnectionConfig(const ConnectionConfig &config);

    /**
     * @brief 获取连接配置
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
     * @param parent QGroundStation实例指针，用于信号发射
     * @return 重连是否成功
     */
    bool reconnectSystem(uint8_t systemId, QObject* parent);

    /**
     * @brief 检查心跳状态
     * @param parent QGroundStation实例指针，用于信号发射
     */
    void checkHeartbeatStatus(QObject* parent);

    /**
     * @brief 更新系统心跳时间
     * @param systemId 系统ID
     */
    void updateHeartbeatTime(uint8_t systemId);

    /**
     * @brief 获取系统最后心跳时间
     * @param systemId 系统ID
     * @return 最后心跳时间戳（毫秒）
     */
    qint64 getLastHeartbeatTime(uint8_t systemId) const;

    /**
     * @brief 获取系统心跳状态
     * @param systemId 系统ID
     * @return 心跳是否活跃
     */
    bool isHeartbeatActive(uint8_t systemId) const;

    /**
     * @brief 设置系统连接信息
     * @param systemId 系统ID
     * @param connectionType 连接类型
     * @param connectionString 连接字符串
     */
    void setSystemConnectionInfo(uint8_t systemId, int connectionType, const QString &connectionString);

    /**
     * @brief 获取系统连接信息
     * @param systemId 系统ID
     * @return 连接字符串，如果未找到则返回空字符串
     */
    QString getSystemConnectionInfo(uint8_t systemId) const;

private:
    std::shared_ptr<mavsdk::Mavsdk> m_mavsdk;        ///< MAVSDK实例
    QTimer *m_statusTimer;                           ///< 状态检查定时器
    QVector<std::shared_ptr<mavsdk::System>> m_connectedSystems; ///< 已连接的系统列表
    bool m_isInitialized;                            ///< 是否已初始化
    
    // 心跳检测相关
    QMap<uint8_t, qint64> m_lastHeartbeatTimes;     ///< 系统最后心跳时间
    QMap<uint8_t, bool> m_heartbeatStatus;           ///< 系统心跳状态
    QMap<uint8_t, QString> m_systemConnectionInfo;    ///< 系统连接信息
    QMap<uint8_t, int> m_systemConnectionTypes;     ///< 系统连接类型
    QMap<uint8_t, int> m_reconnectAttempts;          ///< 系统重连尝试次数
    
    // 连接配置
    std::unique_ptr<ConnectionConfig> m_pConnectionConfig;  ///< 连接配置
};

#endif // QGROUNDSTATIONPRIVATE_H
