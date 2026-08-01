#ifndef _YTY_QATTITUDE_H
#define _YTY_QATTITUDE_H

#include <QObject>
#include <QMetaType>
#include "MiniGCSExport.h"

/**
 * @brief 姿态与航向
 *
 * roll/pitch/yaw 为机体欧拉角（度）；headingDeg 为航向（度，0 为正北）。
 */
class MINIGCS_EXPORT QAttitude
{
    Q_GADGET
    Q_PROPERTY(double rollDeg READ rollDeg WRITE setRollDeg)
    Q_PROPERTY(double pitchDeg READ pitchDeg WRITE setPitchDeg)
    Q_PROPERTY(double yawDeg READ yawDeg WRITE setYawDeg)
    Q_PROPERTY(double headingDeg READ headingDeg WRITE setHeadingDeg)

public:
    QAttitude() = default;
    QAttitude(double rollDeg, double pitchDeg, double yawDeg,
              double headingDeg = 0.0);

    double rollDeg() const { return m_rollDeg; }
    void setRollDeg(double rollDeg);

    double pitchDeg() const { return m_pitchDeg; }
    void setPitchDeg(double pitchDeg);

    double yawDeg() const { return m_yawDeg; }
    void setYawDeg(double yawDeg);

    double headingDeg() const { return m_headingDeg; }
    void setHeadingDeg(double headingDeg);

    bool operator==(const QAttitude &other) const;
    bool operator!=(const QAttitude &other) const;

private:
    double m_rollDeg{0.0};
    double m_pitchDeg{0.0};
    double m_yawDeg{0.0};
    double m_headingDeg{0.0};
};

Q_DECLARE_METATYPE(QAttitude)

#endif // _YTY_QATTITUDE_H
