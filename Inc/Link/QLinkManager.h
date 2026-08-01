#ifndef QLINKMANAGER_H
#define QLINKMANAGER_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QMetaType>
#include <memory>
#include "MiniGCSExport.h"

class QLinkManagerPrivate;
class QGroundControlStation;
class QDataLink;

/**
 * @brief 链路类型
 */
class MINIGCS_EXPORT LinkTypes
{
    Q_GADGET
public:
    enum class Kind {
        TcpServer,
        TcpClient,
        UdpServer,
        UdpClient,
        Serial,
        Raw  ///< 高级：自定义原始字节 I/O，非常规业务链路
    };
    Q_ENUM(Kind)
};
using LinkKind = LinkTypes::Kind;

/**
 * @brief 链路参数
 *
 * Server：hostName 为绑定地址（空则绑定全部网卡）+ port；
 * Client：hostName + port；Serial：portName + baudRate。
 */
struct MINIGCS_EXPORT LinkParams {
    Q_GADGET
    Q_PROPERTY(quint16 port MEMBER port)
    Q_PROPERTY(QString hostName MEMBER hostName)
    Q_PROPERTY(QString portName MEMBER portName)
    Q_PROPERTY(int baudRate MEMBER baudRate)
public:
    quint16 port{0};
    QString hostName;
    QString portName;
    int baudRate{0};
};

inline bool operator==(const LinkParams &lhs, const LinkParams &rhs)
{
    return lhs.port == rhs.port
        && lhs.baudRate == rhs.baudRate
        && lhs.hostName == rhs.hostName
        && lhs.portName == rhs.portName;
}

inline bool operator!=(const LinkParams &lhs, const LinkParams &rhs)
{
    return !(lhs == rhs);
}

Q_DECLARE_METATYPE(LinkKind)
Q_DECLARE_METATYPE(LinkParams)

/**
 * @brief QLinkManager - 链路管理器
 */
class MINIGCS_EXPORT QLinkManager : public QObject
{
    Q_OBJECT

public:
    explicit QLinkManager(QGroundControlStation *groundStation, QObject *parent = nullptr);
    ~QLinkManager();

    Q_INVOKABLE QDataLink *addLink(LinkKind type, const LinkParams &params);
    Q_INVOKABLE void removeLink(LinkKind type, const LinkParams &params);
    Q_INVOKABLE void removeLink(QDataLink *link);
    Q_INVOKABLE void clearAll();

signals:
    void linkCreateFailed(const QString &reason);
    void linkConnectionError(QDataLink *link, const QString &reason);
    void linkReconnected(QDataLink *link);
    void linkReconnectFailed(QDataLink *link, const QString &reason);

private:
    friend class QGroundControlStationPrivate;
    friend class QLinkManagerPrivate;
    void handleConnectionError(const QString &connectionString,
                               const QString &reason);

    QLinkManagerPrivate *d_func() { return d_ptr.get(); }
    const QLinkManagerPrivate *d_func() const { return d_ptr.get(); }
    std::unique_ptr<QLinkManagerPrivate> d_ptr;
};

#endif // QLINKMANAGER_H
