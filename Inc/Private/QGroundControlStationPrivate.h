#ifndef QGROUNDCONTROLSTATIONPRIVATE_H
#define QGROUNDCONTROLSTATIONPRIVATE_H

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
 * @brief QGroundControlStation的私有实现类
 * 
 * 该类封装了QGroundControlStation的所有MAVSDK相关实现细节，
 * 使用PIMPL模式隐藏实现细节
 */
class QGroundControlStationPrivate
{
public:
    QGroundControlStationPrivate();
    ~QGroundControlStationPrivate();

    /**
     * @brief 初始化MAVSDK
     */
    void initializeMavsdk();

    /**
     * @brief 设置连接错误处理
     * @param parent QGroundControlStation实例指针，用于信号发射
     */
    void setupConnectionErrorHandling(QObject* parent);

    /**
     * @brief 连接到数据链路
     * @param connectionType 连接类型
     * @param address 地址（串口名称、IP地址或端口号）
     * @param portOrBaudRate 端口号或波特率
     * @return 连接是否成功
     */
    bool connectToDataLink(int connectionType, const QString &address, int portOrBaudRate);

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
     * @brief 设置系统连接状态回调
     * @param parent QGroundControlStation实例指针，用于信号发射
     */
    void setupSystemConnectionCallbacks(QObject* parent);

    /**
     * @brief 根据连接类型生成连接字符串
     * @param connectionType 连接类型
     * @param address 地址（串口名称、IP地址或端口号）
     * @param portOrBaudRate 端口号或波特率
     * @return 完整的连接字符串
     */
    QString generateConnectionString(int connectionType, const QString &address, int portOrBaudRate) const;

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
     * @brief 设置是否自动重连
     * @param autoReconnect 是否自动重连
     */
    void setAutoReconnect(bool autoReconnect);

    /**
     * @brief 手动重连指定系统
     * @param systemId 系统ID
     * @param parent QGroundControlStation实例指针，用于信号发射
     * @return 重连是否成功
     */
    bool reconnectSystem(uint8_t systemId, QObject* parent);

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

    /**
     * @brief 获取地面站系统ID
     * @return 地面站系统ID
     */
    uint8_t getGroundStationSystemId() const;

    /**
     * @brief 获取地面站组件ID
     * @return 地面站组件ID
     */
    uint8_t getGroundStationComponentId() const;

private:
    std::shared_ptr<mavsdk::Mavsdk> m_mavsdk;        ///< MAVSDK实例
    bool m_isInitialized;                            ///< 是否已初始化
    
    // 系统连接信息
    QMap<uint8_t, QString> m_systemConnectionInfo;    ///< 系统连接信息
    QMap<uint8_t, int> m_systemConnectionTypes;     ///< 系统连接类型
    QMap<uint8_t, int> m_reconnectAttempts;          ///< 系统重连尝试次数
    QMap<uint8_t, mavsdk::System::IsConnectedHandle> m_connectionHandles; ///< 连接状态回调句柄
    
    // 连接配置
    std::unique_ptr<ConnectionConfig> m_pConnectionConfig;  ///< 连接配置
    
    // 地面站ID配置
    uint8_t m_groundStationSystemId;     ///< 地面站系统ID，默认246
    uint8_t m_groundStationComponentId;  ///< 地面站组件ID，默认191
};

#endif // QGROUNDCONTROLSTATIONPRIVATE_H
