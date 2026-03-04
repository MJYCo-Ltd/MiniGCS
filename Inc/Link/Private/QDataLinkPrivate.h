#ifndef QDATALINKPRIVATE_H
#define QDATALINKPRIVATE_H

#include <QtGlobal>

class QDataLink;

/**
 * @brief QDataLinkPrivate - 基于 mavlink_parse_char 的 MAVLink 解析与状态统计
 *
 * 使用 mavsdk/mavlink_include.h 提供的 mavlink_parse_char 对接收字节流解析，
 * 统计完整帧数、丢包数、最后心跳等，并根据 mavlink_status_t 评估信号质量。
 */
class QDataLinkPrivate {
public:
    explicit QDataLinkPrivate(QDataLink *q);
    ~QDataLinkPrivate();

    /**
   * @brief 向解析器喂入接收到的字节
   * @param data 原始字节
   * @return 本段数据中解析出的完整 MAVLink 消息条数
   */
    int feedBytes(const uint8_t *data, int len);

    quint64 mavlinkPacketsReceived() const;
    quint32 mavlinkPacketsDropped() const;
    qint64 lastMessageTime() const { return m_lastMessageTime; }
    qint64 lastHeartbeatTime() const { return m_lastHeartbeatTime; }

    /**
   * @brief 根据 mavlink_status_t 评估的信号质量 [0.0, 1.0]
   * 基于 成功接收数/(成功+丢包) 计算，无数据时为 1.0
   */
    double mavlinkSignalQuality() const { return m_signalQuality; }

    void resetMavlinkStatistics();

private:
    void updateSignalQuality();

    QDataLink *q_ptr;
    qint64 m_lastMessageTime{0};
    qint64 m_lastHeartbeatTime{0};
    double m_signalQuality{1.0};

    struct MavlinkParseState;
    MavlinkParseState *m_parseState{nullptr};
};

#endif // QDATALINKPRIVATE_H
