#include "Private/QMavsdkTextCatalog.h"

#include "QGCSConfig.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

namespace {
struct CatalogCache
{
    QMutex mutex;
    QString path;
    QDateTime lastModified;
    qint64 size{-1};
    QJsonObject root;
};

CatalogCache &catalogCache()
{
    static CatalogCache cache;
    return cache;
}

void reloadIfChanged(CatalogCache &cache, const QString &path)
{
    const QFileInfo info(path);
    if (cache.path == info.absoluteFilePath() &&
        cache.lastModified == info.lastModified() &&
        cache.size == info.size()) {
        return;
    }

    cache.path = info.absoluteFilePath();
    cache.lastModified = info.lastModified();
    cache.size = info.size();
    cache.root = {};

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法读取 MAVSDK 类型文本文件:" << path;
        return;
    }

    QJsonParseError error;
    const QJsonDocument document =
        QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        qWarning() << "MAVSDK 类型文本文件格式错误:" << path
                   << error.errorString();
        return;
    }
    cache.root = document.object();
}
} // namespace

QString QMavsdkTextCatalog::text(QStringView section, int value)
{
    return text(section, QString::number(value));
}

QString QMavsdkTextCatalog::text(QStringView section, QStringView key)
{
    CatalogCache &cache = catalogCache();
    QMutexLocker locker(&cache.mutex);
    reloadIfChanged(cache, QGCSConfig::instance()->typeTextFile());

    const QString sectionName = section.toString();
    const QJsonObject entries = cache.root.value(sectionName).toObject();
    const QJsonValue exact = entries.value(key.toString());
    if (exact.isString()) {
        return exact.toString();
    }

    const QJsonValue fallback = entries.value(QStringLiteral("default"));
    if (fallback.isString()) {
        return fallback.toString();
    }
    return QStringLiteral("%1(%2)").arg(sectionName, key.toString());
}
