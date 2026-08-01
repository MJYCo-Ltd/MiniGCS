#include <QtGlobal>
#include <cmath>
#include "Common/QVelocity.h"

QVelocity::QVelocity(double northMS, double eastMS, double downMS)
    : m_northMS(northMS)
    , m_eastMS(eastMS)
    , m_downMS(downMS)
{
    refreshScalars();
}

void QVelocity::setNorthMS(double northMS)
{
    m_northMS = northMS;
}

void QVelocity::setEastMS(double eastMS)
{
    m_eastMS = eastMS;
}

void QVelocity::setDownMS(double downMS)
{
    m_downMS = downMS;
}

void QVelocity::setGroundSpeedMS(double groundSpeedMS)
{
    m_groundSpeedMS = groundSpeedMS;
}

void QVelocity::setVerticalSpeedMS(double verticalSpeedMS)
{
    m_verticalSpeedMS = verticalSpeedMS;
}

void QVelocity::refreshScalars()
{
    m_groundSpeedMS = std::hypot(m_northMS, m_eastMS);
    m_verticalSpeedMS = std::abs(m_downMS);
}

bool QVelocity::operator==(const QVelocity &other) const
{
    return qFuzzyCompare(m_northMS, other.m_northMS) &&
           qFuzzyCompare(m_eastMS, other.m_eastMS) &&
           qFuzzyCompare(m_downMS, other.m_downMS) &&
           qFuzzyCompare(m_groundSpeedMS, other.m_groundSpeedMS) &&
           qFuzzyCompare(m_verticalSpeedMS, other.m_verticalSpeedMS);
}

bool QVelocity::operator!=(const QVelocity &other) const
{
    return !(*this == other);
}
