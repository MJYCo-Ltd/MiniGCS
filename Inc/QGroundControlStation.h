#ifndef QGROUNDCONTROLSTATION_H
#define QGROUNDCONTROLSTATION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>

// 前向声明
class QDataLink;


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
     * @return 数据链路指针
     */
    QDataLink* createDataLink();

    /**
     * @brief 移除数据链路
     * @param dataLink 数据链路指针
     */
    void removeDataLink(QDataLink* dataLink);

    /**
     * @brief 断开所有数据链路
     */
    void disconnect();


    /**
     * @brief 获取所有数据链路
     * @return 数据链路列表
     */
    QVector<QDataLink*> getAllDataLinks() const;


private:
    QVector<QDataLink*> m_dataLinks;                        ///< 数据链路列表
};

#endif // QGROUNDCONTROLSTATION_H
