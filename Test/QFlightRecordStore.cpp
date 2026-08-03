#include "QFlightRecordStore.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>
#include <QtMath>
#include <cmath>

namespace {
constexpr double EARTH_RADIUS_M = 6371000.0;
}

QFlightRecordStore::QFlightRecordStore(
    qint64 minimumSampleIntervalMs, double minimumSampleDistanceM,
    int maximumRecordCount, QObject *parent)
    : QObject(parent)
    , m_minimumSampleIntervalMs(qMax<qint64>(100, minimumSampleIntervalMs))
    , m_minimumSampleDistanceM(qMax(0.0, minimumSampleDistanceM))
    , m_maximumRecordCount(qMax(1, maximumRecordCount))
{
    load();
}

void QFlightRecordStore::start(int systemId, const QString &droneName,
                               const QVariantList &missionPoints)
{
    ActiveRecord active;
    active.value.insert(QStringLiteral("id"),
                        QUuid::createUuid().toString(QUuid::WithoutBraces));
    active.value.insert(QStringLiteral("systemId"), systemId);
    active.value.insert(QStringLiteral("droneName"), droneName);
    active.value.insert(QStringLiteral("startedAt"),
                        QDateTime::currentDateTime().toString(Qt::ISODate));
    active.value.insert(QStringLiteral("completedAt"), QString());
    active.value.insert(QStringLiteral("success"), false);
    active.value.insert(QStringLiteral("missionPoints"), missionPoints);
    m_activeRecords.insert(systemId, active);
}

void QFlightRecordStore::appendPosition(
    int systemId, const QGpsPosition &position)
{
    auto iterator = m_activeRecords.find(systemId);
    if (iterator == m_activeRecords.end())
        return;

    const double latitude = position.latitude();
    const double longitude = position.longitude();
    const double altitude = position.altitude();
    if (!std::isfinite(latitude) || !std::isfinite(longitude) ||
        !std::isfinite(altitude)) {
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (iterator->hasLastPosition) {
        const qint64 elapsed = now - iterator->lastSampleMs;
        const double distance = distanceM(
            iterator->lastLatitude, iterator->lastLongitude,
            latitude, longitude);
        if (elapsed < m_minimumSampleIntervalMs &&
            distance < m_minimumSampleDistanceM) {
            return;
        }
    }

    QVariantMap sample;
    sample.insert(QStringLiteral("timestamp"),
                  QDateTime::fromMSecsSinceEpoch(now).toString(Qt::ISODate));
    sample.insert(QStringLiteral("latitude"), latitude);
    sample.insert(QStringLiteral("longitude"), longitude);
    sample.insert(QStringLiteral("altitude"), altitude);
    iterator->track.append(sample);
    iterator->lastSampleMs = now;
    iterator->lastLatitude = latitude;
    iterator->lastLongitude = longitude;
    iterator->hasLastPosition = true;
}

void QFlightRecordStore::complete(int systemId)
{
    auto iterator = m_activeRecords.find(systemId);
    if (iterator == m_activeRecords.end())
        return;
    QVariantMap record = iterator->value;
    record.insert(QStringLiteral("track"), iterator->track);
    record.insert(QStringLiteral("completedAt"),
                  QDateTime::currentDateTime().toString(Qt::ISODate));
    record.insert(QStringLiteral("success"), true);
    m_activeRecords.erase(iterator);
    m_records.prepend(record);
    while (m_records.size() > m_maximumRecordCount)
        m_records.removeLast();
    save();
    emit recordsChanged();
}

void QFlightRecordStore::cancel(int systemId)
{
    m_activeRecords.remove(systemId);
}

void QFlightRecordStore::clear()
{
    if (m_records.isEmpty())
        return;
    m_records.clear();
    save();
    emit recordsChanged();
}

QString QFlightRecordStore::storagePath() const
{
    const QString directory = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    QDir().mkpath(directory);
    return QDir(directory).filePath(QStringLiteral("flight_records.json"));
}

void QFlightRecordStore::load()
{
    QFile file(storagePath());
    if (!file.open(QIODevice::ReadOnly))
        return;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isArray())
        return;
    m_records = document.array().toVariantList();
    while (m_records.size() > m_maximumRecordCount)
        m_records.removeLast();
}

void QFlightRecordStore::save() const
{
    QSaveFile file(storagePath());
    if (!file.open(QIODevice::WriteOnly))
        return;
    file.write(QJsonDocument::fromVariant(m_records).toJson(
        QJsonDocument::Compact));
    file.commit();
}

double QFlightRecordStore::distanceM(
    double latitude1, double longitude1,
    double latitude2, double longitude2)
{
    const double lat1 = qDegreesToRadians(latitude1);
    const double lat2 = qDegreesToRadians(latitude2);
    const double deltaLat = qDegreesToRadians(latitude2 - latitude1);
    const double deltaLon = qDegreesToRadians(longitude2 - longitude1);
    const double value = std::sin(deltaLat / 2.0) * std::sin(deltaLat / 2.0) +
        std::cos(lat1) * std::cos(lat2) *
        std::sin(deltaLon / 2.0) * std::sin(deltaLon / 2.0);
    const double a = qBound(0.0, value, 1.0);
    return EARTH_RADIUS_M * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
}
