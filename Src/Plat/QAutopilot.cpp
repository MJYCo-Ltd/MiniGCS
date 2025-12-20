#include "Plat/QAutopilot.h"
#include "Plat/Private/QAutopilotPrivate.h"
#include <QDateTime>
#include <QDebug>
#include <QtGlobal>

// 将FixType枚举转换为中文字符串
static const char* fixTypeToChinese(int fixType) {
    switch (fixType) {
        case 0: // NoGps
            return "无GPS连接";
        case 1: // NoFix
            return "无定位";
        case 2: // Fix2D
            return "2D定位";
        case 3: // Fix3D
            return "3D定位";
        case 4: // FixDgps
            return "DGPS/SBAS辅助定位";
        case 5: // RtkFloat
            return "RTK浮点定位";
        case 6: // RtkFixed
            return "RTK固定定位";
        default:
            return "未知状态";
    }
}

QAutopilot::QAutopilot(QObject *parent) {}

QAutopilot::~QAutopilot() {}

QAutopilotPrivate *QAutopilot::d_func() {
    return static_cast<QAutopilotPrivate *>(d_ptr.get());
}

const QAutopilotPrivate *QAutopilot::d_func() const {
    return static_cast<const QAutopilotPrivate *>(d_ptr.get());
}

void QAutopilot::arm()
{
    d_func()->arm();
}

void QAutopilot::setGpsPosition(const QGpsPosition &position) {
    if (m_gpsPosition != position) {
        m_gpsPosition = position;
        emit gpsPositionChanged(m_gpsPosition);
    }
}

void QAutopilot::setNedPosition(const QNEDPosition &position) {
    if (m_nedPosition != position) {
        m_nedPosition = position;
        emit nedPositionChanged(m_nedPosition);
    }
}

void QAutopilot::setHomePosition(const QGpsPosition &position) {
    if (m_homePosition != position) {
        m_homePosition = position;
        emit homePositionChanged(m_homePosition);
    }
}

void QAutopilot::setStatus(const QAutopilotStatus &status) {
    if (m_status != status) {
        m_status = status;
        emit statusChanged(m_status);
    }
}

void QAutopilot::setHeading(double heading) {
    if (!qFuzzyCompare(m_heading, heading)) {
        m_heading = heading;
        emit headingChanged(m_heading);
    }
}

void QAutopilot::setVehicleType(QAutoVehicleType::Vehicle vehicleType) {
    if (m_vehicleType != vehicleType) {
        m_vehicleType = vehicleType;
        emit vehicleTypeChanged(m_vehicleType);
    }
}

void QAutopilot::setAutopilotType(QAutoVehicleType::Autopilot autopilotType) {
    if (m_autopilotType != autopilotType) {
        m_autopilotType = autopilotType;
        emit autopilotTypeChanged(m_autopilotType);
    }
}

void QAutopilot::positionUpdate(double dLon, double dLat, float dH) {
    if (!qFuzzyCompare(m_gpsPosition.longitude(), dLon) ||
        !qFuzzyCompare(m_gpsPosition.latitude(), dLat) ||
        !qFuzzyCompare(m_gpsPosition.altitude(), dH)) {
        m_gpsPosition.setLongitude(dLon);
        m_gpsPosition.setLatitude(dLat);
        m_gpsPosition.setAltitude(dH);
        emit gpsPositionChanged(m_gpsPosition);
    }
}

void QAutopilot::nedUpdate(float dNorth, float dEast, float dDown) {
    if (!qFuzzyCompare(m_nedPosition.north(), dNorth) ||
        !qFuzzyCompare(m_nedPosition.east(), dEast) ||
        !qFuzzyCompare(m_nedPosition.down(), dDown)) {
        m_nedPosition.setNorth(dNorth);
        m_nedPosition.setEast(dEast);
        m_nedPosition.setDown(dDown);
        emit nedPositionChanged(m_nedPosition);
    }
}

void QAutopilot::gpsInfoUpdate(int gpsCount, int gpsStatus) {
    bool changed = false;
    
    if (m_status.gpsCount() != gpsCount) {
        m_status.setGpsCount(gpsCount);
        changed = true;
    }
    
    QString chineseStatus = QString::fromUtf8(fixTypeToChinese(gpsStatus));
    if (m_status.gpsStatus() != chineseStatus) {
        m_status.setGpsStatus(chineseStatus);
        changed = true;
    }
    
    if (changed) {
        emit statusChanged(m_status);
    }
}

void QAutopilot::batteryUpdate(float batteryVoltage, float batteryRemaining) {
    bool changed = false;

    if (!qFuzzyCompare(m_status.batteryVoltage(), batteryVoltage)) {
        m_status.setBatteryVoltage(batteryVoltage);
        changed = true;
    }
    if (!qFuzzyCompare(m_status.batteryRemaining(), batteryRemaining)) {
        m_status.setBatteryRemaining(batteryRemaining);
        changed = true;
    }

    if (changed) {
        emit statusChanged(m_status);
    }
}

void QAutopilot::rcStatusUpdate(bool isAvailable, float signalStrengthPercent) {
    bool changed = false;
    
    if (m_status.rcIsAvailable() != isAvailable) {
        m_status.setRcIsAvailable(isAvailable);
        changed = true;
    }
    if (!qFuzzyCompare(m_status.rcSignalStrengthPercent(), signalStrengthPercent)) {
        m_status.setRcSignalStrengthPercent(signalStrengthPercent);
        changed = true;
    }
    
    if (changed) {
        emit statusChanged(m_status);
    }
}

void QAutopilot::headingUpdate(double heading) {
    setHeading(heading);
}

void QAutopilot::healthUpdate(bool isGyrometerCalibrationOk, bool isAccelerometerCalibrationOk,
                              bool isMagnetometerCalibrationOk, bool isLocalPositionOk,
                              bool isGlobalPositionOk, bool isHomePositionOk, bool isArmable)
{
    bool changed = false;

    if (m_status.isGyrometerCalibrationOk() != isGyrometerCalibrationOk) {
        m_status.setGyrometerCalibrationOk(isGyrometerCalibrationOk);
        changed = true;
    }
    if (m_status.isAccelerometerCalibrationOk() != isAccelerometerCalibrationOk) {
        m_status.setAccelerometerCalibrationOk(isAccelerometerCalibrationOk);
        changed = true;
    }
    if (m_status.isMagnetometerCalibrationOk() != isMagnetometerCalibrationOk) {
        m_status.setMagnetometerCalibrationOk(isMagnetometerCalibrationOk);
        changed = true;
    }
    if (m_status.isLocalPositionOk() != isLocalPositionOk) {
        m_status.setLocalPositionOk(isLocalPositionOk);
        changed = true;
    }
    if (m_status.isGlobalPositionOk() != isGlobalPositionOk) {
        m_status.setGlobalPositionOk(isGlobalPositionOk);
        changed = true;
    }
    if (m_status.isHomePositionOk() != isHomePositionOk) {
        m_status.setHomePositionOk(isHomePositionOk);
        changed = true;
    }
    if (m_status.isArmable() != isArmable) {
        m_status.setArmable(isArmable);
        changed = true;
    }

    if (changed) {
        emit statusChanged(m_status);
    }
}

void QAutopilot::homeUpdate(double dLon, double dLat, float dH)
{
    if (!qFuzzyCompare(m_homePosition.longitude(), dLon) ||
        !qFuzzyCompare(m_homePosition.latitude(), dLat) ||
        !qFuzzyCompare(m_homePosition.altitude(), dH)) {
        m_homePosition.setLongitude(dLon);
        m_homePosition.setLatitude(dLat);
        m_homePosition.setAltitude(dH);
        emit homePositionChanged(m_homePosition);
    }
}
