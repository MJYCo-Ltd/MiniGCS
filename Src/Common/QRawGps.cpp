#include <QtGlobal>
#include "Common/QRawGps.h"

void QRawGps::setHdop(double hdop) { m_hdop = hdop; }
void QRawGps::setVdop(double vdop) { m_vdop = vdop; }
void QRawGps::setVelocityMS(double velocityMS) { m_velocityMS = velocityMS; }
void QRawGps::setCourseDeg(double courseDeg) { m_courseDeg = courseDeg; }
void QRawGps::setHorizontalUncertaintyM(double meters)
{
    m_horizontalUncertaintyM = meters;
}
void QRawGps::setVerticalUncertaintyM(double meters)
{
    m_verticalUncertaintyM = meters;
}
void QRawGps::setVelocityUncertaintyMS(double metersPerSecond)
{
    m_velocityUncertaintyMS = metersPerSecond;
}
void QRawGps::setHeadingUncertaintyDeg(double degrees)
{
    m_headingUncertaintyDeg = degrees;
}

bool QRawGps::operator==(const QRawGps &other) const
{
    return qFuzzyCompare(m_hdop, other.m_hdop) &&
           qFuzzyCompare(m_vdop, other.m_vdop) &&
           qFuzzyCompare(m_velocityMS, other.m_velocityMS) &&
           qFuzzyCompare(m_courseDeg, other.m_courseDeg) &&
           qFuzzyCompare(m_horizontalUncertaintyM,
                         other.m_horizontalUncertaintyM) &&
           qFuzzyCompare(m_verticalUncertaintyM,
                         other.m_verticalUncertaintyM) &&
           qFuzzyCompare(m_velocityUncertaintyMS,
                         other.m_velocityUncertaintyMS) &&
           qFuzzyCompare(m_headingUncertaintyDeg,
                         other.m_headingUncertaintyDeg);
}

bool QRawGps::operator!=(const QRawGps &other) const
{
    return !(*this == other);
}
