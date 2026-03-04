#include "Link/QDataLink.h"
#include "Link/Private/QDataLinkPrivate.h"

QDataLink::QDataLink(QObject *parent)
    : QObject(parent)
    , d_ptr(new QDataLinkPrivate(this))
{
    connect(this, &QDataLink::needReLink, this, &QDataLink::onReLink);
}

QDataLink::~QDataLink() = default;

void QDataLink::feedReceivedData(const QByteArray &data)
{
    if (d_ptr && !data.isEmpty())
        d_ptr->feedBytes(reinterpret_cast<const uint8_t *>(data.constData()), data.size());
}

quint64 QDataLink::packetsReceived() const
{
    return d_ptr ? d_ptr->mavlinkPacketsReceived() : 0;
}

quint32 QDataLink::packetsDropped() const
{
    return d_ptr ? d_ptr->mavlinkPacketsDropped() : 0;
}

qint64 QDataLink::lastMessageTime() const
{
    return d_ptr ? d_ptr->lastMessageTime() : 0;
}

qint64 QDataLink::lastHeartbeatTime() const
{
    return d_ptr ? d_ptr->lastHeartbeatTime() : 0;
}

double QDataLink::signalQuality() const
{
    return d_ptr ? d_ptr->mavlinkSignalQuality() : 1.0;
}

void QDataLink::resetLinkStatistics()
{
    if (d_ptr)
        d_ptr->resetMavlinkStatistics();
}
