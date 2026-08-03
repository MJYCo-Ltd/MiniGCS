#include <QDebug>
#include "AirLine/QAirLine.h"

QAirLine::QAirLine(QObject *parent)
    : QObject(parent)
    , m_name("未命名航线")
{
}

QAirLine::QAirLine(const QString &name, QObject *parent)
    : QObject(parent)
    , m_name(name)
{
}

QAirLine::~QAirLine()
{
}

QString QAirLine::name() const
{
    return m_name;
}

void QAirLine::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged(m_name);
    }
}

QList<QGpsPosition> QAirLine::waypoints() const
{
    QList<QGpsPosition> result;
    result.reserve(m_missionPoints.size());
    for (const QMissionPoint &point : m_missionPoints) {
        result.append(point.position());
    }
    return result;
}

void QAirLine::setWaypoints(const QList<QGpsPosition> &waypoints)
{
    QList<QMissionPoint> points;
    points.reserve(waypoints.size());
    for (const QGpsPosition &position : waypoints) {
        points.append(QMissionPoint(position));
    }
    setMissionPoints(points);
}

QList<QMissionPoint> QAirLine::missionPoints() const
{
    return m_missionPoints;
}

void QAirLine::setMissionPoints(const QList<QMissionPoint> &points)
{
    if (m_missionPoints != points) {
        m_missionPoints = points;
        emit waypointsChanged();
    }
}

int QAirLine::waypointCount() const
{
    return m_missionPoints.size();
}

void QAirLine::addWaypoint(const QGpsPosition &position)
{
    m_missionPoints.append(QMissionPoint(position));
    emit waypointsChanged();
}

void QAirLine::insertWaypoint(int index, const QGpsPosition &position)
{
    if (index < 0 || index > m_missionPoints.size()) {
        qWarning() << "QAirLine::insertWaypoint: 索引超出范围" << index;
        return;
    }
    m_missionPoints.insert(index, QMissionPoint(position));
    emit waypointsChanged();
}

void QAirLine::removeWaypoint(int index)
{
    if (index < 0 || index >= m_missionPoints.size()) {
        qWarning() << "QAirLine::removeWaypoint: 索引超出范围" << index;
        return;
    }
    m_missionPoints.removeAt(index);
    emit waypointsChanged();
}

QGpsPosition QAirLine::getWaypoint(int index) const
{
    if (index < 0 || index >= m_missionPoints.size()) {
        qWarning() << "QAirLine::getWaypoint: 索引超出范围" << index;
        return QGpsPosition();
    }
    return m_missionPoints.at(index).position();
}

void QAirLine::clearWaypoints()
{
    if (!m_missionPoints.isEmpty()) {
        m_missionPoints.clear();
        emit waypointsChanged();
    }
}

