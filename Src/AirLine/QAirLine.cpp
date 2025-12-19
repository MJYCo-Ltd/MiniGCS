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
    return m_waypoints;
}

void QAirLine::setWaypoints(const QList<QGpsPosition> &waypoints)
{
    if (m_waypoints != waypoints) {
        m_waypoints = waypoints;
        emit waypointsChanged();
    }
}

int QAirLine::waypointCount() const
{
    return m_waypoints.size();
}

void QAirLine::addWaypoint(const QGpsPosition &position)
{
    m_waypoints.append(position);
    emit waypointsChanged();
}

void QAirLine::insertWaypoint(int index, const QGpsPosition &position)
{
    if (index < 0 || index > m_waypoints.size()) {
        qWarning() << "QAirLine::insertWaypoint: 索引超出范围" << index;
        return;
    }
    m_waypoints.insert(index, position);
    emit waypointsChanged();
}

void QAirLine::removeWaypoint(int index)
{
    if (index < 0 || index >= m_waypoints.size()) {
        qWarning() << "QAirLine::removeWaypoint: 索引超出范围" << index;
        return;
    }
    m_waypoints.removeAt(index);
    emit waypointsChanged();
}

QGpsPosition QAirLine::getWaypoint(int index) const
{
    if (index < 0 || index >= m_waypoints.size()) {
        qWarning() << "QAirLine::getWaypoint: 索引超出范围" << index;
        return QGpsPosition();
    }
    return m_waypoints.at(index);
}

void QAirLine::clearWaypoints()
{
    if (!m_waypoints.isEmpty()) {
        m_waypoints.clear();
        emit waypointsChanged();
    }
}

