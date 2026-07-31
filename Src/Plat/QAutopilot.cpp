#include "Plat/QAutopilot.h"
#include "Plat/Private/QAutopilotPrivate.h"
#include "Private/QMavsdkTextCatalog.h"
#include "QGCSConfig.h"
#include <QDateTime>
#include <QDebug>
#include <QtGlobal>
#include <cmath>

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

bool QAutopilot::sendExternCommand(const QString &name,
                                   quint32 componentId,
                                   const QVector<float> &params)
{
    if (!d_func()) {
        return false;
    }
    return d_func()->sendExternCommand(name, componentId, params);
}

void QAutopilot::downloadAirLine()
{
    if (m_airLineDownloading || m_airLineUploading) {
        emit airLineDownloadFailed(
            m_airLineUploading ? tr("航线正在上传")
                               : tr("航线正在下载"));
        return;
    }
    if (!d_func()) {
        emit airLineDownloadFailed(tr("飞控尚未初始化"));
        return;
    }

    m_airLineDownloading = true;
    const quint64 requestId = ++m_airLineDownloadRequestId;
    emit airLineDownloadingChanged(true);
    d_func()->downloadAirLine(requestId);
}

void QAutopilot::uploadAirLine(const QList<QGpsPosition> &waypoints)
{
    if (m_airLineUploading || m_airLineDownloading) {
        emit airLineUploadFailed(
            m_airLineDownloading ? tr("航线正在下载")
                                 : tr("航线正在上传"));
        return;
    }
    if (waypoints.isEmpty()) {
        emit airLineUploadFailed(tr("航线没有有效航点"));
        return;
    }
    for (qsizetype index = 0; index < waypoints.size(); ++index) {
        const QGpsPosition &waypoint = waypoints.at(index);
        const double latitude = waypoint.latitude();
        const double longitude = waypoint.longitude();
        const double altitude = waypoint.altitude();
        if (!std::isfinite(latitude) || !std::isfinite(longitude) ||
            !std::isfinite(altitude) || latitude < -90.0 ||
            latitude > 90.0 || longitude < -180.0 ||
            longitude > 180.0) {
            emit airLineUploadFailed(
                tr("第 %1 个航点坐标或高度无效").arg(index + 1));
            return;
        }
    }
    if (!d_func()) {
        emit airLineUploadFailed(tr("飞控尚未初始化"));
        return;
    }

    m_airLineUploading = true;
    const quint64 requestId = ++m_airLineUploadRequestId;
    emit airLineUploadingChanged(true);
    d_func()->uploadAirLine(requestId, waypoints);
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

void QAutopilot::completeAirLineUpload(quint64 requestId)
{
    if (requestId != m_airLineUploadRequestId) {
        return;
    }
    m_airLineUploading = false;
    emit airLineUploadingChanged(false);
    emit airLineUploaded();
}

void QAutopilot::failAirLineUpload(quint64 requestId,
                                   const QString &reason)
{
    if (requestId != m_airLineUploadRequestId) {
        return;
    }
    m_airLineUploading = false;
    emit airLineUploadingChanged(false);
    emit airLineUploadFailed(reason);
}

void QAutopilot::cancelAirLineUpload()
{
    if (!m_airLineUploading) {
        return;
    }
    ++m_airLineUploadRequestId;
    m_airLineUploading = false;
    emit airLineUploadingChanged(false);
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
        emit vehicleNameChanged();
    }
}

void QAutopilot::setAutopilotType(QAutoVehicleType::Autopilot autopilotType) {
    if (m_autopilotType != autopilotType) {
        m_autopilotType = autopilotType;
        emit autopilotTypeChanged(m_autopilotType);
        emit autopilotNameChanged();
    }
}

void QAutopilot::positionUpdate(double dLon, double dLat, float dH,
                                float relativeAltitudeM) {
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
    if (!qFuzzyCompare(m_relativeAltitudeM,
                       static_cast<double>(relativeAltitudeM))) {
        m_relativeAltitudeM = relativeAltitudeM;
        emit positionDetailsChanged();
    }
}

QString QAutopilot::autopilotName() const
{
    return QAutoVehicleType::getAutopilotName(m_autopilotType);
}

QString QAutopilot::vehicleName() const
{
    return QAutoVehicleType::getVehicleName(m_vehicleType);
}

QString QAutopilot::flightModeName() const
{
    const QString localized = QMavsdkTextCatalog::text(
        QStringLiteral("flightMode"), m_flightMode);
    const QString missing = QStringLiteral("flightMode(%1)").arg(m_flightMode);
    return localized == missing && !m_flightModeFallbackName.isEmpty()
        ? m_flightModeFallbackName : localized;
}

QString QAutopilot::landedStateName() const
{
    const QString localized = QMavsdkTextCatalog::text(
        QStringLiteral("landedState"), m_landedState);
    const QString missing = QStringLiteral("landedState(%1)").arg(m_landedState);
    return localized == missing && !m_landedStateFallbackName.isEmpty()
        ? m_landedStateFallbackName : localized;
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
    m_velocityNorthMS = northSpeed;
    m_velocityEastMS = eastSpeed;
    m_velocityDownMS = downSpeed;
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
    const auto *config = QGCSConfig::instance();
    const bool movementCandidate =
        m_inAir ||
        m_groundSpeedMS >= config->motionStartHorizontalSpeedMS() ||
        m_verticalSpeedMS >= config->motionStartVerticalSpeedMS();
    const bool stationaryCandidate =
        !m_inAir &&
        m_groundSpeedMS <= config->motionStopHorizontalSpeedMS() &&
        m_verticalSpeedMS <= config->motionStopVerticalSpeedMS();

    if (!m_moving) {
        m_motionStopSamples = 0;
        if (movementCandidate) {
            ++m_motionStartSamples;
            const int requiredSamples =
                m_inAir ? 1 : config->motionStartSampleCount();
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
        if (m_motionStopSamples >= config->motionStopSampleCount()) {
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

void QAutopilot::batteryUpdate(int batteryId, float temperatureC,
                               float batteryVoltage, float batteryCurrentA,
                               float consumedAh, float batteryRemaining,
                               float timeRemainingS, int batteryFunction) {
    bool changed = false;

    if (!qFuzzyCompare(m_status.batteryVoltage(), batteryVoltage)) {
        m_status.setBatteryVoltage(batteryVoltage);
        changed = true;
    }
    if (!qFuzzyCompare(m_status.batteryRemaining(), batteryRemaining)) {
        m_status.setBatteryRemaining(batteryRemaining);
        changed = true;
    }
    if (m_status.batteryId() != batteryId) {
        m_status.setBatteryId(batteryId);
        changed = true;
    }
    if (!qFuzzyCompare(m_status.batteryTemperatureC(), temperatureC)) {
        m_status.setBatteryTemperatureC(temperatureC);
        changed = true;
    }
    if (!qFuzzyCompare(m_status.batteryCurrentA(), batteryCurrentA)) {
        m_status.setBatteryCurrentA(batteryCurrentA);
        changed = true;
    }
    if (!qFuzzyCompare(m_status.batteryConsumedAh(), consumedAh)) {
        m_status.setBatteryConsumedAh(consumedAh);
        changed = true;
    }
    if (!qFuzzyCompare(m_status.batteryTimeRemainingS(), timeRemainingS)) {
        m_status.setBatteryTimeRemainingS(timeRemainingS);
        changed = true;
    }
    const QString function = QMavsdkTextCatalog::text(
        QStringLiteral("batteryFunction"), batteryFunction);
    if (m_status.batteryFunction() != function) {
        m_status.setBatteryFunction(function);
        changed = true;
    }

    if (changed) {
        emit statusChanged(m_status);
    }
}

void QAutopilot::attitudeUpdate(float rollDeg, float pitchDeg, float yawDeg)
{
    if (qFuzzyCompare(m_rollDeg, static_cast<double>(rollDeg)) &&
        qFuzzyCompare(m_pitchDeg, static_cast<double>(pitchDeg)) &&
        qFuzzyCompare(m_yawDeg, static_cast<double>(yawDeg))) {
        return;
    }
    m_rollDeg = rollDeg;
    m_pitchDeg = pitchDeg;
    m_yawDeg = yawDeg;
    emit attitudeChanged();
}

void QAutopilot::flightModeUpdate(int flightMode, const QString &fallbackName)
{
    if (m_flightMode == flightMode &&
        m_flightModeFallbackName == fallbackName) {
        return;
    }
    m_flightMode = flightMode;
    m_flightModeFallbackName = fallbackName;
    emit flightModeChanged();
}

void QAutopilot::landedStateUpdate(int landedState,
                                   const QString &fallbackName)
{
    if (m_landedState == landedState &&
        m_landedStateFallbackName == fallbackName) {
        return;
    }
    m_landedState = landedState;
    m_landedStateFallbackName = fallbackName;
    emit landedStateChanged();
}

void QAutopilot::rawGpsUpdate(float hdop, float vdop, float velocityMS,
                              float courseDeg, float horizontalUncertaintyM,
                              float verticalUncertaintyM,
                              float velocityUncertaintyMS,
                              float headingUncertaintyDeg)
{
    m_gpsHdop = hdop;
    m_gpsVdop = vdop;
    m_gpsVelocityMS = velocityMS;
    m_gpsCourseDeg = courseDeg;
    m_gpsHorizontalUncertaintyM = horizontalUncertaintyM;
    m_gpsVerticalUncertaintyM = verticalUncertaintyM;
    m_gpsVelocityUncertaintyMS = velocityUncertaintyMS;
    m_gpsHeadingUncertaintyDeg = headingUncertaintyDeg;
    emit rawGpsChanged();
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
