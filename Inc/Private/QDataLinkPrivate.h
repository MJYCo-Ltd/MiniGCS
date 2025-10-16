#ifndef QDATALINKPRIVATE_H
#define QDATALINKPRIVATE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>
#include <memory>
#include <mavsdk/mavsdk.h>
#include <mavsdk/system.h>

// 前向声明
struct ConnectionConfig;
class QVehicle;

/**
 * @brief QDataLink的私有实现类
 * 
 * 该类封装了QDataLink的所有MAVSDK相关实现细节，
 * 使用PIMPL模式隐藏实现细节
 */
class QDataLinkPrivate
{
public:
    QDataLinkPrivate();
    ~QDataLinkPrivate();

    /**
     * @brief 初始化MAVSDK
     */
    void initializeMavsdk();

    /**
     * @brief 设置连接字符串
     * @param connectionString 连接字符串
     */
    void setConnectionString(const QString &connectionString);

    /**
     * @brief 连接到数据链路
     * @return 连接是否成功
     */
    bool connectToDataLink();

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
     * @brief 获取所有已连接的系统ID
     * @return 系统ID列表
     */
    QVector<uint8_t> getConnectedSystemIds() const;

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
     * @brief 获取地面站系统ID
     * @return 地面站系统ID
     */
    uint8_t getGroundStationSystemId() const;

    /**
     * @brief 获取地面站组件ID
     * @return 地面站组件ID
     */
    uint8_t getGroundStationComponentId() const;

    /**
     * @brief 获取所有飞控对象
     * @return 飞控对象列表
     */
    QVector<QVehicle*> getAllVehicles() const;

    /**
     * @brief 获取指定系统ID的飞控对象
     * @param systemId 系统ID
     * @return 飞控对象指针，如果未找到则返回nullptr
     */
    QVehicle* getVehicle(uint8_t systemId) const;

    /**
     * @brief 获取所有已连接的系统ID
     * @return 系统ID列表
     */
    QVector<uint8_t> getVehicleIDs() const;

    /**
     * @brief 设置连接错误处理
     * @param parent QDataLink实例指针，用于信号发射
     */
    void setupConnectionErrorHandling(QObject* parent);

    /**
     * @brief 设置新系统发现回调
     * @param parent QDataLink实例指针，用于信号发射
     */
    void setupNewSystemDiscoveryCallback(QObject* parent);

    /**
     * @brief 设置系统连接状态回调
     * @param parent QDataLink实例指针，用于信号发射
     */
    void setupSystemConnectionCallbacks(QObject* parent);

    /**
     * @brief 创建或更新飞控对象
     * @param system 系统指针
     * @param parent QDataLink实例指针，用于信号发射
     * @return 飞控对象指针
     */
    QVehicle* createOrUpdateVehicle(void* system, QObject* parent);


private:
    std::shared_ptr<mavsdk::Mavsdk> m_mavsdk;        ///< MAVSDK实例
    bool m_isInitialized;                            ///< 是否已初始化
    
    // 连接参数
    QString m_connectionString;                      ///< 连接字符串
    
    // 连接管理
    mavsdk::Mavsdk::ConnectionHandle m_connectionHandle; ///< 连接句柄
    QMap<uint8_t, mavsdk::System::IsConnectedHandle> m_systemConnectionHandles; ///< 系统连接状态回调句柄
    mavsdk::Mavsdk::NewSystemHandle m_newSystemHandle; ///< 新系统订阅句柄
    
    // 地面站ID配置
    uint8_t m_groundStationSystemId;     ///< 地面站系统ID
    uint8_t m_groundStationComponentId;  ///< 地面站组件ID
    
    // 飞控对象管理
    QMap<uint8_t, QVehicle*> m_vehicles; ///< 飞控对象映射表
};

#endif // QDATALINKPRIVATE_H
