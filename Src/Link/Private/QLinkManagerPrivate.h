#ifndef QLINKMANAGERPRIVATE_H
#define QLINKMANAGERPRIVATE_H

#include "Link/QLinkManager.h"
#include <QString>
#include <QMap>
#include <QPointer>
#include <QSet>

class QGroundControlStation;
class QDataLink;

/**
 * @brief QLinkManagerPrivate - 链路管理器私有实现
 *
 * 根据 LinkKind 和 LinkParams 生成连接字符串，创建 QDataLink，通过地面站添加/移除连接。
 */
class QLinkManagerPrivate
{
public:
    explicit QLinkManagerPrivate(QLinkManager *owner,
                                 QGroundControlStation *groundStation);
    ~QLinkManagerPrivate();

    static QString buildConnectionString(LinkKind type, const LinkParams &params);

    bool hasConnection(const QString &connStr) const;
    QDataLink *addConnection(LinkKind type, const QString &connStr);
    void removeConnection(const QString &connStr);
    void removeLink(QDataLink *link);
    QStringList connectionStrings() const;
    void handleConnectionError(const QString &connStr, const QString &reason);

private:
    bool openConnection(QDataLink *link);
    void scheduleReconnect(const QString &connStr, const QString &lastError,
                           quint64 generation);
    void invalidateReconnect(const QString &connStr);

    QPointer<QLinkManager> m_owner;
    QPointer<QGroundControlStation> m_groundStation;
    QMap<QString, QPointer<QDataLink>> m_connections;  ///< connStr -> QDataLink
    QMap<QString, quint64> m_reconnectGenerations;
    QSet<QString> m_pendingReconnects;
};

#endif // QLINKMANAGERPRIVATE_H
