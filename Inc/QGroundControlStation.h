#ifndef QGROUNDCONTROLSTATION_H
#define QGROUNDCONTROLSTATION_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <cstdint>
#include "MiniGCSExport.h"

// 前向声明
class QPlat;
class QLinkManager;

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
     * @brief 获取链路管理器
     */
    QLinkManager *linkManager() const { return m_linkManager; }

    /**
     * @brief 清除所有链路连接
     */
    void ClearAllLinks();

    /**
     * @brief 获取所有飞控对象
     * @return 飞控对象列表
     */
    Q_INVOKABLE QList<QObject*> plats() const{return(m_listPlat);}

    /**
     * @brief 向 Raw 链路发送数据（供 QDataLink 使用）
     * @param data 数据指针
     * @param length 数据长度
     * @return 是否成功
     */
    bool feedRawData(const char *data, int length);

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

private:
    /**
     * @brief 创建或者获取ID
     * @param uId
     * @return
     */
    QPlat* getOrCreatePlat(uint8_t uId, bool bIsAutopilot);

private:
    friend class QGroundControlStationPrivate;
    friend class QLinkManagerPrivate;
    friend class QDataLink;
    std::unique_ptr<QGroundControlStationPrivate> d_ptr;    ///< 私有实现指针
    QLinkManager *m_linkManager{nullptr};
    QList<QObject*> m_listPlat;
    // 飞控对象管理
    QMap<uint8_t, QPlat*> m_mapId2Standalone;///< 状态
};

#endif // QGROUNDCONTROLSTATION_H
