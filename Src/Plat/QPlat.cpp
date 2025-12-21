#include "Plat/QPlat.h"
#include "Plat/Private/QPlatPrivate.h"
#include <QDateTime>

QPlat::QPlat(QObject *parent)
    : QObject(parent)
{
}


void QPlat::SetPrivate(QPlatPrivate *pPlatPrivate) {
    d_ptr.reset(pPlatPrivate);
    d_ptr->setupMessageHandling();
}

QPlat::~QPlat()
{
}

QString QPlat::getFirmwareVersion() const
{
    if (!d_ptr) {
        return QString("Unknown");
    }
    return d_ptr->getFirmwareVersion();
}

QString QPlat::getSoftwareVersion() const
{
    if (!d_ptr) {
        return QString("Unknown");
    }
    return d_ptr->getSoftwareVersion();
}

bool QPlat::isConnected() const
{
    return (m_bConnected);
}

void QPlat::updateConnection(bool bConnected)
{
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
        return QString("QPlat (未初始化)");
    }
    return d_ptr->toString();
}
