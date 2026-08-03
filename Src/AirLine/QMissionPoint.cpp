#include "AirLine/QMissionPoint.h"

#include <QtGlobal>

QMissionPoint::QMissionPoint(const QGpsPosition &position,
                             Action action,
                             double actionDurationS,
                             double speedMS,
                             bool flyThrough)
    : m_position(position)
    , m_action(action)
    , m_actionDurationS(actionDurationS)
    , m_speedMS(speedMS)
    , m_flyThrough(flyThrough)
{
}

bool QMissionPoint::operator==(const QMissionPoint &other) const
{
    return m_position == other.m_position &&
           m_action == other.m_action &&
           qFuzzyCompare(m_actionDurationS, other.m_actionDurationS) &&
           qFuzzyCompare(m_speedMS, other.m_speedMS) &&
           m_flyThrough == other.m_flyThrough;
}
