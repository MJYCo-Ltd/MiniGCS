#ifndef QUDPSERVERDATALINK_H
#define QUDPSERVERDATALINK_H

#include "Link/QDataLink.h"
#include <QString>
#include <QTimer>
#include <QMap>
#include <QHostAddress>

// 前向声明
class QUdpSocket;

/**
 * @brief UDP客户端信息结构
 */
struct UdpClientInfo {
    QHostAddress address;  ///< 客户端地址
    quint16 port;          ///< 客户端端口
    qint64 lastActiveTime; ///< 最后活跃时间（毫秒时间戳）

    UdpClientInfo() : port(0), lastActiveTime(0) {}
    UdpClientInfo(const QHostAddress &addr, quint16 p, qint64 time)
        : address(addr), port(p), lastActiveTime(time) {}
};

/**
 * @brief UDP客户端键值（用于Map的key）
 */
struct UdpClientKey {
    QString address;
    quint16 port;

    UdpClientKey() : port(0) {}
    UdpClientKey(const QString &addr, quint16 p) : address(addr), port(p) {}

    bool operator<(const UdpClientKey &other) const {
        if (address != other.address) {
            return address < other.address;
        }
        return port < other.port;
    }

    bool operator==(const UdpClientKey &other) const {
        return address == other.address && port == other.port;
    }
};

/**
 * @brief QUdpServerDataLink类 - UDP服务器数据链路类
 *
 * 该类继承自QDataLink，实现基于UDP服务器的数据通信
 */
class QUdpServerDataLink : public QDataLink
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数（UDP服务器）
     * @param port 监听端口
     * @param parent 父对象
     */
    explicit QUdpServerDataLink(quint16 port, QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~QUdpServerDataLink();

    /**
     * @brief 打开UDP连接
     * @return 是否打开成功
     */
    bool connectLink() override;

    /**
     * @brief 关闭UDP连接
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
     * @brief 清理超时的客户端
     */
    void cleanupInactiveClients();
    /**
     * @brief 处理UDP数据接收
     */
    void onReadyRead();

    /**
     * @brief 线程安全的发送数据槽函数
     * @param data 要发送的数据
     */
    void onSendDataRequested(const QByteArray &data) override;

private:
    /**
     * @brief 更新或添加客户端信息
     * @param address 客户端地址
     * @param port 客户端端口
     */
    void updateClientInfo(const QHostAddress &address, quint16 port);

    /**
     * @brief 获取客户端键值
     * @param address 地址
     * @param port 端口
     * @return 客户端键值
     */
    UdpClientKey getClientKey(const QHostAddress &address, quint16 port) const;

private:
    QUdpSocket* m_udpSocket;                          ///< UDP套接字
    quint16 m_port;                                   ///< 监听端口
    QMap<UdpClientKey, UdpClientInfo> m_clientMap;   ///< 客户端列表
    QTimer* m_cleanupTimer;                           ///< 清理定时器
    static const int CLIENT_TIMEOUT_MS = 10000;       ///< 客户端超时时间（10秒）
};

#endif // QUDPSERVERDATALINK_H
