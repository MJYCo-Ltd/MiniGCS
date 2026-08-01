#ifndef _YTY_QRAWGPS_H
#define _YTY_QRAWGPS_H

#include <QObject>
#include <QMetaType>
#include "MiniGCSExport.h"

/**
 * @brief 原始 GPS 质量与速度相关量
 */
class MINIGCS_EXPORT QRawGps
{
    Q_GADGET
    Q_PROPERTY(double hdop READ hdop WRITE setHdop)
    Q_PROPERTY(double vdop READ vdop WRITE setVdop)
    Q_PROPERTY(double velocityMS READ velocityMS WRITE setVelocityMS)
    Q_PROPERTY(double courseDeg READ courseDeg WRITE setCourseDeg)
    Q_PROPERTY(double horizontalUncertaintyM READ horizontalUncertaintyM WRITE setHorizontalUncertaintyM)
    Q_PROPERTY(double verticalUncertaintyM READ verticalUncertaintyM WRITE setVerticalUncertaintyM)
    Q_PROPERTY(double velocityUncertaintyMS READ velocityUncertaintyMS WRITE setVelocityUncertaintyMS)
    Q_PROPERTY(double headingUncertaintyDeg READ headingUncertaintyDeg WRITE setHeadingUncertaintyDeg)

public:
    QRawGps() = default;

    double hdop() const { return m_hdop; }
    void setHdop(double hdop);

    double vdop() const { return m_vdop; }
    void setVdop(double vdop);

    double velocityMS() const { return m_velocityMS; }
    void setVelocityMS(double velocityMS);

    double courseDeg() const { return m_courseDeg; }
    void setCourseDeg(double courseDeg);

    double horizontalUncertaintyM() const { return m_horizontalUncertaintyM; }
    void setHorizontalUncertaintyM(double meters);

    double verticalUncertaintyM() const { return m_verticalUncertaintyM; }
    void setVerticalUncertaintyM(double meters);

    double velocityUncertaintyMS() const { return m_velocityUncertaintyMS; }
    void setVelocityUncertaintyMS(double metersPerSecond);

    double headingUncertaintyDeg() const { return m_headingUncertaintyDeg; }
    void setHeadingUncertaintyDeg(double degrees);

    bool operator==(const QRawGps &other) const;
    bool operator!=(const QRawGps &other) const;

private:
    double m_hdop{0.0};
    double m_vdop{0.0};
    double m_velocityMS{0.0};
    double m_courseDeg{0.0};
    double m_horizontalUncertaintyM{0.0};
    double m_verticalUncertaintyM{0.0};
    double m_velocityUncertaintyMS{0.0};
    double m_headingUncertaintyDeg{0.0};
};

Q_DECLARE_METATYPE(QRawGps)

#endif // _YTY_QRAWGPS_H
