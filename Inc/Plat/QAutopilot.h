#ifndef _YTY_QAUTOPILOT_H
#define _YTY_QAUTOPILOT_H

#include "Plat/QPlat.h"
#include "AirLine/QGpsPosition.h"
#include "AirLine/QNEDPosition.h"
#include "Plat/QAutopilotStatus.h"
#include "Plat/QAutopilotFixedwing.h"
#include "Plat/QAutoVehicleType.h"
#include "MiniGCSExport.h"

/**
 * @brief QAutopilot类 - 具备自动驾驶功能的系统
 * 
 * 该类封装了自动驾驶功能的所有信息，包括连接状态、硬件信息等
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
    Q_PROPERTY(double heading READ heading NOTIFY headingChanged)
    Q_PROPERTY(double groundSpeedMS READ groundSpeedMS NOTIFY motionChanged)
    Q_PROPERTY(double verticalSpeedMS READ verticalSpeedMS NOTIFY motionChanged)
    Q_PROPERTY(bool moving READ moving NOTIFY motionChanged)
    Q_PROPERTY(bool armed READ armed NOTIFY armedChanged)
    Q_PROPERTY(bool inAir READ inAir NOTIFY inAirChanged)
    Q_PROPERTY(bool airLineDownloading READ airLineDownloading NOTIFY airLineDownloadingChanged)
    Q_PROPERTY(QAutoVehicleType::Vehicle vehicleType READ vehicleType WRITE setVehicleType NOTIFY vehicleTypeChanged)
    Q_PROPERTY(QAutoVehicleType::Autopilot autopilotType READ autopilotType WRITE setAutopilotType NOTIFY autopilotTypeChanged)

public:
    explicit QAutopilot(QObject *parent = nullptr);
    ~QAutopilot();

    /**
     * @brief 解锁无人机
     */
    Q_INVOKABLE void arm();
    Q_INVOKABLE void disarm();
    Q_INVOKABLE void takeoff();
    Q_INVOKABLE void land();
    Q_INVOKABLE void returnToLaunch();

    /**
     * @brief 显式下载当前任务航线
     */
    Q_INVOKABLE void downloadAirLine();
    bool airLineDownloading() const { return m_airLineDownloading; }

    /**
     * @brief 获取GPS位置
     * @return GPS位置
     */
    QGpsPosition gpsPosition() const { return m_gpsPosition; }
    bool hasGpsPosition() const { return m_hasGpsPosition; }

    /**
     * @brief 获取NED位置
     * @return NED位置
     */
    QNEDPosition nedPosition() const { return m_nedPosition; }

    /**
     * @brief 获取Home位置
     * @return Home位置
     */
    QGpsPosition homePosition() const { return m_homePosition; }

    /**
     * @brief 获取状态信息
     * @return 状态信息
     */
    QAutopilotStatus status() const { return m_status; }

    /**
     * @brief 获取固定翼状态
     * @return 固定翼状态
     */
    QAutopilotFixedwing fixedwing() const { return m_fixedwing; }

    /**
     * @brief 获取航向角
     * @return 航向角（度）
     */
    double heading() const { return m_heading; }
    double groundSpeedMS() const { return m_groundSpeedMS; }
    double verticalSpeedMS() const { return m_verticalSpeedMS; }
    bool moving() const { return m_moving; }
    bool armed() const { return m_armed; }
    bool inAir() const { return m_inAir; }

    /**
     * @brief 设置航向角
     * @param heading 航向角（度）
     */
    void setHeading(double heading);

    /**
     * @brief 获取载具类型
     * @return 载具类型
     */
    QAutoVehicleType::Vehicle vehicleType() const { return m_vehicleType; }

    /**
     * @brief 设置载具类型
     * @param vehicleType 载具类型
     */
    void setVehicleType(QAutoVehicleType::Vehicle vehicleType);

    /**
     * @brief 获取自动驾驶仪类型
     * @return 自动驾驶仪类型
     */
    QAutoVehicleType::Autopilot autopilotType() const { return m_autopilotType; }

    /**
     * @brief 设置自动驾驶仪类型
     * @param autopilotType 自动驾驶仪类型
     */
    void setAutopilotType(QAutoVehicleType::Autopilot autopilotType);


signals:
    /**
     * @brief GPS位置信息变化信号
     * @param position 新的GPS位置信息
     */
    void gpsPositionChanged(const QGpsPosition &position);
    void hasGpsPositionChanged(bool available);

    /**
     * @brief NED位置信息变化信号
     * @param position 新的NED位置信息
     */
    void nedPositionChanged(const QNEDPosition &position);

    /**
     * @brief Home位置信息变化信号
     * @param position 新的Home位置信息
     */
    void homePositionChanged(const QGpsPosition &position);

    /**
     * @brief 状态信息变化信号
     * @param status 新的状态信息
     */
    void statusChanged(const QAutopilotStatus &status);

    /**
     * @brief 航向角变化信号
     * @param heading 新的航向角（度）
     */
    void headingChanged(double heading);
    void motionChanged();
    void armedChanged(bool armed);
    void inAirChanged(bool inAir);

    /**
     * @brief 固定翼状态变化信号
     * @param fixedwing 新的固定翼状态
     */
    void fixedwingChanged(const QAutopilotFixedwing &fixedwing);

    /**
     * @brief 载具类型变化信号
     * @param vehicleType 新的载具类型
     */
    void vehicleTypeChanged(QAutoVehicleType::Vehicle vehicleType);

    /**
     * @brief 自动驾驶仪类型变化信号
     * @param autopilotType 新的自动驾驶仪类型
     */
    void autopilotTypeChanged(QAutoVehicleType::Autopilot autopilotType);

    /**
     * @brief 航线下载完成
     * @note 航点高度为 MAVSDK Mission 提供的相对起飞点高度
     */
    void airLineDownloaded(const QList<QGpsPosition> &waypoints);

    /**
     * @brief 航线下载失败
     */
    void airLineDownloadFailed(const QString &reason);
    void airLineDownloadingChanged(bool downloading);

protected slots:
    void positionUpdate(double dLon, double dLat, float dH);
    void nedUpdate(float dNorth, float dEast, float dDown,
                   float velocityNorth, float velocityEast,
                   float velocityDown);
    void armedUpdate(bool armed);
    void inAirUpdate(bool inAir);
    void gpsInfoUpdate(int gpsCount, int gpsStatus);
    void batteryUpdate(float batteryVoltage, float batteryRemaining);
    void rcStatusUpdate(bool isAvailable, float signalStrengthPercent);
    void headingUpdate(double heading);
    void healthUpdate(bool isGyrometerCalibrationOk, bool isAccelerometerCalibrationOk,
                      bool isMagnetometerCalibrationOk, bool isLocalPositionOk,
                      bool isGlobalPositionOk, bool isHomePositionOk, bool isArmable);
    void homeUpdate(double dLon,double dLat,float dH);
    void fixedwingUpdate(float airspeedMS, float throttlePercentage, float climbRateMS,
                        float groundspeedMS, float headingDeg, float absoluteAltitudeM);

protected:

    friend class QAutopilotPrivate;
    void completeAirLineDownload(quint64 requestId,
                                 const QList<QGpsPosition> &waypoints);
    void failAirLineDownload(quint64 requestId, const QString &reason);
    void cancelAirLineDownload();
    void updateMovingState();
    /**
     * @brief 获取QAutopilotPrivate指针的辅助方法
     * @return QAutopilotPrivate指针
     */
    QAutopilotPrivate* d_func();
    const QAutopilotPrivate* d_func() const;

    QGpsPosition m_gpsPosition;
    bool m_hasGpsPosition{false};
    QNEDPosition m_nedPosition;
    QGpsPosition m_homePosition;
    QAutopilotStatus m_status;
    QAutopilotFixedwing m_fixedwing;
    double m_heading{0.0};  ///< 航向角（度）
    double m_groundSpeedMS{0.0};
    double m_verticalSpeedMS{0.0};
    bool m_moving{false};
    bool m_armed{false};
    bool m_inAir{false};
    int m_motionStartSamples{0};
    int m_motionStopSamples{0};
    QAutoVehicleType::Vehicle m_vehicleType{QAutoVehicleType::Vehicle_Unknown};  ///< 载具类型
    QAutoVehicleType::Autopilot m_autopilotType{QAutoVehicleType::Autopilot_Unknown};  ///< 自动驾驶仪类型
    bool m_airLineDownloading{false};
    quint64 m_airLineDownloadRequestId{0};
};

#endif // _YTY_QAUTOPILOT_H
