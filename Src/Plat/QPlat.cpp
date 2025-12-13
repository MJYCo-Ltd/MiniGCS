#include "QPlat.h"
#include "Private/QPlatPrivate.h"
#include <QDateTime>

QPlat::QPlat(QObject *parent)
{
}


void QPlat::SetPrivate(std::unique_ptr<QPlatPrivate> &pPlatPrivate) {
    d_ptr.reset(pPlatPrivate.get());
    d_ptr->setupMessageHandling(this);
}

QPlat::~QPlat()
{
}

QString QPlat::getFirmwareVersion() const
{
    return d_ptr->getFirmwareVersion();
}

QString QPlat::getHardwareVersion() const
{
    return d_ptr->getHardwareVersion();
}

QString QPlat::getSoftwareVersion() const
{
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

bool QPlat::hasCamera() const
{
    return d_ptr->hasCamera();
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
    return d_ptr->toString();
}
