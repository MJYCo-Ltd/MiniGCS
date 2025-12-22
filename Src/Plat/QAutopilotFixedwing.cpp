#include "Plat/QAutopilotFixedwing.h"

#include <QtGlobal>
#include <cmath>

QAutopilotFixedwing::QAutopilotFixedwing() = default;

void QAutopilotFixedwing::setAirspeedMS(float airspeed)
{
    m_airspeed_m_s = airspeed;
}

void QAutopilotFixedwing::setThrottlePercentage(float throttle)
{
    m_throttle_percentage = throttle;
}

void QAutopilotFixedwing::setClimbRateMS(float climbRate)
{
    m_climb_rate_m_s = climbRate;
}

void QAutopilotFixedwing::setGroundspeedMS(float groundspeed)
{
    m_groundspeed_m_s = groundspeed;
}

void QAutopilotFixedwing::setHeadingDeg(float heading)
{
    m_heading_deg = heading;
}

void QAutopilotFixedwing::setAbsoluteAltitudeM(float altitude)
{
    m_absolute_altitude_m = altitude;
}

bool QAutopilotFixedwing::operator==(const QAutopilotFixedwing &other) const
{
    auto eq = [](float a, float b) {
        if (std::isnan(a) && std::isnan(b)) {
            return true;
        }
        return qFuzzyCompare(a, b);
    };

    return eq(m_airspeed_m_s, other.m_airspeed_m_s) &&
           eq(m_throttle_percentage, other.m_throttle_percentage) &&
           eq(m_climb_rate_m_s, other.m_climb_rate_m_s) &&
           eq(m_groundspeed_m_s, other.m_groundspeed_m_s) &&
           eq(m_heading_deg, other.m_heading_deg) &&
           eq(m_absolute_altitude_m, other.m_absolute_altitude_m);
}

bool QAutopilotFixedwing::operator!=(const QAutopilotFixedwing &other) const
{
    return !(*this == other);
}


