#ifndef _YTY_QAUTOPILOT_H
#define _YTY_QAUTOPILOT_H

#include "Plat/QPlat.h"
#include "AirLine/QGpsPosition.h"
#include "AirLine/QNEDPosition.h"
#include "Plat/QAutopilotStatus.h"
#include "Plat/QAutoVehicleType.h"

/**
 * @brief QAutopilot类 - 具备自动驾驶功能的系统
 * 
 * 该类封装了自动驾驶功能的所有信息，包括连接状态、硬件信息等
 */
class QAutopilot : public QPlat
{
    Q_OBJECT
    Q_PROPERTY(QGpsPosition gpsPosition READ gpsPosition WRITE setGpsPosition NOTIFY gpsPositionChanged)
    Q_PROPERTY(QNEDPosition nedPosition READ nedPosition WRITE setNedPosition NOTIFY nedPositionChanged)
    Q_PROPERTY(QGpsPosition homePosition READ homePosition WRITE setHomePosition NOTIFY homePositionChanged)
    Q_PROPERTY(QAutopilotStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(double heading READ heading WRITE setHeading NOTIFY headingChanged)
    Q_PROPERTY(QAutoVehicleType::Vehicle vehicleType READ vehicleType WRITE setVehicleType NOTIFY vehicleTypeChanged)
    Q_PROPERTY(QAutoVehicleType::Autopilot autopilotType READ autopilotType WRITE setAutopilotType NOTIFY autopilotTypeChanged)

public:
    explicit QAutopilot(QObject *parent = nullptr);
    ~QAutopilot();

    /**
     * @brief 解锁无人机
     */
    void arm();

    /**
     * @brief 获取GPS位置
     * @return GPS位置
     */
    QGpsPosition gpsPosition() const { return m_gpsPosition; }

    /**
     * @brief 设置GPS位置
     * @param position GPS位置
     */
    void setGpsPosition(const QGpsPosition &position);

    /**
     * @brief 获取NED位置
     * @return NED位置
     */
    QNEDPosition nedPosition() const { return m_nedPosition; }

    /**
     * @brief 设置NED位置
     * @param position NED位置
     */
    void setNedPosition(const QNEDPosition &position);

    /**
     * @brief 获取Home位置
     * @return Home位置
     */
    QGpsPosition homePosition() const { return m_homePosition; }

    /**
     * @brief 设置Home位置
     * @param position Home位置
     */
    void setHomePosition(const QGpsPosition &position);

    /**
     * @brief 获取状态信息
     * @return 状态信息
     */
    QAutopilotStatus status() const { return m_status; }

    /**
     * @brief 设置状态信息
     * @param status 状态信息
     */
    void setStatus(const QAutopilotStatus &status);

    /**
     * @brief 获取航向角
     * @return 航向角（度）
     */
    double heading() const { return m_heading; }

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

protected slots:
    void positionUpdate(double dLon, double dLat, float dH);
    void nedUpdate(float dNorth, float dEast, float dDown);
    void gpsInfoUpdate(int gpsCount, int gpsStatus);
    void batteryUpdate(float batteryVoltage, float batteryRemaining);
    void rcStatusUpdate(bool isAvailable, float signalStrengthPercent);
    void headingUpdate(double heading);
    void healthUpdate(bool isGyrometerCalibrationOk, bool isAccelerometerCalibrationOk,
                      bool isMagnetometerCalibrationOk, bool isLocalPositionOk,
                      bool isGlobalPositionOk, bool isHomePositionOk, bool isArmable);
    void homeUpdate(double dLon,double dLat,float dH);

protected:

    friend class QAutopilotPrivate;
    /**
     * @brief 获取QAutopilotPrivate指针的辅助方法
     * @return QAutopilotPrivate指针
     */
    QAutopilotPrivate* d_func();
    const QAutopilotPrivate* d_func() const;

    QGpsPosition m_gpsPosition;
    QNEDPosition m_nedPosition;
    QGpsPosition m_homePosition;
    QAutopilotStatus m_status;
    double m_heading{0.0};  ///< 航向角（度）
    QAutoVehicleType::Vehicle m_vehicleType{QAutoVehicleType::Vehicle_Unknown};  ///< 载具类型
    QAutoVehicleType::Autopilot m_autopilotType{QAutoVehicleType::Autopilot_Unknown};  ///< 自动驾驶仪类型
};

#endif // _YTY_QAUTOPILOT_H
