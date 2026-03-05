#ifndef QDATALINK_H
#define QDATALINK_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QtGlobal>
#include "Link/QLinkManager.h"
#include "MiniGCSExport.h"

/**
 * @brief QDataLink类 - 数据链路
 *
 * 表示一条链路连接，可设置重连次数和自动重连。
 * Raw 模式下支持发送和接收原始数据（字符串、长度）。
 */
class MINIGCS_EXPORT QDataLink : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int reconnectCount READ reconnectCount WRITE setReconnectCount NOTIFY reconnectCountChanged)
    Q_PROPERTY(bool autoReconnect READ autoReconnect WRITE setAutoReconnect NOTIFY autoReconnectChanged)
    Q_PROPERTY(LinkKind linkKind READ linkKind CONSTANT)
    Q_PROPERTY(QString connectionString READ connectionString CONSTANT)

public:
    explicit QDataLink(LinkKind kind, const QString &connStr, QObject *parent = nullptr);
    ~QDataLink();

    LinkKind linkKind() const { return m_linkKind; }
    QString connectionString() const { return m_connectionString; }

    /** 重连次数，0 表示不限制 */
    int reconnectCount() const { return m_reconnectCount; }
    void setReconnectCount(int count);

    /** 是否开启自动重连 */
    bool autoReconnect() const { return m_autoReconnect; }
    void setAutoReconnect(bool enable);

    /**
     * @brief 发送原始数据（仅 Raw 模式有效）
     * @param data 数据指针
     * @param length 数据长度
     * @return 是否发送成功
     */
    Q_INVOKABLE bool sendRawData(const char *data, int length);

    /**
     * @brief 发送原始数据（QByteArray 重载）
     */
    Q_INVOKABLE bool sendRawData(const QByteArray &data);

signals:
    void reconnectCountChanged();
    void autoReconnectChanged();

    /**
     * @brief 接收到原始数据（仅 Raw 模式）
     * @param data 数据，可用 data.constData() 和 data.size() 获取指针和长度
     */
    void rawDataReceived(const QByteArray &data);

private slots:
    /** 内部使用：触发 rawDataReceived 信号 */
    void emitRawDataReceived(const QByteArray &data);

private:
    friend class QLinkManagerPrivate;
    LinkKind m_linkKind;
    QString m_connectionString;
    int m_reconnectCount{0};
    bool m_autoReconnect{false};
};

#endif // QDATALINK_H
