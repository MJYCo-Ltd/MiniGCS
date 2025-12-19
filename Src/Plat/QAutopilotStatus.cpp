#include "Plat/QAutopilotStatus.h"
#include <QtGlobal>

// QAutopilotStatus 实现
QAutopilotStatus::QAutopilotStatus()
    : m_gpsCount(0), m_gpsStatus("未知"), m_batteryVoltage(0.0f),
      m_batteryRemaining(0.0f)
{
}

QAutopilotStatus::QAutopilotStatus(int gpsCount, const QString &gpsStatus, float batteryVoltage,
                                   float batteryRemaining, bool rcConnected)
    : m_gpsCount(gpsCount), m_gpsStatus(gpsStatus), m_batteryVoltage(batteryVoltage),
      m_batteryRemaining(batteryRemaining)
{
}

void QAutopilotStatus::setGpsCount(int gpsCount)
{
    m_gpsCount = gpsCount;
}

void QAutopilotStatus::setGpsStatus(const QString &gpsStatus)
{
    m_gpsStatus = gpsStatus;
}

void QAutopilotStatus::setBatteryVoltage(float batteryVoltage)
{
    m_batteryVoltage = batteryVoltage;
}

void QAutopilotStatus::setBatteryRemaining(float batteryRemaining)
{
    m_batteryRemaining = batteryRemaining;
}

void QAutopilotStatus::setGyrometerCalibrationOk(bool isOk)
{
    m_isGyrometerCalibrationOk = isOk;
}

void QAutopilotStatus::setAccelerometerCalibrationOk(bool isOk)
{
    m_isAccelerometerCalibrationOk = isOk;
}

void QAutopilotStatus::setMagnetometerCalibrationOk(bool isOk)
{
    m_isMagnetometerCalibrationOk = isOk;
}

void QAutopilotStatus::setLocalPositionOk(bool isOk)
{
    m_isLocalPositionOk = isOk;
}

void QAutopilotStatus::setGlobalPositionOk(bool isOk)
{
    m_isGlobalPositionOk = isOk;
}

void QAutopilotStatus::setHomePositionOk(bool isOk)
{
    m_isHomePositionOk = isOk;
}

void QAutopilotStatus::setArmable(bool isArmable)
{
    m_isArmable = isArmable;
}

void QAutopilotStatus::setRcIsAvailable(bool isAvailable)
{
    m_rcIsAvailable = isAvailable;
}

void QAutopilotStatus::setRcSignalStrengthPercent(float signalStrengthPercent)
{
    m_rcSignalStrengthPercent = signalStrengthPercent;
}

bool QAutopilotStatus::operator==(const QAutopilotStatus &other) const
{
    return m_gpsCount == other.m_gpsCount &&
           m_gpsStatus == other.m_gpsStatus &&
           qFuzzyCompare(m_batteryVoltage, other.m_batteryVoltage) &&
           qFuzzyCompare(m_batteryRemaining, other.m_batteryRemaining) &&
           m_isGyrometerCalibrationOk == other.m_isGyrometerCalibrationOk &&
           m_isAccelerometerCalibrationOk == other.m_isAccelerometerCalibrationOk &&
           m_isMagnetometerCalibrationOk == other.m_isMagnetometerCalibrationOk &&
           m_isLocalPositionOk == other.m_isLocalPositionOk &&
           m_isGlobalPositionOk == other.m_isGlobalPositionOk &&
           m_isHomePositionOk == other.m_isHomePositionOk &&
           m_isArmable == other.m_isArmable &&
           m_rcIsAvailable == other.m_rcIsAvailable &&
           qFuzzyCompare(m_rcSignalStrengthPercent, other.m_rcSignalStrengthPercent);
}

bool QAutopilotStatus::operator!=(const QAutopilotStatus &other) const
{
    return !(*this == other);
}

