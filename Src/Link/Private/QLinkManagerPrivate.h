#ifndef QLINKMANAGERPRIVATE_H
#define QLINKMANAGERPRIVATE_H

#include "Link/QLinkManager.h"
#include <QString>
#include <QMap>

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
    explicit QLinkManagerPrivate(QGroundControlStation *groundStation);
    ~QLinkManagerPrivate();

    QGroundControlStation *groundStation() const { return m_groundStation; }

    static QString buildConnectionString(LinkKind type, const LinkParams &params);

    bool hasConnection(const QString &connStr) const;
    QDataLink *addConnection(LinkKind type, const QString &connStr);
    void removeConnection(const QString &connStr);
    void removeLink(QDataLink *link);
    QStringList connectionStrings() const;

private:
    QGroundControlStation *m_groundStation{nullptr};
    QMap<QString, QDataLink *> m_connections;  ///< connStr -> QDataLink
};

#endif // QLINKMANAGERPRIVATE_H
