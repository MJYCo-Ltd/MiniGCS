#include <QtGlobal>
#include "AirLine/QGpsPosition.h"

// QGpsPosition 实现
QGpsPosition::QGpsPosition()
    : m_longitude(0.0), m_latitude(0.0), m_altitude(0.0)
{
}

QGpsPosition::QGpsPosition(double longitude, double latitude, double altitude)
    : m_longitude(longitude), m_latitude(latitude), m_altitude(altitude)
{
}

void QGpsPosition::setLongitude(double longitude)
{
    m_longitude = longitude;
}

void QGpsPosition::setLatitude(double latitude)
{
    m_latitude = latitude;
}

void QGpsPosition::setAltitude(float altitude)
{
    m_altitude = altitude;
}

bool QGpsPosition::operator==(const QGpsPosition &other) const
{
    return qFuzzyCompare(m_longitude, other.m_longitude) &&
           qFuzzyCompare(m_latitude, other.m_latitude) &&
           qFuzzyCompare(m_altitude, other.m_altitude);
}

bool QGpsPosition::operator!=(const QGpsPosition &other) const
{
    return !(*this == other);
}

