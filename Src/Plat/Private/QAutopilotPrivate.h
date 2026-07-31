#ifndef Q_SUTOPILOT_PRIVATE_H
#define Q_SUTOPILOT_PRIVATE_H

#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mission/mission.h>
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
    QAutopilotPrivate(QPlat*pPlat);
    ~QAutopilotPrivate() override;

    /**
     * @brief 解锁
     */
    void arm();
    void disarm();
    void takeoff();
    void land();
    void returnToLaunch();

    void setSystem(std::shared_ptr<mavsdk::System> system) override;
    void setupMessageHandling() override;

    void setTelemetryRate();

    void downloadAirLine(quint64 requestId);

protected:
    void clearTelemetrySubscriptions();

    /**
     * @brief 获取QAutopilotPrivate指针的辅助方法
     * @return QAutopilotPrivate指针
     */
    class QAutopilot* q_func();
    const  class QAutopilot* q_func() const;

    std::unique_ptr<mavsdk::Telemetry> m_telemetry; ///< 遥测插件
    std::unique_ptr<mavsdk::Action>    m_action;
    std::unique_ptr<mavsdk::Mission>   m_mission; /// 任务

    mavsdk::Telemetry::PositionHandle m_positionHandle;
    mavsdk::Telemetry::HeadingHandle m_headingHandle;
    mavsdk::Telemetry::BatteryHandle m_batteryHandle;
    mavsdk::Telemetry::FlightModeHandle m_flightModeHandle;
    mavsdk::Telemetry::HealthHandle m_healthHandle;
    mavsdk::Telemetry::GpsInfoHandle m_gpsInfoHandle;
    mavsdk::Telemetry::PositionVelocityNedHandle m_positionVelocityHandle;
    mavsdk::Telemetry::ArmedHandle m_armedHandle;
    mavsdk::Telemetry::InAirHandle m_inAirHandle;
    mavsdk::Telemetry::DistanceSensorHandle m_distanceSensorHandle;
    mavsdk::Telemetry::HomeHandle m_homeHandle;
    mavsdk::Telemetry::RcStatusHandle m_rcStatusHandle;
    mavsdk::Telemetry::FixedwingMetricsHandle m_fixedwingMetricsHandle;
};

#endif // Q_SUTOPILOT_PRIVATE_H
