#include "Link/QDataLink.h"
#include "QGroundControlStation.h"

QDataLink::QDataLink(LinkKind kind, const QString &connStr, QObject *parent)
    : QObject(parent)
    , m_linkKind(kind)
    , m_connectionString(connStr)
{
}

QDataLink::~QDataLink() = default;

void QDataLink::setReconnectCount(int count)
{
    if (m_reconnectCount != count) {
        m_reconnectCount = count;
        emit reconnectCountChanged();
    }
}

void QDataLink::setAutoReconnect(bool enable)
{
    if (m_autoReconnect != enable) {
        m_autoReconnect = enable;
        emit autoReconnectChanged();
    }
}

bool QDataLink::sendRawData(const char *data, int length)
{
    if (m_linkKind != LinkKind::Raw || !data || length <= 0) {
        return false;
    }
    QObject *p = parent();
    while (p) {
        auto *station = qobject_cast<QGroundControlStation *>(p);
        if (station) return station->feedRawData(data, length);
        p = p->parent();
    }
    return false;
}

bool QDataLink::sendRawData(const QByteArray &data)
{
    if (data.isEmpty()) return true;
    return sendRawData(data.constData(), data.size());
}

void QDataLink::emitRawDataReceived(const QByteArray &data)
{
    if (!data.isEmpty()) {
        emit rawDataReceived(data);
    }
}
