#ifndef Q_SUTOPILOT_PRIVATE_H
#define Q_SUTOPILOT_PRIVATE_H

#include "QPlatPrivate.h"


/**
 * @brief QAutopilot的私有实现类
 * 
 * 该类封装了QAutopilot的所有MAVSDK相关实现细节，
 * 使用PIMPL模式隐藏实现细节
 */
class QAutopilotPrivate:public QPlatPrivate
{
public:
    QAutopilotPrivate();
    ~QAutopilotPrivate();

    /**
     * @brief 设置自动驾驶仪类型
     * @param autopilotType 自动驾驶仪类型
     */
    void setAutopilotType(const QString &autopilotType);

    /**
     * @brief 获取自动驾驶仪类型
     * @return 自动驾驶仪类型
     */
    QString getAutopilotType() const;

    /**
     * @brief 设置载具类型
     * @param vehicleType 载具类型
     */
    void setVehicleType(const QString &vehicleType);

    /**
     * @brief 获取载具类型
     * @return 载具类型
     */
    QString getVehicleType() const;

    void setSystem(std::shared_ptr<mavsdk::System> system);
    void setupMessageHandling(QObject* parent);

    void setTelemetryRate(QObject* parent);
protected:
    QString m_autopilotType;                ///< 自动驾驶仪类型
    QString m_vehicleType;                  ///< 载具类型
    std::unique_ptr<mavsdk::Telemetry> m_telemetry; ///< 遥测插件
};

#endif // Q_SUTOPILOT_PRIVATE_H
