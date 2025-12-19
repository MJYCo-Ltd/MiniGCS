#include <QtGlobal>
#include "AirLine/QDronePosition.h"

// QDronePosition 实现
QDronePosition::QDronePosition()
    : m_longitude(0.0), m_latitude(0.0), m_altitude(0.0),
      m_nedX(0.0), m_nedY(0.0), m_nedZ(0.0)
{
}

QDronePosition::QDronePosition(double longitude, double latitude, double altitude,
                                double nedX, double nedY, double nedZ)
    : m_longitude(longitude), m_latitude(latitude), m_altitude(altitude),
      m_nedX(nedX), m_nedY(nedY), m_nedZ(nedZ)
{
}

void QDronePosition::setLongitude(double longitude)
{
    m_longitude = longitude;
}

void QDronePosition::setLatitude(double latitude)
{
    m_latitude = latitude;
}

void QDronePosition::setAltitude(double altitude)
{
    m_altitude = altitude;
}

void QDronePosition::setNedX(double nedX)
{
    m_nedX = nedX;
}

void QDronePosition::setNedY(double nedY)
{
    m_nedY = nedY;
}

void QDronePosition::setNedZ(double nedZ)
{
    m_nedZ = nedZ;
}

bool QDronePosition::operator==(const QDronePosition &other) const
{
    return qFuzzyCompare(m_longitude, other.m_longitude) &&
           qFuzzyCompare(m_latitude, other.m_latitude) &&
           qFuzzyCompare(m_altitude, other.m_altitude) &&
           qFuzzyCompare(m_nedX, other.m_nedX) &&
           qFuzzyCompare(m_nedY, other.m_nedY) &&
           qFuzzyCompare(m_nedZ, other.m_nedZ);
}

bool QDronePosition::operator!=(const QDronePosition &other) const
{
    return !(*this == other);
}

