#include "Link/Private/QDataLinkPrivate.h"
#include "Link/QDataLink.h"
#include <QDateTime>
#include <QMetaObject>
#include <mavsdk/mavlink_include.h>

struct QDataLinkPrivate::MavlinkParseState {
    mavlink_message_t message;
    mavlink_status_t status;
};

QDataLinkPrivate::QDataLinkPrivate(QDataLink *q) : q_ptr(q) {
    m_parseState = new MavlinkParseState();
    m_parseState->status.parse_state = MAVLINK_PARSE_STATE_IDLE;
    m_parseState->status.packet_rx_success_count = 0;
    m_parseState->status.packet_rx_drop_count = 0;
}

QDataLinkPrivate::~QDataLinkPrivate() {
    delete m_parseState;
    m_parseState = nullptr;
}

quint64 QDataLinkPrivate::mavlinkPacketsReceived() const {
    return m_parseState ? static_cast<quint64>(
                              m_parseState->status.packet_rx_success_count)
                        : 0;
}

quint32 QDataLinkPrivate::mavlinkPacketsDropped() const {
    return m_parseState ? m_parseState->status.packet_rx_drop_count : 0;
}

void QDataLinkPrivate::updateSignalQuality() {
    if (!m_parseState) {
        if (m_signalQuality != 1.0 && q_ptr) {
            m_signalQuality = 1.0;
            QMetaObject::invokeMethod(q_ptr, "signalQualityChanged",
                                      Qt::QueuedConnection, Q_ARG(double, m_signalQuality));
        } else {
            m_signalQuality = 1.0;
        }
        return;
    }
    quint32 received = m_parseState->status.packet_rx_success_count;
    quint32 dropped = m_parseState->status.packet_rx_drop_count;
    qDebug()<<received<<','<<dropped;
    double newQuality = 1.0;
    if (received + dropped > 0) {
        newQuality =
            static_cast<double>(received) / static_cast<double>(received + dropped);
        if (newQuality > 1.0)
            newQuality = 1.0;
        if (newQuality < 0.0)
            newQuality = 0.0;
    }

    if (m_signalQuality != newQuality && q_ptr) {
        m_signalQuality = newQuality;
        QMetaObject::invokeMethod(q_ptr, "signalQualityChanged",
                                  Qt::QueuedConnection, Q_ARG(double, m_signalQuality));
    }
}

int QDataLinkPrivate::feedBytes(const uint8_t *data, int len) {
    if (!data || len <= 0 || !m_parseState)
        return 0;

    int completeCount = 0;
    const uint8_t channel = q_ptr ? q_ptr->index() : 0;

    for (int i = 0; i < len; ++i) {
        uint8_t result = mavlink_parse_char(
            channel, data[i], &m_parseState->message, &m_parseState->status);

        if (result == MAVLINK_FRAMING_OK) {
            ++completeCount;
            const qint64 now = QDateTime::currentMSecsSinceEpoch();
            if (m_lastMessageTime != now && q_ptr) {
                m_lastMessageTime = now;
                QMetaObject::invokeMethod(q_ptr, "lastMessageTimeChanged", Qt::QueuedConnection);
            } else {
                m_lastMessageTime = now;
            }

            if (m_parseState->message.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
                if (m_lastHeartbeatTime != m_lastMessageTime && q_ptr) {
                    m_lastHeartbeatTime = m_lastMessageTime;
                    QMetaObject::invokeMethod(q_ptr, "lastHeartbeatTimeChanged", Qt::QueuedConnection);
                } else {
                    m_lastHeartbeatTime = m_lastMessageTime;
                }
            }
        }
    }

    updateSignalQuality();
    return completeCount;
}

void QDataLinkPrivate::resetMavlinkStatistics() {
    m_lastMessageTime = 0;
    m_lastHeartbeatTime = 0;
    m_signalQuality = 1.0;

    if (m_parseState) {
        m_parseState->status.packet_rx_success_count = 0;
        m_parseState->status.packet_rx_drop_count = 0;
    }
    if (q_ptr) {
        QMetaObject::invokeMethod(q_ptr, "lastMessageTimeChanged", Qt::QueuedConnection);
        QMetaObject::invokeMethod(q_ptr, "lastHeartbeatTimeChanged", Qt::QueuedConnection);
        QMetaObject::invokeMethod(q_ptr, "signalQualityChanged",
                                  Qt::QueuedConnection, Q_ARG(double, m_signalQuality));
    }
}
