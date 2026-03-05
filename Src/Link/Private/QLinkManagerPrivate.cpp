#include "Link/Private/QLinkManagerPrivate.h"
#include "Link/QDataLink.h"
#include "QGroundControlStation.h"
#include "Private/QGroundControlStationPrivate.h"
#include <QString>

QString QLinkManagerPrivate::buildConnectionString(LinkKind type, const LinkParams &params)
{
    switch (type) {
    case LinkKind::TcpServer:
        return QString("tcpin://0.0.0.0:%1").arg(params.port);
    case LinkKind::TcpClient:
        return QString("tcpout://%1:%2").arg(params.hostName).arg(params.port);
    case LinkKind::UdpServer:
        return QString("udpin://0.0.0.0:%1").arg(params.port);
    case LinkKind::UdpClient:
        return QString("udpout://%1:%2").arg(params.hostName).arg(params.port);
    case LinkKind::Serial:
        return QString("serial://%1:%2").arg(params.portName).arg(params.baudRate);
    case LinkKind::Raw:
        return QString("raw://");
    default:
        return QString();
    }
}

QLinkManagerPrivate::QLinkManagerPrivate(QGroundControlStation *groundStation)
    : m_groundStation(groundStation)
{
}

QLinkManagerPrivate::~QLinkManagerPrivate() = default;

bool QLinkManagerPrivate::hasConnection(const QString &connStr) const
{
    return m_connections.contains(connStr);
}

QDataLink *QLinkManagerPrivate::addConnection(LinkKind type, const QString &connStr)
{
    if (connStr.isEmpty() || hasConnection(connStr)) {
        return nullptr;
    }
    if (!m_groundStation || !m_groundStation->d_ptr) {
        return nullptr;
    }

    QDataLink *link = new QDataLink(type, connStr, m_groundStation);

    if (type == LinkKind::Raw) {
        if (!m_groundStation->d_ptr->addRawConnection(link)) {
            link->deleteLater();
            return nullptr;
        }
    } else {
        if (!m_groundStation->d_ptr->addConnection(connStr)) {
            link->deleteLater();
            return nullptr;
        }
    }
    m_connections.insert(connStr, link);
    return link;
}

void QLinkManagerPrivate::removeConnection(const QString &connStr)
{
    auto it = m_connections.find(connStr);
    if (it != m_connections.end()) {
        QDataLink *link = it.value();
        if (m_groundStation && m_groundStation->d_ptr) {
            m_groundStation->d_ptr->removeConnection(connStr);
        }
        m_connections.erase(it);
        link->deleteLater();
    }
}

void QLinkManagerPrivate::removeLink(QDataLink *link)
{
    if (!link) return;
    QString connStr = link->connectionString();
    if (m_connections.contains(connStr)) {
        removeConnection(connStr);
    }
}

QStringList QLinkManagerPrivate::connectionStrings() const
{
    return m_connections.keys();
}
