#ifndef QUDPCLIENTDATALINK_H
#define QUDPCLIENTDATALINK_H

#include "Link/QDataLink.h"
#include <QString>
#include <QHostAddress>

// 前向声明
class QUdpSocket;

/**
 * @brief QUdpClientDataLink类 - UDP客户端数据链路类
 *
 * 该类继承自QDataLink，实现作为UDP客户端连接远程服务器（固定目标地址）的数据通信
 */
class QUdpClientDataLink : public QDataLink
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数（UDP客户端）
     * @param hostName 远程主机名或IP地址
     * @param port 远程端口
     * @param parent 父对象
     */
    explicit QUdpClientDataLink(const QString &hostName, quint16 port, QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~QUdpClientDataLink();

    bool connectLink() override;
    void disConnectLink() override;
    bool sendLinkData(const QByteArray &data) override;
    bool isLinkConnected() const override;

    QString hostName() const { return m_hostName; }
    quint16 port() const { return m_port; }

private slots:
    void onReadyRead();
    void onSendDataRequested(const QByteArray &data) override;

private:
    QUdpSocket *m_udpSocket;
    QString m_hostName;
    quint16 m_port;
    QHostAddress m_remoteAddress;
};

#endif // QUDPCLIENTDATALINK_H
