#ifndef QDATALINK_H
#define QDATALINK_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QtGlobal>
#include "Link/QLinkManager.h"
#include "MiniGCSExport.h"

/**
 * @brief QDataLink - 单条数据链路
 *
 * 表示一条链路连接，可设置重连次数和自动重连。
 * Raw 模式为高级能力，支持收发原始字节。
 */
class MINIGCS_EXPORT QDataLink : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int reconnectCount READ reconnectCount WRITE setReconnectCount NOTIFY reconnectCountChanged)
    Q_PROPERTY(bool autoReconnect READ autoReconnect WRITE setAutoReconnect NOTIFY autoReconnectChanged)
    Q_PROPERTY(bool opened READ isOpened NOTIFY openStatusChanged)
    Q_PROPERTY(int reconnectAttempts READ reconnectAttempts NOTIFY reconnectAttemptsChanged)
    Q_PROPERTY(LinkKind linkKind READ linkKind CONSTANT)

public:
    ~QDataLink();

    LinkKind linkKind() const { return m_linkKind; }

    int reconnectCount() const { return m_reconnectCount; }
    void setReconnectCount(int count);

    bool autoReconnect() const { return m_autoReconnect; }
    void setAutoReconnect(bool enable);

    bool isOpened() const { return m_opened; }
    int reconnectAttempts() const { return m_reconnectAttempts; }

    /**
     * @brief 发送原始数据（仅 Raw 高级模式）
     */
    bool sendRawData(const char *data, int length);
    Q_INVOKABLE bool sendRawData(const QByteArray &data);

signals:
    void reconnectCountChanged();
    void autoReconnectChanged();
    void openStatusChanged(bool opened);
    void reconnectAttemptsChanged(int attempts);
    /** 接收到原始数据（仅 Raw 高级模式） */
    void rawDataReceived(const QByteArray &data);

private slots:
    void emitRawDataReceived(const QByteArray &data);

private:
    friend class QLinkManagerPrivate;
    explicit QDataLink(LinkKind kind, const QString &connStr,
                       QObject *parent = nullptr);
    QString connectionString() const { return m_connectionString; }
    void setOpened(bool opened);
    void setReconnectAttempts(int attempts);

    LinkKind m_linkKind;
    QString m_connectionString;
    int m_reconnectCount{0};
    bool m_autoReconnect{false};
    bool m_opened{true};
    int m_reconnectAttempts{0};
};

#endif // QDATALINK_H
