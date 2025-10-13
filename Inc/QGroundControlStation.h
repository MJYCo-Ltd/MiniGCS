#ifndef QGROUNDCONTROLSTATION_H
#define QGROUNDCONTROLSTATION_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>

// 前向声明
class QDataLink;

// 连接方式枚举
enum class ConnectionType {
    Serial,     ///< 串口连接
    TCP,        ///< TCP连接
    UDP         ///< UDP连接
};


/**
 * @brief QGroundControlStation类 - 地面控制站类
 * 
 * 该类继承自QObject，支持串口、TCP、UDP连接飞控，
 * 并提供飞控信息查询功能
 */
class QGroundControlStation : public QObject
{
    Q_OBJECT

public:
    explicit QGroundControlStation(QObject *parent = nullptr);
    ~QGroundControlStation();

    /**
     * @brief 创建数据链路
     * @param connectionType 连接类型
     * @param address 地址（串口名称、IP地址）
     * @param portOrBaudRate 端口号或波特率
     * @return 数据链路指针
     */
    QDataLink* createDataLink(ConnectionType connectionType, const QString &address, int portOrBaudRate);

    /**
     * @brief 移除数据链路
     * @param dataLink 数据链路指针
     */
    void removeDataLink(QDataLink* dataLink);

    /**
     * @brief 获取所有数据链路
     * @return 数据链路列表
     */
     QVector<QDataLink*> getAllDataLinks() const;

    /**
     * @brief 断开所有数据链路
     */
    void disconnect();

signals:
    /**
     * @brief 数据链路数量变化信号
     * @param count 当前数据链路数量
     */
    void dataLinkCountChanged(int count);

private:

    /**
     * @brief 根据连接类型生成连接字符串
     * @param connectionType 连接类型
     * @param address 地址（串口名称、IP地址）
     * @param portOrBaudRate 端口号或波特率
     * @return 完整的连接字符串
     */
    QString generateConnectionString(ConnectionType connectionType, const QString &address, int portOrBaudRate) const;

    /**
     * @brief 通过连接字符串查找已存在的通信链路
     * @param connectionString 连接字符串
     * @return 数据链路指针，如果未找到则返回nullptr
     */
    QDataLink* findExistingDataLink(const QString &connectionString) const;


    /**
     * @brief 创建数据链路的内部实现
     * @param connectionString 连接字符串
     * @return 数据链路指针，如果连接失败则返回nullptr
     */
    QDataLink* createDataLinkInternal(const QString &connectionString);

    /**
     * @brief 发射数据链路数量变化信号
     */
    void emitDataLinkCountChanged();
private:
    QMap<QString, QDataLink*> m_connectionStringMap;       ///< 连接字符串到数据链路的映射
};

#endif // QGROUNDCONTROLSTATION_H
