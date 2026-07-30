#include "Link/Private/QLinkManagerPrivate.h"
#include "Link/QDataLink.h"
#include "QGroundControlStation.h"
#include "Private/QGroundControlStationPrivate.h"
#include <QString>
#include <QTimer>

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

QLinkManagerPrivate::QLinkManagerPrivate(QLinkManager *owner,
                                         QGroundControlStation *groundStation)
    : m_owner(owner)
    , m_groundStation(groundStation)
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
    if (!openConnection(link)) {
        link->deleteLater();
        return nullptr;
    }

    m_connections.insert(connStr, link);
    if (m_owner) {
        QObject::connect(link, &QObject::destroyed, m_owner,
                         [this, connStr, link]() {
            if (m_connections.value(connStr).data() == link) {
                m_connections.remove(connStr);
            }
        });
    }
    return link;
}

bool QLinkManagerPrivate::openConnection(QDataLink *link)
{
    if (!link || !m_groundStation || !m_groundStation->d_ptr) {
        return false;
    }
    if (link->linkKind() == LinkKind::Raw) {
        return m_groundStation->d_ptr->addRawConnection(link);
    }
    return m_groundStation->d_ptr->addConnection(link->connectionString());
}

void QLinkManagerPrivate::removeConnection(const QString &connStr)
{
    auto it = m_connections.find(connStr);
    if (it != m_connections.end()) {
        QPointer<QDataLink> link = it.value();
        if (m_groundStation && m_groundStation->d_ptr) {
            m_groundStation->d_ptr->removeConnection(connStr);
        }
        m_connections.erase(it);
        if (link) {
            link->deleteLater();
        }
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

void QLinkManagerPrivate::handleConnectionError(const QString &connStr,
                                                const QString &reason)
{
    QPointer<QDataLink> link = m_connections.value(connStr);
    if (!link) {
        return;
    }

    link->setConnected(false);
    link->setReconnectAttempts(0);
    if (m_owner) {
        emit m_owner->linkConnectionError(link, reason);
    }

    if (link->autoReconnect()) {
        scheduleReconnect(connStr, reason);
    }
}

void QLinkManagerPrivate::scheduleReconnect(const QString &connStr,
                                            const QString &lastError)
{
    QPointer<QDataLink> link = m_connections.value(connStr);
    if (!link || !link->autoReconnect() || !m_owner) {
        return;
    }

    const int nextAttempt = link->reconnectAttempts() + 1;
    const int maxAttempts = link->reconnectCount();
    if (maxAttempts > 0 && nextAttempt > maxAttempts) {
        emit m_owner->linkReconnectFailed(link, lastError);
        return;
    }

    link->setReconnectAttempts(nextAttempt);
    const int exponent = qMin(nextAttempt - 1, 4);
    const int delayMs = qMin(1000 * (1 << exponent), 15000);
    QPointer<QLinkManager> owner = m_owner;

    QTimer::singleShot(delayMs, owner.data(),
                       [this, owner, link, connStr, lastError]() {
        if (!owner || !link ||
            m_connections.value(connStr).data() != link.data()) {
            return;
        }
        if (!link->autoReconnect()) {
            link->setReconnectAttempts(0);
            return;
        }

        if (openConnection(link)) {
            link->setReconnectAttempts(0);
            link->setConnected(true);
            emit owner->linkReconnected(link);
            return;
        }

        scheduleReconnect(connStr, lastError);
    });
}
