#ifndef QFLIGHTRECORDSTORE_H
#define QFLIGHTRECORDSTORE_H

#include "Common/QGpsPosition.h"

#include <QHash>
#include <QObject>
#include <QVariantList>

class QFlightRecordStore : public QObject
{
    Q_OBJECT

public:
    explicit QFlightRecordStore(qint64 minimumSampleIntervalMs,
                                double minimumSampleDistanceM,
                                int maximumRecordCount,
                                QObject *parent = nullptr);

    QVariantList records() const { return m_records; }
    void start(int systemId, const QString &droneName,
               const QVariantList &missionPoints);
    void appendPosition(int systemId, const QGpsPosition &position);
    void complete(int systemId);
    void cancel(int systemId);
    void clear();

signals:
    void recordsChanged();

private:
    struct ActiveRecord {
        QVariantMap value;
        QVariantList track;
        qint64 lastSampleMs{0};
        double lastLatitude{0.0};
        double lastLongitude{0.0};
        bool hasLastPosition{false};
    };

    QString storagePath() const;
    void load();
    void save() const;
    static double distanceM(double latitude1, double longitude1,
                            double latitude2, double longitude2);

    qint64 m_minimumSampleIntervalMs;
    double m_minimumSampleDistanceM;
    int m_maximumRecordCount;
    QVariantList m_records;
    QHash<int, ActiveRecord> m_activeRecords;
};

#endif // QFLIGHTRECORDSTORE_H
