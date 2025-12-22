#ifndef _YTY_QAUTOPILOTFIXEDWING_H
#define _YTY_QAUTOPILOTFIXEDWING_H

#include <QObject>
#include <QMetaType>
#include "MiniGCSExport.h"

/**
 * @brief 固定翼飞行状态信息
 *
 * 参考 QAutopilotStatus 设计，用于描述固定翼特有的飞行参数
 */
class MINIGCS_EXPORT QAutopilotFixedwing
{
    Q_GADGET
    Q_PROPERTY(float airspeedMS READ airspeedMS WRITE setAirspeedMS)
    Q_PROPERTY(float throttlePercentage READ throttlePercentage WRITE setThrottlePercentage)
    Q_PROPERTY(float climbRateMS READ climbRateMS WRITE setClimbRateMS)
    Q_PROPERTY(float groundspeedMS READ groundspeedMS WRITE setGroundspeedMS)
    Q_PROPERTY(float headingDeg READ headingDeg WRITE setHeadingDeg)
    Q_PROPERTY(float absoluteAltitudeM READ absoluteAltitudeM WRITE setAbsoluteAltitudeM)

public:
    QAutopilotFixedwing();

    float airspeedMS() const { return m_airspeed_m_s; }
    void setAirspeedMS(float airspeed);

    float throttlePercentage() const { return m_throttle_percentage; }
    void setThrottlePercentage(float throttle);

    float climbRateMS() const { return m_climb_rate_m_s; }
    void setClimbRateMS(float climbRate);

    float groundspeedMS() const { return m_groundspeed_m_s; }
    void setGroundspeedMS(float groundspeed);

    float headingDeg() const { return m_heading_deg; }
    void setHeadingDeg(float heading);

    float absoluteAltitudeM() const { return m_absolute_altitude_m; }
    void setAbsoluteAltitudeM(float altitude);

    bool operator==(const QAutopilotFixedwing &other) const;
    bool operator!=(const QAutopilotFixedwing &other) const;

private:
    /// 当前指示空速 IAS，单位 m/s
    float m_airspeed_m_s{float(NAN)};
    /// 当前油门开度，0-100
    float m_throttle_percentage{float(NAN)};
    /// 当前爬升率，m/s
    float m_climb_rate_m_s{float(NAN)};
    /// 当前地速，m/s
    float m_groundspeed_m_s{float(NAN)};
    /// 当前航向角，0-360 度，0 为正北
    float m_heading_deg{float(NAN)};
    /// 当前绝对高度（MSL），单位 m
    float m_absolute_altitude_m{float(NAN)};
};

Q_DECLARE_METATYPE(QAutopilotFixedwing)

#endif // _YTY_QAUTOPILOTFIXEDWING_H


