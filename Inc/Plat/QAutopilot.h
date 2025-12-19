#ifndef _YTY_QAUTOPILOT_H
#define _YTY_QAUTOPILOT_H

#include "Plat/QPlat.h"
#include "AirLine/QGpsPosition.h"
#include "AirLine/QNEDPosition.h"
#include "Plat/QAutopilotStatus.h"

/**
 * @brief QAutopilot类 - 具备自动驾驶功能的系统
 * 
 * 该类封装了自动驾驶功能的所有信息，包括连接状态、硬件信息等
 */
class QAutopilot : public QPlat
{
    Q_OBJECT
    Q_PROPERTY(QGpsPosition gpsPosition MEMBER m_gpsPosition NOTIFY gpsPositionChanged)
    Q_PROPERTY(QNEDPosition nedPosition MEMBER m_nedPosition NOTIFY nedPositionChanged)
    Q_PROPERTY(QGpsPosition homePosition MEMBER m_homePosition NOTIFY homePositionChanged)
    Q_PROPERTY(QAutopilotStatus status MEMBER m_status NOTIFY statusChanged)
    Q_PROPERTY(double heading MEMBER m_heading NOTIFY headingChanged)

public:
    explicit QAutopilot(QObject *parent = nullptr);
    ~QAutopilot();

    /**
     * @brief 获取自动驾驶仪类型
     * @return 自动驾驶仪类型
     */
    QString getAutopilotType() const;

    /**
     * @brief 设置自动驾驶仪类型
     * @param autopilotType 自动驾驶仪类型
     */
    void setAutopilotType(const QString &autopilotType);

    /**
     * @brief 获取载具类型
     * @return 载具类型
     */
    QString getVehicleType() const;

    /**
     * @brief 设置载具类型
     * @param vehicleType 载具类型
     */
    void setVehicleType(const QString &vehicleType);


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
};

#endif // _YTY_QAUTOPILOT_H
