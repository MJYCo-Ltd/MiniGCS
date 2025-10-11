#ifndef QDATALINK_H
#define QDATALINK_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>
#include <memory>

// 前向声明
class QDataLinkPrivate;
class QVehicle;

/**
 * @brief 连接方式枚举
 */
enum class ConnectionType {
    Serial,     ///< 串口连接
    TCP,        ///< TCP连接
    UDP         ///< UDP连接
};

/**
 * @brief QDataLink类 - 数据链路通信类
 * 
 * 该类负责管理MAVLink数据链路的连接、断开和通信
 */
class QDataLink : public QObject
{
    Q_OBJECT

public:
    explicit QDataLink(QObject *parent = nullptr);
    ~QDataLink();

    /**
     * @brief 初始化MAVSDK
     */
    void initializeMavsdk();

    /**
     * @brief 连接到数据链路
     * @param connectionType 连接类型
     * @param address 地址（串口名称、IP地址）
     * @param portOrBaudRate 端口号或波特率
     * @return 连接是否成功
     */
    bool connectToDataLink(ConnectionType connectionType, const QString &address, int portOrBaudRate);

    /**
     * @brief 断开所有连接
     */
    void disconnect();

    /**
     * @brief 获取所有已连接的系统ID
     * @return 系统ID列表
     */
    QVector<uint8_t> getConnectedSystemIds() const;

    /**
     * @brief 获取指定系统的原始句柄
     * @param systemId 系统ID
     * @return 系统句柄指针，如果未找到则返回nullptr
     */
    void* getSystemHandle(uint8_t systemId) const;

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

signals:
    /**
     * @brief 连接错误信号
     * @param error 错误描述
     */
    void connectionError(const QString &error);

    /**
     * @brief 新飞控对象创建信号
     * @param vehicle 飞控对象指针
     */
    void vehicleCreated(QVehicle* vehicle);

    /**
     * @brief 系统连接状态变化信号
     * @param systemId 系统ID
     * @param connected 是否连接
     */
    void systemConnectionStatusChanged(uint8_t systemId, bool connected);

private slots:
    /**
     * @brief 处理新系统发现
     * @param systemId 系统ID
     */
    void onNewSystemDiscovered(uint8_t systemId);

private:
    std::unique_ptr<QDataLinkPrivate> d_ptr;    ///< 私有实现指针
};

#endif // QDATALINK_H
