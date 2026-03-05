#ifndef QLINKMANAGER_H
#define QLINKMANAGER_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include "MiniGCSExport.h"

class QLinkManagerPrivate;
class QGroundControlStation;
class QDataLink;

/**
 * @brief 链路类型枚举
 *
 * 用于判断并生成连接字符串
 */
enum class LinkKind {
    TcpServer,   ///< TCP 服务器（监听）
    TcpClient,   ///< TCP 客户端（连接）
    UdpServer,   ///< UDP 服务器（监听）
    UdpClient,   ///< UDP 客户端（连接）
    Serial,      ///< 串口
    Raw          ///< 原始字节（自定义 I/O）
};

/**
 * @brief 链路参数结构体
 *
 * 根据 LinkKind 使用不同字段：Tcp/Udp Server 用 port；Client 用 hostName+port；Serial 用 portName+baudRate
 */
struct LinkParams {
    quint16 port{0};
    QString hostName;
    QString portName;   ///< 串口名称（Serial 专用）
    int baudRate{0};    ///< 波特率（Serial 专用）
};

/**
 * @brief QLinkManager类 - 链路管理器
 *
 * 通过 LinkKind 和 LinkParams 创建/移除连接。
 */
class MINIGCS_EXPORT QLinkManager : public QObject
{
    Q_OBJECT

public:
    explicit QLinkManager(QGroundControlStation *groundStation, QObject *parent = nullptr);
    ~QLinkManager();

    /**
     * @brief 根据 LinkKind 和 LinkParams 新增链路
     * @param type 链路类型
     * @param params 链路参数
     * @return 成功返回 QDataLink*，失败返回 nullptr
     */
    Q_INVOKABLE QDataLink *addLink(LinkKind type, const LinkParams &params);

    /**
     * @brief 根据 LinkKind 和 LinkParams 移除链路
     * @param type 链路类型
     * @param params 链路参数
     */
    Q_INVOKABLE void removeLink(LinkKind type, const LinkParams &params);

    /**
     * @brief 移除指定链路
     * @param link 链路对象
     */
    Q_INVOKABLE void removeLink(QDataLink *link);

    /**
     * @brief 清除所有连接
     */
    Q_INVOKABLE void clearAll();

    /**
     * @brief 根据链路类型和参数构建连接字符串
     * @param type 链路类型
     * @param params 链路参数
     * @return 连接字符串
     */
    Q_INVOKABLE static QString buildConnectionString(LinkKind type, const LinkParams &params);

signals:
    /** 创建失败（如重复连接）时发出 */
    void linkCreateFailed(const QString &reason);

private:
    QLinkManagerPrivate *d_func() { return d_ptr.get(); }
    const QLinkManagerPrivate *d_func() const { return d_ptr.get(); }
    std::unique_ptr<QLinkManagerPrivate> d_ptr;
};

#endif // QLINKMANAGER_H
