#ifndef QDATALINK_H
#define QDATALINK_H

#include <QObject>
#include <QByteArray>
#include <memory>
#include "MiniGCSExport.h"

class QDataLinkPrivate;

/**
 * @brief QDataLink类 - 数据链路基类
 *
 * 定义数据链路基本接口；通过 QDataLinkPrivate 统计接收包数、丢包、最后心跳及信号质量。
 */
class MINIGCS_EXPORT QDataLink : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 lastMessageTime READ lastMessageTime NOTIFY lastMessageTimeChanged)
    Q_PROPERTY(qint64 lastHeartbeatTime READ lastHeartbeatTime NOTIFY lastHeartbeatTimeChanged)
    Q_PROPERTY(double signalQuality READ signalQuality NOTIFY signalQualityChanged)

public:
    explicit QDataLink(QObject *parent = nullptr);
    virtual ~QDataLink();

    void setIndex(quint8 uIndex) { m_unIndex = uIndex; }
    quint8 index() const { return m_unIndex; }

    virtual bool sendLinkData(const QByteArray &data) = 0;
    virtual bool isLinkConnected() const = 0;
    Q_INVOKABLE virtual bool connectLink() = 0;
    Q_INVOKABLE virtual void disConnectLink() = 0;

    /** 将接收到的原始数据喂入解析器并更新链路状态统计 */
    void feedReceivedData(const QByteArray &data);

    Q_INVOKABLE quint64 packetsReceived() const;
    Q_INVOKABLE quint32 packetsDropped() const;
    Q_INVOKABLE qint64 lastMessageTime() const;
    Q_INVOKABLE qint64 lastHeartbeatTime() const;
    /** 信号质量 [0.0, 1.0] */
    Q_INVOKABLE double signalQuality() const;
    Q_INVOKABLE void resetLinkStatistics();

signals:
    void linkStatusChanged(bool connected);
    void messageReceived(const QByteArray &data);
    void linkError(const QString &);
    void needReLink();
    void lastMessageTimeChanged();
    void lastHeartbeatTimeChanged();
    /** 信号质量变化时发出，参数为 [0.0, 1.0] */
    void signalQualityChanged(double quality);

protected slots:
    virtual void onSendDataRequested(const QByteArray &data) = 0;
    void onReLink() { disConnectLink(); connectLink(); }

protected:
    QDataLinkPrivate *d_func() { return d_ptr.get(); }
    const QDataLinkPrivate *d_func() const { return d_ptr.get(); }
    quint8 m_unIndex{};

private:
    std::unique_ptr<QDataLinkPrivate> d_ptr;
};
#endif // QDATALINK_H

