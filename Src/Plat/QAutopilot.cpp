#include "Plat/QAutopilot.h"
#include "Plat/Private/QAutopilotPrivate.h"
#include "Private/QMavsdkTextCatalog.h"
#include <QDateTime>
#include <QDebug>
#include <QtGlobal>
#include <cmath>

namespace {
constexpr double MotionStartHorizontalMS = 0.7;
constexpr double MotionStartVerticalMS = 0.5;
constexpr double MotionStopHorizontalMS = 0.25;
constexpr double MotionStopVerticalMS = 0.2;
constexpr int MotionStartSampleCount = 2;
constexpr int MotionStopSampleCount = 5;
}

QAutopilot::QAutopilot(QObject *parent)
    : QPlat(parent)
{
}

QAutopilot::~QAutopilot() {}

QAutopilotPrivate *QAutopilot::d_func() {
    return static_cast<QAutopilotPrivate *>(d_ptr.get());
}

const QAutopilotPrivate *QAutopilot::d_func() const {
    return static_cast<const QAutopilotPrivate *>(d_ptr.get());
}

void QAutopilot::arm()
{
    if (d_func()) {
        d_func()->arm();
    }
}

void QAutopilot::disarm()
{
    if (d_func()) {
        d_func()->disarm();
    }
}

void QAutopilot::takeoff()
{
    if (d_func()) {
        d_func()->takeoff();
    }
}

void QAutopilot::land()
{
    if (d_func()) {
        d_func()->land();
    }
}

void QAutopilot::returnToLaunch()
{
    if (d_func()) {
        d_func()->returnToLaunch();
    }
}

void QAutopilot::downloadAirLine()
{
    if (m_airLineDownloading) {
        emit airLineDownloadFailed(QStringLiteral("航线正在下载"));
        return;
    }
    if (!d_func()) {
        emit airLineDownloadFailed(QStringLiteral("飞控尚未初始化"));
        return;
    }

    m_airLineDownloading = true;
    const quint64 requestId = ++m_airLineDownloadRequestId;
    emit airLineDownloadingChanged(true);
    d_func()->downloadAirLine(requestId);
}

void QAutopilot::completeAirLineDownload(
    quint64 requestId, const QList<QGpsPosition> &waypoints)
{
    if (requestId != m_airLineDownloadRequestId) {
        return;
    }
    m_airLineDownloading = false;
    emit airLineDownloadingChanged(false);
    emit airLineDownloaded(waypoints);
}

void QAutopilot::failAirLineDownload(quint64 requestId,
                                     const QString &reason)
{
    if (requestId != m_airLineDownloadRequestId) {
        return;
    }
    m_airLineDownloading = false;
    emit airLineDownloadingChanged(false);
    emit airLineDownloadFailed(reason);
}

void QAutopilot::cancelAirLineDownload()
{
    if (!m_airLineDownloading) {
        return;
    }
    ++m_airLineDownloadRequestId;
    m_airLineDownloading = false;
    emit airLineDownloadingChanged(false);
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
    const bool firstPosition = !m_hasGpsPosition;
    if (firstPosition ||
        !qFuzzyCompare(m_gpsPosition.longitude(), dLon) ||
        !qFuzzyCompare(m_gpsPosition.latitude(), dLat) ||
        !qFuzzyCompare(m_gpsPosition.altitude(), dH)) {
        m_hasGpsPosition = true;
        m_gpsPosition.setLongitude(dLon);
        m_gpsPosition.setLatitude(dLat);
        m_gpsPosition.setAltitude(dH);
        if (firstPosition) {
            emit hasGpsPositionChanged(true);
        }
        emit gpsPositionChanged(m_gpsPosition);
    }
}

void QAutopilot::nedUpdate(float dNorth, float dEast, float dDown,
                           float velocityNorth, float velocityEast,
                           float velocityDown) {
    if (!qFuzzyCompare(m_nedPosition.north(), dNorth) ||
        !qFuzzyCompare(m_nedPosition.east(), dEast) ||
        !qFuzzyCompare(m_nedPosition.down(), dDown)) {
        m_nedPosition.setNorth(dNorth);
        m_nedPosition.setEast(dEast);
        m_nedPosition.setDown(dDown);
        emit nedPositionChanged(m_nedPosition);
    }

    const double northSpeed = static_cast<double>(velocityNorth);
    const double eastSpeed = static_cast<double>(velocityEast);
    const double downSpeed = static_cast<double>(velocityDown);
    if (!std::isfinite(northSpeed) ||
        !std::isfinite(eastSpeed) ||
        !std::isfinite(downSpeed)) {
        return;
    }

    m_groundSpeedMS = std::hypot(northSpeed, eastSpeed);
    m_verticalSpeedMS = std::abs(downSpeed);
    updateMovingState();
    emit motionChanged();
}

void QAutopilot::armedUpdate(bool armed)
{
    if (m_armed == armed) {
        return;
    }
    m_armed = armed;
    emit armedChanged(m_armed);
}

void QAutopilot::inAirUpdate(bool inAir)
{
    if (m_inAir == inAir) {
        return;
    }
    m_inAir = inAir;
    updateMovingState();
    emit inAirChanged(m_inAir);
    emit motionChanged();
}

void QAutopilot::updateMovingState()
{
    const bool movementCandidate =
        m_inAir ||
        m_groundSpeedMS >= MotionStartHorizontalMS ||
        m_verticalSpeedMS >= MotionStartVerticalMS;
    const bool stationaryCandidate =
        !m_inAir &&
        m_groundSpeedMS <= MotionStopHorizontalMS &&
        m_verticalSpeedMS <= MotionStopVerticalMS;

    if (!m_moving) {
        m_motionStopSamples = 0;
        if (movementCandidate) {
            ++m_motionStartSamples;
            const int requiredSamples =
                m_inAir ? 1 : MotionStartSampleCount;
            if (m_motionStartSamples >= requiredSamples) {
                m_moving = true;
                m_motionStartSamples = 0;
            }
        } else {
            m_motionStartSamples = 0;
        }
        return;
    }

    m_motionStartSamples = 0;
    if (stationaryCandidate) {
        ++m_motionStopSamples;
        if (m_motionStopSamples >= MotionStopSampleCount) {
            m_moving = false;
            m_motionStopSamples = 0;
        }
    } else {
        m_motionStopSamples = 0;
    }
}

void QAutopilot::gpsInfoUpdate(int gpsCount, int gpsStatus) {
    bool changed = false;
    
    if (m_status.gpsCount() != gpsCount) {
        m_status.setGpsCount(gpsCount);
        changed = true;
    }
    
    const QString chineseStatus =
        QMavsdkTextCatalog::text(QStringLiteral("gpsFixType"), gpsStatus);
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

void QAutopilot::fixedwingUpdate(float airspeedMS, float throttlePercentage, float climbRateMS,
                                 float groundspeedMS, float headingDeg, float absoluteAltitudeM)
{
    bool changed = false;
    
    // 检查空速
    if (!qFuzzyCompare(m_fixedwing.airspeedMS(), airspeedMS)) {
        m_fixedwing.setAirspeedMS(airspeedMS);
        changed = true;
    }
    
    // 检查油门
    if (!qFuzzyCompare(m_fixedwing.throttlePercentage(), throttlePercentage)) {
        m_fixedwing.setThrottlePercentage(throttlePercentage);
        changed = true;
    }
    
    // 检查爬升率
    if (!qFuzzyCompare(m_fixedwing.climbRateMS(), climbRateMS)) {
        m_fixedwing.setClimbRateMS(climbRateMS);
        changed = true;
    }
    
    // 检查地速
    if (!qFuzzyCompare(m_fixedwing.groundspeedMS(), groundspeedMS)) {
        m_fixedwing.setGroundspeedMS(groundspeedMS);
        changed = true;
    }
    
    // 检查航向
    if (!qFuzzyCompare(m_fixedwing.headingDeg(), headingDeg)) {
        m_fixedwing.setHeadingDeg(headingDeg);
        changed = true;
    }
    
    // 检查绝对高度
    if (!qFuzzyCompare(m_fixedwing.absoluteAltitudeM(), absoluteAltitudeM)) {
        m_fixedwing.setAbsoluteAltitudeM(absoluteAltitudeM);
        changed = true;
    }
    
    if (changed) {
        emit fixedwingChanged(m_fixedwing);
    }
}
