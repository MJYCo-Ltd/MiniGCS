#ifndef _YTY_QAUTOPILOTSTATUS_H
#define _YTY_QAUTOPILOTSTATUS_H

#include <QObject>
#include <QMetaType>
#include <QString>

/**
 * @brief 自动驾驶仪状态信息结构体
 * 
 * 包含GPS信息、电池信息、遥控器连接状态等
 */
class QAutopilotStatus
{
    Q_GADGET
    Q_PROPERTY(int gpsCount READ gpsCount WRITE setGpsCount)
    Q_PROPERTY(QString gpsStatus READ gpsStatus WRITE setGpsStatus)
    Q_PROPERTY(float batteryVoltage READ batteryVoltage WRITE setBatteryVoltage)
    Q_PROPERTY(float batteryRemaining READ batteryRemaining WRITE setBatteryRemaining)
    Q_PROPERTY(bool isGyrometerCalibrationOk READ isGyrometerCalibrationOk WRITE setGyrometerCalibrationOk)
    Q_PROPERTY(bool isAccelerometerCalibrationOk READ isAccelerometerCalibrationOk WRITE setAccelerometerCalibrationOk)
    Q_PROPERTY(bool isMagnetometerCalibrationOk READ isMagnetometerCalibrationOk WRITE setMagnetometerCalibrationOk)
    Q_PROPERTY(bool isLocalPositionOk READ isLocalPositionOk WRITE setLocalPositionOk)
    Q_PROPERTY(bool isGlobalPositionOk READ isGlobalPositionOk WRITE setGlobalPositionOk)
    Q_PROPERTY(bool isHomePositionOk READ isHomePositionOk WRITE setHomePositionOk)
    Q_PROPERTY(bool isArmable READ isArmable WRITE setArmable)
    Q_PROPERTY(bool rcIsAvailable READ rcIsAvailable WRITE setRcIsAvailable)
    Q_PROPERTY(float rcSignalStrengthPercent READ rcSignalStrengthPercent WRITE setRcSignalStrengthPercent)

public:
    QAutopilotStatus();
    QAutopilotStatus(int gpsCount, const QString &gpsStatus, float batteryVoltage, 
                     float batteryRemaining, bool rcConnected);
    
    int gpsCount() const { return m_gpsCount; }
    void setGpsCount(int gpsCount);
    
    QString gpsStatus() const { return m_gpsStatus; }
    void setGpsStatus(const QString &gpsStatus);
    
    float batteryVoltage() const { return m_batteryVoltage; }
    void setBatteryVoltage(float batteryVoltage);
    
    float batteryRemaining() const { return m_batteryRemaining; }
    void setBatteryRemaining(float batteryRemaining);
    
    bool isGyrometerCalibrationOk() const { return m_isGyrometerCalibrationOk; }
    void setGyrometerCalibrationOk(bool isOk);
    
    bool isAccelerometerCalibrationOk() const { return m_isAccelerometerCalibrationOk; }
    void setAccelerometerCalibrationOk(bool isOk);
    
    bool isMagnetometerCalibrationOk() const { return m_isMagnetometerCalibrationOk; }
    void setMagnetometerCalibrationOk(bool isOk);
    
    bool isLocalPositionOk() const { return m_isLocalPositionOk; }
    void setLocalPositionOk(bool isOk);
    
    bool isGlobalPositionOk() const { return m_isGlobalPositionOk; }
    void setGlobalPositionOk(bool isOk);
    
    bool isHomePositionOk() const { return m_isHomePositionOk; }
    void setHomePositionOk(bool isOk);
    
    bool isArmable() const { return m_isArmable; }
    void setArmable(bool isArmable);
    
    bool rcIsAvailable() const { return m_rcIsAvailable; }
    void setRcIsAvailable(bool isAvailable);
    
    float rcSignalStrengthPercent() const { return m_rcSignalStrengthPercent; }
    void setRcSignalStrengthPercent(float signalStrengthPercent);
    
    bool operator==(const QAutopilotStatus &other) const;
    bool operator!=(const QAutopilotStatus &other) const;

private:
    int m_gpsCount{0};              ///< GPS数量
    QString m_gpsStatus;              ///< GPS状态
    float m_batteryVoltage{0.0f};   ///< 电池电压（伏特）
    float m_batteryRemaining{0.0f};  ///< 电池电量（百分比，0-100）
    bool m_isGyrometerCalibrationOk{false};      ///< 陀螺仪是否已校准
    bool m_isAccelerometerCalibrationOk{false};  ///< 加速度计是否已校准
    bool m_isMagnetometerCalibrationOk{false};   ///< 磁力计是否已校准
    bool m_isLocalPositionOk{false};             ///< 本地位置估计是否足够好，可用于位置控制模式
    bool m_isGlobalPositionOk{false};            ///< 全局位置估计是否足够好，可用于位置控制模式
    bool m_isHomePositionOk{false};              ///< 家位置是否已正确初始化
    bool m_isArmable{false};                     ///< 系统是否可以解锁
    bool m_rcIsAvailable{false};                 ///< 遥控器当前是否可用
    float m_rcSignalStrengthPercent{0.0f};      ///< 遥控器信号强度百分比（0-100）
};

Q_DECLARE_METATYPE(QAutopilotStatus)

#endif // _YTY_QAUTOPILOTSTATUS_H

