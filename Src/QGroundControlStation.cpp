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
    // 先销毁飞控插件并取消其订阅，再由 Mavsdk 析构统一停止 I/O 线程和连接。
    // 当前 MAVSDK 已允许安全地显式 remove_connection；这里保留统一析构，避免
    // 在退出路径重复逐条清理同一批连接。
    const QList<QPlat *> platformChildren =
        findChildren<QPlat *>(QString(), Qt::FindDirectChildrenOnly);
    qDeleteAll(platformChildren);
    m_mapId2Standalone.clear();

    d_ptr.reset();
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
    if (!d_ptr || !d_ptr->mavsdk() || !data || length <= 0) {
        return false;
    }
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
        pPlat->setSystemId(uId);
        m_mapId2Standalone.insert(uId, pPlat);
        registryChanged = true;
    } else {
        if (!bIsAutopilot) {
            if (nullptr != qobject_cast<QAutopilot *>(pPlat)) {
                pPlat->deleteLater();
                pPlat = new QPlat(this);
                pPlat->setSystemId(uId);
                m_mapId2Standalone[uId] = pPlat;
                registryChanged = true;
            }
        } else {
            if (nullptr == qobject_cast<QAutopilot *>(pPlat)) {
                pPlat->deleteLater();
                pPlat = new QAutopilot(this);
                pPlat->setSystemId(uId);
                m_mapId2Standalone[uId] = pPlat;
                registryChanged = true;
            }
        }
    }

    if (registryChanged) {
        connect(pPlat, &QPlat::componentsChanged, this,
                [this, uId]() {
                    if (d_ptr) {
                        d_ptr->refreshConnectedSystem(this, uId);
                    }
                });
        emit platsChanged();
    }

    return (pPlat);
}
