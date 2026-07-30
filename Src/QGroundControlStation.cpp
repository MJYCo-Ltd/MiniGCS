#include "QGroundControlStation.h"
#include "Link/QLinkManager.h"
#include "Plat/QAutopilot.h"
#include "Private/QGroundControlStationPrivate.h"
#include <QCoreApplication>
#include <QDebug>
#include <QtAlgorithms>

QGroundControlStation::QGroundControlStation(QObject *parent)
    : QObject(parent)
    , d_ptr(new QGroundControlStationPrivate)
    , m_linkManager(new QLinkManager(this, this))
{
}

QGroundControlStation::~QGroundControlStation()
{
    ClearAllLinks();

    // QPlat 是 QObject 子对象，必须在 MAVSDK 私有实现销毁前停止其订阅和后台任务。
    const QList<QPlat *> platformChildren =
        findChildren<QPlat *>(QString(), Qt::FindDirectChildrenOnly);
    qDeleteAll(platformChildren);
    m_mapId2Standalone.clear();
}

void QGroundControlStation::Init()
{
    d_ptr->initializeMavsdk();
    d_ptr->setupConnectionErrorHandling(this);
    d_ptr->setupNewSystemDiscoveryCallback(this);
}

void QGroundControlStation::ClearAllLinks()
{
    if (m_linkManager) {
        m_linkManager->clearAll();
    }
}

bool QGroundControlStation::feedRawData(const char *data, int length)
{
    if (!d_ptr || !data || length <= 0) return false;
    d_ptr->processReceivedRawData(QByteArray(data, length));
    return true;
}

QList<QObject *> QGroundControlStation::plats() const
{
    QList<QObject *> result;
    result.reserve(m_mapId2Standalone.size());
    for (QPlat *plat : m_mapId2Standalone) {
        result.append(plat);
    }
    return result;
}

QPlat *QGroundControlStation::getOrCreatePlat(uint8_t uId, bool bIsAutopilot)
{
    QPlat *pPlat = m_mapId2Standalone.value(uId, nullptr);
    bool registryChanged = false;
    if (nullptr == pPlat) {
        if (bIsAutopilot) {
            pPlat = new QAutopilot(this);
        } else {
            pPlat = new QPlat(this);
        }
        m_mapId2Standalone.insert(uId, pPlat);
        registryChanged = true;
    } else {
        if (!bIsAutopilot) {
            if (nullptr != qobject_cast<QAutopilot *>(pPlat)) {
                pPlat->deleteLater();
                pPlat = new QPlat(this);
                m_mapId2Standalone[uId] = pPlat;
                registryChanged = true;
            }
        } else {
            if (nullptr == qobject_cast<QAutopilot *>(pPlat)) {
                pPlat->deleteLater();
                pPlat = new QAutopilot(this);
                m_mapId2Standalone[uId] = pPlat;
                registryChanged = true;
            }
        }
    }

    if (registryChanged) {
        emit platsChanged();
    }

    return (pPlat);
}
