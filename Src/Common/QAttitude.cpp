#include <QtGlobal>
#include "Common/QAttitude.h"

QAttitude::QAttitude(double rollDeg, double pitchDeg, double yawDeg,
                     double headingDeg)
    : m_rollDeg(rollDeg)
    , m_pitchDeg(pitchDeg)
    , m_yawDeg(yawDeg)
    , m_headingDeg(headingDeg)
{
}

void QAttitude::setRollDeg(double rollDeg)
{
    m_rollDeg = rollDeg;
}

void QAttitude::setPitchDeg(double pitchDeg)
{
    m_pitchDeg = pitchDeg;
}

void QAttitude::setYawDeg(double yawDeg)
{
    m_yawDeg = yawDeg;
}

void QAttitude::setHeadingDeg(double headingDeg)
{
    m_headingDeg = headingDeg;
}

bool QAttitude::operator==(const QAttitude &other) const
{
    return qFuzzyCompare(m_rollDeg, other.m_rollDeg) &&
           qFuzzyCompare(m_pitchDeg, other.m_pitchDeg) &&
           qFuzzyCompare(m_yawDeg, other.m_yawDeg) &&
           qFuzzyCompare(m_headingDeg, other.m_headingDeg);
}

bool QAttitude::operator!=(const QAttitude &other) const
{
    return !(*this == other);
}
