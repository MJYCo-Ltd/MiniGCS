#ifndef QTCPCLIENTDATALINK_H
#define QTCPCLIENTDATALINK_H

#include "Link/QDataLink.h"
#include <QString>

// 前向声明
class QTcpSocket;

/**
 * @brief QTcpClientDataLink类 - TCP客户端数据链路类
 *
 * 该类继承自QDataLink，实现作为TCP客户端连接远程服务器的数据通信
 */
class QTcpClientDataLink : public QDataLink
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数（TCP客户端）
     * @param hostName 远程主机名或IP地址
     * @param port 远程端口
     * @param parent 父对象
     */
    explicit QTcpClientDataLink(const QString &hostName, quint16 port, QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~QTcpClientDataLink();

    bool connectLink() override;
    void disConnectLink() override;
    bool sendLinkData(const QByteArray &data) override;
    bool isLinkConnected() const override;

    /**
     * @brief 获取远程主机名
     */
    QString hostName() const { return m_hostName; }

    /**
     * @brief 获取远程端口
     */
    quint16 port() const { return m_port; }

private slots:
    void onSocketReadyRead();
    void onErrorOccurred();
    void onSocketDisconnected();
    void onSendDataRequested(const QByteArray &data) override;

private:
    QTcpSocket *m_tcpSocket;
    QString m_hostName;
    quint16 m_port;
};

#endif // QTCPCLIENTDATALINK_H
