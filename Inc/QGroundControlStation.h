#ifndef QGROUNDCONTROLSTATION_H
#define QGROUNDCONTROLSTATION_H

#include <QObject>
#include <QSet>

// 前向声明
class QGroundControlStationPrivate;
class QVehicle;
class QDataLink;

/**
 * @brief QGroundControlStation类 - 地面控制站类
 */
class QGroundControlStation : public QObject
{
    Q_OBJECT

public:
    explicit QGroundControlStation(QObject *parent = nullptr);
    ~QGroundControlStation();

    /**
     * @brief 初始化系统
     */
    void Init();

    /**
     * @brief 增加数据链路
     * @param pDataLink
     * @return
     */
    bool AddDataLink(const QDataLink* pDataLink);

    /**
     * @brief 移除数据链路
     * @param pDataLink
     * @return
     */
    void RemoveDatLink(const QDataLink* pDataLink);

    /**
     * @brief 清除所有数据链路
     */
    void ClearAllDataLink();

    /**
     * @brief 获取飞机数量
     */
    int getVehicleCount() const;

    /**
     * @brief 获取所有飞控对象
     * @return 飞控对象列表
     */
    QVector<QVehicle*> getAllVehicles() const;

signals:
    /**
     * @brief 新飞控对象创建信号
     * @param vehicle 飞控对象指针
     */
    void newVehicleFind(QVehicle* vehicle);
    void connectionError(const QString& error);
    void systemConnectionStatusChanged(uint8_t id,bool isConnected);

private:
    /**
     * @brief 处理从DataLink接收到的消息
     * @param data 接收到的原始数据
     */
    void processDataLinkMessage(const QByteArray &data);

    /**
     * @brief 发送数据到所有DataLink
     * @param data 要发送的数据
     */
    void sendDataToAllLinks(const QByteArray &data);

private:
    std::unique_ptr<QGroundControlStationPrivate> d_ptr;    ///< 私有实现指针
    QSet<const QDataLink*> m_setLink;
};

#endif // QGROUNDCONTROLSTATION_H
