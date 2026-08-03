#include "Plat/QPlat.h"
#include "Plat/Private/QPlatPrivate.h"
#include <QDateTime>

QPlat::QPlat(QObject *parent)
    : QObject(parent)
{
}


void QPlat::SetPrivate(QPlatPrivate *pPlatPrivate) {
    d_ptr.reset(pPlatPrivate);
    connect(this, &QPlat::connectionStatusChanged,
            this, &QPlat::updateConnection, Qt::UniqueConnection);
    d_ptr->setupMessageHandling();
}

QPlat::~QPlat()
{
}

QString QPlat::getFirmwareVersion() const
{
    if (!d_ptr) {
        return tr("未知");
    }
    return d_ptr->getFirmwareVersion();
}

QString QPlat::getSoftwareVersion() const
{
    if (!d_ptr) {
        return tr("未知");
    }
    return d_ptr->getSoftwareVersion();
}

bool QPlat::isConnected() const
{
    return (m_bConnected);
}

void QPlat::updateConnection(bool bConnected)
{
    if (m_bConnected == bConnected) {
        return;
    }
    m_bConnected = bConnected;

    if (m_bConnected) {
        m_lastConnectedTime = QDateTime::currentDateTime();
    } else {
        m_lastDisconnectedTime = QDateTime::currentDateTime();
    }
}

QDateTime QPlat::getLastConnectedTime() const
{
    return (m_lastConnectedTime);
}


QDateTime QPlat::getLastDisconnectedTime() const
{
    return(m_lastDisconnectedTime);
}

QString QPlat::toString() const
{
    if (!d_ptr) {
        return tr("QPlat（未初始化）");
    }
    return d_ptr->toString();
}
