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

    void setSystem(std::shared_ptr<mavsdk::System> system);
    void setupMessageHandling();

    void setTelemetryRate();

    void downloadAirLine(quint64 requestId);

protected:
    /**
     * @brief 获取QAutopilotPrivate指针的辅助方法
     * @return QAutopilotPrivate指针
     */
    class QAutopilot* q_func();
    const  class QAutopilot* q_func() const;

    std::unique_ptr<mavsdk::Telemetry> m_telemetry; ///< 遥测插件
    std::unique_ptr<mavsdk::Action>    m_action;
    std::unique_ptr<mavsdk::Mission>   m_mission; /// 任务
};

#endif // Q_SUTOPILOT_PRIVATE_H
