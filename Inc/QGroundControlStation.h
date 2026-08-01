#ifndef QGROUNDCONTROLSTATION_H
#define QGROUNDCONTROLSTATION_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <cstdint>
#include <memory>
#include "Link/QLinkManager.h"
#include "MiniGCSExport.h"

class QPlat;
class QDataLink;

/**
 * @brief QGroundControlStation - 地面控制站
 */
class MINIGCS_EXPORT QGroundControlStation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QLinkManager *linkManager READ linkManager CONSTANT)

public:
    explicit QGroundControlStation(QObject *parent = nullptr);
    ~QGroundControlStation();

    void Init();
    QLinkManager *linkManager() const { return m_linkManager; }
    void ClearAllLinks();
    Q_INVOKABLE QList<QObject*> plats() const;

signals:
    void newPlatFind(QPlat* vehicle);
    /** 链路/连接错误（业务层可读描述） */
    void connectionError(const QString& error);
    void platsChanged();

private:
    QPlat* getOrCreatePlat(uint8_t uId, bool bIsAutopilot);

    /**
     * @brief 高级/内部：向 Raw 链路注入接收字节
     * @note 仅供 QDataLink Raw 模式使用，不属于常规业务 API
     */
    bool feedRawData(const char *data, int length);

private:
    friend class QGroundControlStationPrivate;
    friend class QLinkManagerPrivate;
    friend class QDataLink;
    std::unique_ptr<QGroundControlStationPrivate> d_ptr;
    QLinkManager *m_linkManager{nullptr};
    QMap<uint8_t, QPlat*> m_mapId2Standalone;
};

#endif // QGROUNDCONTROLSTATION_H
