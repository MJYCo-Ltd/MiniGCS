#ifndef QDATALINK_H
#define QDATALINK_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>

// 前向声明
class QDataLinkPrivate;
class QVehicle;

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
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isConnected() const;

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
     * @brief 连接错误信号
     * @param error 错误描述
     */
    void connectionError(const QString &error);

    /**
     * @brief 新飞控对象创建信号
     * @param vehicle 飞控对象指针
     */
    void newVehicleFind(QVehicle* vehicle);

    /**
     * @brief 系统连接状态变化信号
     * @param systemId 系统ID
     * @param connected 是否连接
     */
    void systemConnectionStatusChanged(uint8_t systemId, bool connected);

private:
    std::unique_ptr<QDataLinkPrivate> d_ptr;    ///< 私有实现指针
};

#endif // QDATALINK_H
