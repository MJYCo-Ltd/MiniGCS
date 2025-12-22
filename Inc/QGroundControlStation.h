#ifndef QGROUNDCONTROLSTATION_H
#define QGROUNDCONTROLSTATION_H

#include <QObject>
#include <QMap>
#include "MiniGCSExport.h"

// 前向声明
class QPlat;
class QDataLink;
class QThread;

/**
 * @brief QGroundControlStation类 - 地面控制站类
 */
class MINIGCS_EXPORT QGroundControlStation : public QObject
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
    bool AddDataLink(QDataLink *pDataLink);

    /**
     * @brief 移除数据链路
     * @param pDataLink
     * @return
     */
    void RemoveDatLink(QDataLink *pDataLink);

    /**
     * @brief 清除所有数据链路
     */
    void ClearAllDataLink();

    /**
     * @brief 获取所有飞控对象
     * @return 飞控对象列表
     */
    Q_INVOKABLE QList<QObject*> plats() const{return(m_listPlat);}

    Q_INVOKABLE QList<QObject*> dataLinks() const{return(m_listLink);}

signals:
    /**
     * @brief 新飞控对象创建信号
     * @param vehicle 飞控对象指针
     */
    void newPlatFind(QPlat* vehicle);
    void mavConnectionError(const QString& error);
    /**
     * @brief 飞机数量变化信号
     */
    void platsChanged();

    void dataLinksChanged();

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

    /**
     * @brief 创建或者获取ID
     * @param uId
     * @return
     */
    QPlat* getOrCreatePlat(uint8_t uId, bool bIsAutopilot);

    /**
     * @brief 处理datalink的线程
     * @param itor
     */
    void dealDataLinkThread(QMap<QDataLink*,QThread*>::Iterator itor);

private:
    friend class QGroundControlStationPrivate;
    std::unique_ptr<QGroundControlStationPrivate> d_ptr;    ///< 私有实现指针
    QMap<QDataLink*,QThread*> m_mapLink;
    QList<QObject*> m_listLink;
    QList<QObject*> m_listPlat;
    // 飞控对象管理
    QMap<uint8_t, QPlat*> m_mapId2Standalone;///< 状态
};

#endif // QGROUNDCONTROLSTATION_H
