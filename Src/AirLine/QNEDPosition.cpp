#include <QtGlobal>
#include "AirLine/QNEDPosition.h"

// QNEDPosition 实现
QNEDPosition::QNEDPosition()
    : m_north(0.0), m_east(0.0), m_down(0.0)
{
}

QNEDPosition::QNEDPosition(float north, float east, float down)
    : m_north(north), m_east(east), m_down(down)
{
}

void QNEDPosition::setNorth(float north)
{
    m_north = north;
}

void QNEDPosition::setEast(float east)
{
    m_east = east;
}

void QNEDPosition::setDown(float down)
{
    m_down = down;
}

bool QNEDPosition::operator==(const QNEDPosition &other) const
{
    return qFuzzyCompare(m_north, other.m_north) &&
           qFuzzyCompare(m_east, other.m_east) &&
           qFuzzyCompare(m_down, other.m_down);
}

bool QNEDPosition::operator!=(const QNEDPosition &other) const
{
    return !(*this == other);
}

