#include "Link/QLinkManager.h"
#include "Link/Private/QLinkManagerPrivate.h"
#include "Link/QDataLink.h"
#include "QGroundControlStation.h"
#include <QDebug>

QString QLinkManager::buildConnectionString(LinkKind type, const LinkParams &params)
{
    return QLinkManagerPrivate::buildConnectionString(type, params);
}

QLinkManager::QLinkManager(QGroundControlStation *groundStation, QObject *parent)
    : QObject(parent)
    , d_ptr(new QLinkManagerPrivate(groundStation))
{
}

QLinkManager::~QLinkManager() = default;

QDataLink *QLinkManager::addLink(LinkKind type, const LinkParams &params)
{
    QString connStr = buildConnectionString(type, params);
    if (connStr.isEmpty()) {
        emit linkCreateFailed("无效的链路参数");
        return nullptr;
    }
    QLinkManagerPrivate *const d = d_func();
    if (d->hasConnection(connStr)) {
        emit linkCreateFailed(QString("连接已存在: %1").arg(connStr));
        return nullptr;
    }
    QDataLink *link = d->addConnection(type, connStr);
    if (!link) {
        emit linkCreateFailed("添加链路失败");
    }
    return link;
}

void QLinkManager::removeLink(LinkKind type, const LinkParams &params)
{
    QString connStr = buildConnectionString(type, params);
    if (!connStr.isEmpty()) {
        d_func()->removeConnection(connStr);
    }
}

void QLinkManager::removeLink(QDataLink *link)
{
    if (link) {
        d_func()->removeLink(link);
    }
}

void QLinkManager::clearAll()
{
    QLinkManagerPrivate *const d = d_func();
    QStringList connStrs = d->connectionStrings();
    for (const QString &connStr : connStrs) {
        d->removeConnection(connStr);
    }
}
