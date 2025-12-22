#ifndef _YTY_QAUTOPILOTSTATUS_H
#define _YTY_QAUTOPILOTSTATUS_H

#include <QObject>
#include <QMetaType>
#include <QString>
#include "MiniGCSExport.h"

/**
 * @brief 自动驾驶仪状态信息结构体
 * 
 * 包含GPS信息、电池信息、遥控器连接状态等
 */
class MINIGCS_EXPORT QAutopilotStatus
{
    Q_GADGET
    Q_PROPERTY(int gpsCount READ gpsCount)
    Q_PROPERTY(QString gpsStatus READ gpsStatus)
    Q_PROPERTY(float batteryVoltage READ batteryVoltage)
    Q_PROPERTY(float batteryRemaining READ batteryRemaining)
    Q_PROPERTY(bool isGyrometerCalibrationOk READ isGyrometerCalibrationOk)
    Q_PROPERTY(bool isAccelerometerCalibrationOk READ isAccelerometerCalibrationOk)
    Q_PROPERTY(bool isMagnetometerCalibrationOk READ isMagnetometerCalibrationOk)
    Q_PROPERTY(bool isLocalPositionOk READ isLocalPositionOk)
    Q_PROPERTY(bool isGlobalPositionOk READ isGlobalPositionOk)
    Q_PROPERTY(bool isHomePositionOk READ isHomePositionOk)
    Q_PROPERTY(bool isArmable READ isArmable)
    Q_PROPERTY(bool rcIsAvailable READ rcIsAvailable)
    Q_PROPERTY(float rcSignalStrengthPercent READ rcSignalStrengthPercent)

public:
    QAutopilotStatus();
    QAutopilotStatus(int gpsCount, const QString &gpsStatus, float batteryVoltage, 
                     float batteryRemaining, bool rcConnected);
    
    int gpsCount() const { return m_gpsCount; }
    
    QString gpsStatus() const { return m_gpsStatus; }
    
    float batteryVoltage() const { return m_batteryVoltage; }
    
    float batteryRemaining() const { return m_batteryRemaining; }
    
    bool isGyrometerCalibrationOk() const { return m_isGyrometerCalibrationOk; }
    
    bool isAccelerometerCalibrationOk() const { return m_isAccelerometerCalibrationOk; }
    
    bool isMagnetometerCalibrationOk() const { return m_isMagnetometerCalibrationOk; }
    
    bool isLocalPositionOk() const { return m_isLocalPositionOk; }
    
    bool isGlobalPositionOk() const { return m_isGlobalPositionOk; }
    
    bool isHomePositionOk() const { return m_isHomePositionOk; }
    
    bool isArmable() const { return m_isArmable; }
    
    bool rcIsAvailable() const { return m_rcIsAvailable; }
    
    float rcSignalStrengthPercent() const { return m_rcSignalStrengthPercent; }
    
    bool operator==(const QAutopilotStatus &other) const;
    bool operator!=(const QAutopilotStatus &other) const;

private:
    // 仅允许 QAutopilot 更新状态
    friend class QAutopilot;

    void setGpsCount(int gpsCount);
    void setGpsStatus(const QString &gpsStatus);
    void setBatteryVoltage(float batteryVoltage);
    void setBatteryRemaining(float batteryRemaining);
    void setGyrometerCalibrationOk(bool isOk);
    void setAccelerometerCalibrationOk(bool isOk);
    void setMagnetometerCalibrationOk(bool isOk);
    void setLocalPositionOk(bool isOk);
    void setGlobalPositionOk(bool isOk);
    void setHomePositionOk(bool isOk);
    void setArmable(bool isArmable);
    void setRcIsAvailable(bool isAvailable);
    void setRcSignalStrengthPercent(float signalStrengthPercent);

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

