#ifndef _YTY_QAUTOPILOT_H
#define _YTY_QAUTOPILOT_H

#include "Plat/QPlat.h"
#include "Common/QGpsPosition.h"
#include "Common/QNEDPosition.h"
#include "Common/QAttitude.h"
#include "Common/QVelocity.h"
#include "Common/QRawGps.h"
#include "Plat/QAutopilotStatus.h"
#include "Plat/QAutopilotFixedwing.h"
#include "Plat/QAutoVehicleType.h"
#include "MiniGCSExport.h"

#include <QVector>

/**
 * @brief QAutopilot - 具备自动驾驶能力的飞行平台
 *
 * 对外仅暴露业务状态与业务控制接口。
 */
class MINIGCS_EXPORT QAutopilot : public QPlat
{
    Q_OBJECT
    Q_PROPERTY(QGpsPosition gpsPosition READ gpsPosition NOTIFY gpsPositionChanged)
    Q_PROPERTY(bool hasGpsPosition READ hasGpsPosition NOTIFY hasGpsPositionChanged)
    Q_PROPERTY(QNEDPosition nedPosition READ nedPosition NOTIFY nedPositionChanged)
    Q_PROPERTY(QGpsPosition homePosition READ homePosition NOTIFY homePositionChanged)
    Q_PROPERTY(QAutopilotStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(QAutopilotFixedwing fixedwing READ fixedwing NOTIFY fixedwingChanged)
    Q_PROPERTY(QAttitude attitude READ attitude NOTIFY attitudeChanged)
    Q_PROPERTY(QVelocity velocity READ velocity NOTIFY velocityChanged)
    Q_PROPERTY(QRawGps rawGps READ rawGps NOTIFY rawGpsChanged)
    Q_PROPERTY(double relativeAltitudeM READ relativeAltitudeM NOTIFY positionDetailsChanged)
    Q_PROPERTY(bool moving READ moving NOTIFY movingChanged)
    Q_PROPERTY(bool armed READ armed NOTIFY armedChanged)
    Q_PROPERTY(bool inAir READ inAir NOTIFY inAirChanged)
    Q_PROPERTY(FlightMode flightMode READ flightMode NOTIFY flightModeChanged)
    Q_PROPERTY(QString flightModeName READ flightModeName NOTIFY flightModeChanged)
    Q_PROPERTY(LandedState landedState READ landedState NOTIFY landedStateChanged)
    Q_PROPERTY(QString landedStateName READ landedStateName NOTIFY landedStateChanged)
    Q_PROPERTY(bool airLineDownloading READ airLineDownloading NOTIFY airLineDownloadingChanged)
    Q_PROPERTY(bool airLineUploading READ airLineUploading NOTIFY airLineUploadingChanged)
    Q_PROPERTY(QAutoVehicleType::Vehicle vehicleType READ vehicleType WRITE setVehicleType NOTIFY vehicleTypeChanged)
    Q_PROPERTY(QString vehicleName READ vehicleName NOTIFY vehicleNameChanged)
    Q_PROPERTY(QAutoVehicleType::Autopilot autopilotType READ autopilotType WRITE setAutopilotType NOTIFY autopilotTypeChanged)
    Q_PROPERTY(QString autopilotName READ autopilotName NOTIFY autopilotNameChanged)

public:
    enum ActionCommand {
        ArmAction,
        DisarmAction,
        TakeoffAction,
        LandAction,
        ReturnToLaunchAction
    };
    Q_ENUM(ActionCommand)

    enum FlightMode {
        FlightModeUnknown = 0,
        FlightModeReady,
        FlightModeTakeoff,
        FlightModeHold,
        FlightModeMission,
        FlightModeReturnToLaunch,
        FlightModeLand,
        FlightModeOffboard,
        FlightModeFollowMe,
        FlightModeManual,
        FlightModeAltitudeControl,
        FlightModePositionControl,
        FlightModeAcro,
        FlightModeStabilized,
        FlightModeRattitude
    };
    Q_ENUM(FlightMode)

    enum LandedState {
        LandedStateUnknown = 0,
        LandedOnGround,
        LandedInAir,
        LandedTakingOff,
        LandedLanding
    };
    Q_ENUM(LandedState)

    explicit QAutopilot(QObject *parent = nullptr);
    ~QAutopilot();

    Q_INVOKABLE void arm();
    Q_INVOKABLE void disarm();
    Q_INVOKABLE void takeoff();
    Q_INVOKABLE void land();
    Q_INVOKABLE void returnToLaunch();

    /** 下载当前任务航线 */
    Q_INVOKABLE void downloadAirLine();
    bool airLineDownloading() const { return m_airLineDownloading; }

    /**
     * @brief 上传任务航线
     * @param waypoints 航点列表，高度为相对起飞点高度（米）
     */
    Q_INVOKABLE void uploadAirLine(const QList<QGpsPosition> &waypoints);
    bool airLineUploading() const { return m_airLineUploading; }

    QGpsPosition gpsPosition() const { return m_gpsPosition; }
    bool hasGpsPosition() const { return m_hasGpsPosition; }
    QNEDPosition nedPosition() const { return m_nedPosition; }
    QGpsPosition homePosition() const { return m_homePosition; }
    QAutopilotStatus status() const { return m_status; }
    QAutopilotFixedwing fixedwing() const { return m_fixedwing; }
    QAttitude attitude() const { return m_attitude; }
    QVelocity velocity() const { return m_velocity; }
    QRawGps rawGps() const { return m_rawGps; }
    double relativeAltitudeM() const { return m_relativeAltitudeM; }
    bool moving() const { return m_moving; }
    bool armed() const { return m_armed; }
    bool inAir() const { return m_inAir; }
    FlightMode flightMode() const { return m_flightMode; }
    QString flightModeName() const;
    LandedState landedState() const { return m_landedState; }
    QString landedStateName() const;

    QAutoVehicleType::Vehicle vehicleType() const { return m_vehicleType; }
    QString vehicleName() const;
    void setVehicleType(QAutoVehicleType::Vehicle vehicleType);

    QAutoVehicleType::Autopilot autopilotType() const { return m_autopilotType; }
    QString autopilotName() const;
    void setAutopilotType(QAutoVehicleType::Autopilot autopilotType);

signals:
    /** 业务控制命令完成（确认、拒绝或超时） */
    void actionCommandFinished(
        QAutopilot::ActionCommand command, bool success,
        const QString &reason);

    void gpsPositionChanged(const QGpsPosition &position);
    void hasGpsPositionChanged(bool available);
    void nedPositionChanged(const QNEDPosition &position);
    void homePositionChanged(const QGpsPosition &position);
    void statusChanged(const QAutopilotStatus &status);
    void positionDetailsChanged();
    void attitudeChanged(const QAttitude &attitude);
    void velocityChanged(const QVelocity &velocity);
    void movingChanged(bool moving);
    void armedChanged(bool armed);
    void inAirChanged(bool inAir);
    void flightModeChanged();
    void landedStateChanged();
    void rawGpsChanged(const QRawGps &rawGps);
    void fixedwingChanged(const QAutopilotFixedwing &fixedwing);
    void vehicleTypeChanged(QAutoVehicleType::Vehicle vehicleType);
    void vehicleNameChanged();
    void autopilotTypeChanged(QAutoVehicleType::Autopilot autopilotType);
    void autopilotNameChanged();

    /** 航线下载完成；航点高度为相对起飞点高度 */
    void airLineDownloaded(const QList<QGpsPosition> &waypoints);
    void airLineDownloadFailed(const QString &reason);
    void airLineDownloadingChanged(bool downloading);
    void airLineUploaded();
    void airLineUploadFailed(const QString &reason);
    void airLineUploadingChanged(bool uploading);

private slots:
    void positionUpdate(double dLon, double dLat, float dH,
                        float relativeAltitudeM);
    void nedUpdate(float dNorth, float dEast, float dDown,
                   float velocityNorth, float velocityEast,
                   float velocityDown);
    void armedUpdate(bool armed);
    void inAirUpdate(bool inAir);
    void gpsInfoUpdate(int gpsCount, int gpsStatus);
    void batteryUpdate(int batteryId, float temperatureC,
                       float batteryVoltage, float batteryCurrentA,
                       float consumedAh, float batteryRemaining,
                       float timeRemainingS, int batteryFunction);
    void rcStatusUpdate(bool isAvailable, float signalStrengthPercent);
    void headingUpdate(double heading);
    void attitudeUpdate(float rollDeg, float pitchDeg, float yawDeg);
    void flightModeUpdate(QAutopilot::FlightMode flightMode,
                          const QString &fallbackName);
    void landedStateUpdate(QAutopilot::LandedState landedState,
                           const QString &fallbackName);
    void rawGpsUpdate(float hdop, float vdop, float velocityMS,
                      float courseDeg, float horizontalUncertaintyM,
                      float verticalUncertaintyM,
                      float velocityUncertaintyMS,
                      float headingUncertaintyDeg);
    void healthUpdate(bool isGyrometerCalibrationOk, bool isAccelerometerCalibrationOk,
                      bool isMagnetometerCalibrationOk, bool isLocalPositionOk,
                      bool isGlobalPositionOk, bool isHomePositionOk, bool isArmable);
    void homeUpdate(double dLon, double dLat, float dH);
    void fixedwingUpdate(float airspeedMS, float throttlePercentage, float climbRateMS,
                        float groundspeedMS, float headingDeg, float absoluteAltitudeM);

private:
    friend class QAutopilotPrivate;
    void completeAirLineDownload(quint64 requestId,
                                 const QList<QGpsPosition> &waypoints);
    void failAirLineDownload(quint64 requestId, const QString &reason);
    void cancelAirLineDownload();
    void completeAirLineUpload(quint64 requestId);
    void failAirLineUpload(quint64 requestId, const QString &reason);
    void cancelAirLineUpload();
    void updateMovingState();
    QAutopilotPrivate* d_func();
    const QAutopilotPrivate* d_func() const;

    QGpsPosition m_gpsPosition;
    bool m_hasGpsPosition{false};
    QNEDPosition m_nedPosition;
    QGpsPosition m_homePosition;
    QAutopilotStatus m_status;
    QAutopilotFixedwing m_fixedwing;
    QAttitude m_attitude;
    QVelocity m_velocity;
    QRawGps m_rawGps;
    double m_relativeAltitudeM{0.0};
    bool m_moving{false};
    bool m_armed{false};
    bool m_inAir{false};
    FlightMode m_flightMode{FlightModeUnknown};
    LandedState m_landedState{LandedStateUnknown};
    QString m_flightModeFallbackName;
    QString m_landedStateFallbackName;
    int m_motionStartSamples{0};
    int m_motionStopSamples{0};
    QAutoVehicleType::Vehicle m_vehicleType{QAutoVehicleType::Vehicle_Unknown};
    QAutoVehicleType::Autopilot m_autopilotType{QAutoVehicleType::Autopilot_Unknown};
    bool m_airLineDownloading{false};
    quint64 m_airLineDownloadRequestId{0};
    bool m_airLineUploading{false};
    quint64 m_airLineUploadRequestId{0};
};

#endif // _YTY_QAUTOPILOT_H
