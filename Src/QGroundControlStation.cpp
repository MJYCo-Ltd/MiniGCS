#include "QGroundControlStation.h"
#include "Link/QLinkManager.h"
#include "Plat/QAutopilot.h"
#include "Private/QGroundControlStationPrivate.h"
#include <QCoreApplication>
#include <QDebug>

QGroundControlStation::QGroundControlStation(QObject *parent)
    : QObject(parent)
    , d_ptr(new QGroundControlStationPrivate)
    , m_linkManager(new QLinkManager(this, this))
{
}

QGroundControlStation::~QGroundControlStation()
{
    ClearAllLinks();
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

QPlat *QGroundControlStation::getOrCreatePlat(uint8_t uId, bool bIsAutopilot)
{
    QPlat *pPlat = m_mapId2Standalone.value(uId, nullptr);
    bool bNewPlatCreated = false;
    if (nullptr == pPlat) {
        if (bIsAutopilot) {
            pPlat = new QAutopilot(this);
        } else {
            pPlat = new QPlat(this);
        }
        m_mapId2Standalone.insert(uId, pPlat);
        m_listPlat.push_back(pPlat);
        bNewPlatCreated = true;
    } else {
        if (!bIsAutopilot) {
            if (nullptr != qobject_cast<QAutopilot *>(pPlat)) {
                pPlat->deleteLater();
                pPlat = new QPlat(this);
                m_mapId2Standalone[uId] = pPlat;
            }
        } else {
            if (nullptr == qobject_cast<QAutopilot *>(pPlat)) {
                pPlat->deleteLater();
                pPlat = new QAutopilot(this);
                m_mapId2Standalone[uId] = pPlat;
            }
        }
    }

    if (bNewPlatCreated) {
        emit platsChanged();
    }

    return (pPlat);
}
