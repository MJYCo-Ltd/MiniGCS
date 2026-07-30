#ifndef QGROUNDCONTROLSTATIONPRIVATE_H
#define QGROUNDCONTROLSTATIONPRIVATE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>
#include <map>
#include <memory>
#include <mavsdk/mavsdk.h>
#include <mavsdk/system.h>

// 前向声明
struct ConnectionConfig;
class QPlat;

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
     * @brief 设置连接错误处理
     * @param parent QGroundControlStation实例指针，用于信号发射
     */
    void setupConnectionErrorHandling(QObject* parent);

    /**
     * @brief 设置新系统发现回调
     * @param parent QGroundControlStation实例指针，用于信号发射
     */
    void setupNewSystemDiscoveryCallback(QObject* parent);

    /**
     * @brief 设置系统连接状态回调
     * @param parent QGroundControlStation实例指针，用于信号发射
     */
    void setupSystemConnectionCallbacks(QObject* parent);

    /**
     * @brief 处理接收到的原始数据
     * @param data 接收到的原始数据
     */
    void processReceivedRawData(const QByteArray &data);

    /**
     * @brief 设置发送原始字节的回调
     * @param callback 回调函数，用于将数据发送到所有DataLink
     * @param parent QGroundControlStation实例指针，用于确保线程安全
     */
    void setupRawBytesToBeSentCallback(std::function<void(const QByteArray&)> callback, QObject* parent);

    /**
     * @brief 取消订阅发送原始字节的回调
     */
    void unsubscribeRawBytesToBeSent();

    /**
     * @brief 获取最多的通道数
     * @return
     */
    int getMaxChannel();

    /**
     * @brief 添加连接（供 QLinkManager 使用）
     * @param connectionUrl 连接字符串
     * @return 是否添加成功
     */
    bool addConnection(const QString &connectionUrl);

    /**
     * @brief 添加 Raw 连接（需传入 QDataLink 以接收数据）
     * @param rawDataLink Raw 链路对象，用于接收 rawDataReceived 信号
     * @return 是否添加成功
     */
    bool addRawConnection(class QDataLink *rawDataLink);

    /**
     * @brief 移除连接（供 QLinkManager 使用）
     * @param connectionUrl 连接字符串
     */
    void removeConnection(const QString &connectionUrl);

    std::shared_ptr<mavsdk::Mavsdk> mavsdk() const { return m_mavsdk; }

private:
    std::shared_ptr<mavsdk::Mavsdk> m_mavsdk;        ///< MAVSDK实例
    bool m_isInitialized;                            ///< 是否已初始化

    // 连接管理（connectionUrl -> handle）
    std::map<std::string, mavsdk::Mavsdk::ConnectionHandle> m_connectionHandles;
    mavsdk::Mavsdk::NewSystemHandle m_newSystemHandle;
    mavsdk::Mavsdk::RawBytesHandle m_rawBytesHandle;
    class QDataLink *m_rawDataLink{nullptr};  ///< Raw 模式下的数据链路，用于接收回调
};

#endif // QGROUNDCONTROLSTATIONPRIVATE_H

