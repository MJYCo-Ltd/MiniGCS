#ifndef QTCPSERVERDATALINK_H
#define QTCPSERVERDATALINK_H

#include "Link/QDataLink.h"
#include <QString>
#include <QList>

// 前向声明
class QTcpSocket;
class QTcpServer;

/**
 * @brief QTcpServerDataLink类 - TCP服务器数据链路类
 *
 * 该类继承自QDataLink，实现基于TCP服务器的数据通信
 */
class QTcpServerDataLink : public QDataLink
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数（TCP服务器）
     * @param port 监听端口
     * @param parent 父对象
     */
    explicit QTcpServerDataLink(quint16 port, QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~QTcpServerDataLink();

    /**
     * @brief 打开TCP连接
     * @return 是否打开成功
     */
    bool connectLink() override;

    /**
     * @brief 关闭TCP连接
     */
    void disConnectLink() override;

    /**
     * @brief 发送数据
     * @param data 要发送的数据
     * @return 是否发送成功
     */
    bool sendLinkData(const QByteArray &data) override;

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isLinkConnected() const override;

    /**
     * @brief 获取端口
     * @return 端口
     */
    quint16 port() const{return(m_port);}

private slots:
    /**
     * @brief 处理socket的数据接收
     */
    void onSocketReadyRead();

    /**
     * @brief 处理TCP连接错误
     */
    void onErrorOccurred();

    /**
     * @brief 处理新连接
     */
    void onNewConnection();

    /**
     * @brief 处理socket的断开连接
     */
    void onSocketDisconnected();

    /**
     * @brief 线程安全的发送数据槽函数
     * @param data 要发送的数据
     */
    void onSendDataRequested(const QByteArray &data) override;

private:
    /**
     * @brief 从客户端列表中移除socket
     * @param socket 要移除的socket
     */
    void removeClientSocket(QTcpSocket* socket);

private:
    QTcpServer* m_tcpServer;              ///< TCP服务器
    QList<QTcpSocket*> m_clientSockets;   ///< 客户端socket列表
    quint16 m_port;                       ///< 监听端口
};

#endif // QTCPSERVERDATALINK_H
