#include "Plat/QAutopilot.h"
#include "Plat/Private/QAutopilotPrivate.h"
#include "Private/QMavsdkTextCatalog.h"
#include "QGCSConfig.h"
#include <QDateTime>
#include <QDebug>
#include <QMetaType>
#include <QtGlobal>
#include <cmath>

QAutopilot::QAutopilot(QObject *parent)
    : QPlat(parent)
{
    qRegisterMetaType<QAutopilot::FlightMode>("QAutopilot::FlightMode");
    qRegisterMetaType<QAutopilot::LandedState>("QAutopilot::LandedState");
    qRegisterMetaType<QList<QMissionPoint>>("QList<QMissionPoint>");
    connect(this, &QPlat::connectionStatusChanged, this,
            [this](bool connected) {
                if (!connected) {
                    missionActiveUpdate(false);
                }
            });
    connect(this, &QAutopilot::actionCommandFinished, this,
            [this](ActionCommand command, bool success, const QString &) {
                if (!success) {
                    return;
                }
                if (command == DisarmAction || command == LandAction ||
                    command == ReturnToLaunchAction) {
                    missionActiveUpdate(false);
                }
            });
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
    QList<QMissionPoint> points;
    points.reserve(waypoints.size());
    for (const QGpsPosition &waypoint : waypoints) {
        points.append(QMissionPoint(waypoint));
    }
    uploadMission(points, true);
}

void QAutopilot::uploadMission(const QList<QMissionPoint> &points,
                               bool returnHomeAfterMission)
{
    if (m_airLineUploading || m_airLineDownloading) {
        emit airLineUploadFailed(
            m_airLineDownloading ? tr("航线正在下载")
                                 : tr("航线正在上传"));
        return;
    }
    if (points.isEmpty()) {
        emit airLineUploadFailed(tr("航线没有有效航点"));
        return;
    }
    for (qsizetype index = 0; index < points.size(); ++index) {
        const QMissionPoint &point = points.at(index);
        const QGpsPosition waypoint = point.position();
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
        if (!std::isfinite(point.actionDurationS()) ||
            point.actionDurationS() < 0.0 ||
            !std::isfinite(point.speedMS()) || point.speedMS() < 0.0) {
            emit airLineUploadFailed(
                tr("第 %1 个任务点动作参数无效").arg(index + 1));
            return;
        }
        if (point.action() == QMissionPoint::LandAction &&
            index != points.size() - 1) {
            emit airLineUploadFailed(
                tr("降落动作只能设置在最后一个任务点"));
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
    d_func()->uploadAirLine(requestId, points, returnHomeAfterMission);
}

void QAutopilot::startAirLine()
{
    if (m_airLineUploading || m_airLineDownloading) {
        emit airLineStartFailed(
            m_airLineUploading ? tr("航线正在上传")
                               : tr("航线正在下载"));
        return;
    }
    if (!d_func()) {
        emit airLineStartFailed(tr("飞控尚未初始化"));
        return;
    }
    d_func()->startAirLine();
}

void QAutopilot::completeAirLineDownload(
    quint64 requestId, const QList<QMissionPoint> &points)
{
    if (requestId != m_airLineDownloadRequestId) {
        return;
    }
    m_airLineDownloading = false;
    emit airLineDownloadingChanged(false);
    QList<QGpsPosition> waypoints;
    waypoints.reserve(points.size());
    for (const QMissionPoint &point : points) {
        waypoints.append(point.position());
    }
    emit missionDownloaded(points);
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
        QStringLiteral("flightMode"), static_cast<int>(m_flightMode));
    const QString missing =
        QStringLiteral("flightMode(%1)").arg(static_cast<int>(m_flightMode));
    return localized == missing && !m_flightModeFallbackName.isEmpty()
        ? m_flightModeFallbackName : localized;
}

QString QAutopilot::landedStateName() const
{
    const QString localized = QMavsdkTextCatalog::text(
        QStringLiteral("landedState"), static_cast<int>(m_landedState));
    const QString missing =
        QStringLiteral("landedState(%1)").arg(static_cast<int>(m_landedState));
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

    QVelocity next(northSpeed, eastSpeed, downSpeed);
    if (m_velocity != next) {
        m_velocity = next;
        emit velocityChanged(m_velocity);
    }
    updateMovingState();
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
    emit inAirChanged(m_inAir);
    updateMovingState();
}

void QAutopilot::updateMovingState()
{
    const auto *config = QGCSConfig::instance();
    const bool movementCandidate =
        m_inAir ||
        m_velocity.groundSpeedMS() >= config->motionStartHorizontalSpeedMS() ||
        m_velocity.verticalSpeedMS() >= config->motionStartVerticalSpeedMS();
    const bool stationaryCandidate =
        !m_inAir &&
        m_velocity.groundSpeedMS() <= config->motionStopHorizontalSpeedMS() &&
        m_velocity.verticalSpeedMS() <= config->motionStopVerticalSpeedMS();

    bool nextMoving = m_moving;
    if (!m_moving) {
        m_motionStopSamples = 0;
        if (movementCandidate) {
            ++m_motionStartSamples;
            const int requiredSamples =
                m_inAir ? 1 : config->motionStartSampleCount();
            if (m_motionStartSamples >= requiredSamples) {
                nextMoving = true;
                m_motionStartSamples = 0;
            }
        } else {
            m_motionStartSamples = 0;
        }
    } else {
        m_motionStartSamples = 0;
        if (stationaryCandidate) {
            ++m_motionStopSamples;
            if (m_motionStopSamples >= config->motionStopSampleCount()) {
                nextMoving = false;
                m_motionStopSamples = 0;
            }
        } else {
            m_motionStopSamples = 0;
        }
    }

    if (m_moving != nextMoving) {
        m_moving = nextMoving;
        emit movingChanged(m_moving);
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
    QAttitude next = m_attitude;
    next.setRollDeg(rollDeg);
    next.setPitchDeg(pitchDeg);
    next.setYawDeg(yawDeg);
    if (m_attitude == next) {
        return;
    }
    m_attitude = next;
    emit attitudeChanged(m_attitude);
}

void QAutopilot::flightModeUpdate(QAutopilot::FlightMode flightMode,
                                   const QString &fallbackName)
{
    if (m_flightMode == flightMode &&
        m_flightModeFallbackName == fallbackName) {
        return;
    }
    m_flightMode = flightMode;
    m_flightModeFallbackName = fallbackName;
    if (flightMode == FlightModeMission) {
        missionActiveUpdate(true);
    } else if (flightMode == FlightModeReturnToLaunch ||
               flightMode == FlightModeLand ||
               flightMode == FlightModeReady) {
        missionActiveUpdate(false);
    }
    emit flightModeChanged();
}

void QAutopilot::landedStateUpdate(QAutopilot::LandedState landedState,
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
    QRawGps next = m_rawGps;
    next.setHdop(hdop);
    next.setVdop(vdop);
    next.setVelocityMS(velocityMS);
    next.setCourseDeg(courseDeg);
    next.setHorizontalUncertaintyM(horizontalUncertaintyM);
    next.setVerticalUncertaintyM(verticalUncertaintyM);
    next.setVelocityUncertaintyMS(velocityUncertaintyMS);
    next.setHeadingUncertaintyDeg(headingUncertaintyDeg);
    if (m_rawGps == next) {
        return;
    }
    m_rawGps = next;
    emit rawGpsChanged(m_rawGps);
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
    if (qFuzzyCompare(m_attitude.headingDeg(), heading)) {
        return;
    }
    m_attitude.setHeadingDeg(heading);
    emit attitudeChanged(m_attitude);
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

void QAutopilot::pauseAirLine()
{
    if (m_airLineUploading || m_airLineDownloading) {
        emit airLinePauseFailed(
            m_airLineUploading ? tr("航线正在上传")
                               : tr("航线正在下载"));
        return;
    }
    if (!d_func()) {
        emit airLinePauseFailed(tr("飞控尚未初始化"));
        return;
    }
    d_func()->pauseAirLine();
}

void QAutopilot::missionProgressUpdate(int current, int total)
{
    if (m_missionCurrent == current && m_missionTotal == total) {
        if (total > 0 && current >= total) {
            missionActiveUpdate(false);
        }
        return;
    }
    m_missionCurrent = current;
    m_missionTotal = total;
    emit missionProgressChanged();
    if (total > 0 && current >= total) {
        missionActiveUpdate(false);
    }
}

void QAutopilot::missionActiveUpdate(bool active)
{
    if (m_missionActive == active) {
        return;
    }
    m_missionActive = active;
    emit missionActiveChanged(active);
}
