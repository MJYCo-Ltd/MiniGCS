#ifndef Q_SUTOPILOT_PRIVATE_H
#define Q_SUTOPILOT_PRIVATE_H

#include <mavsdk/plugins/action/action.h>
#include "QPlatPrivate.h"

/**
 * @brief 滤波后的位置数据结构
 */
struct FilteredPosition {
    double lat;
    double lon;
    double P;   // 协方差（简化为标量）
};

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
    ~QAutopilotPrivate(){}

    /**
     * @brief 解锁
     */
    void arm();

    void setSystem(std::shared_ptr<mavsdk::System> system);
    void setupMessageHandling();

    void setTelemetryRate();

private:
    /**
     * @brief 判断是否静止（IMU数值较小时）
     * @param imu IMU数据
     * @return 是否静止
     */
    bool isStatic(const mavsdk::Telemetry::Imu& imu) const;

    /**
     * @brief 更新位置滤波器
     * @param state 滤波状态
     * @param gps_lat GPS纬度
     * @param gps_lon GPS经度
     * @param is_static 是否静止
     */
    void updateFilter(FilteredPosition& state,
                      double gps_lat,
                      double gps_lon,
                      bool is_static) const;

protected:
    /**
     * @brief 获取QAutopilotPrivate指针的辅助方法
     * @return QAutopilotPrivate指针
     */
    class QAutopilot* q_func();
    const  class QAutopilot* q_func() const;

    std::unique_ptr<mavsdk::Telemetry> m_telemetry; ///< 遥测插件
    std::unique_ptr<mavsdk::Action>    m_action;
    mavsdk::Telemetry::Imu m_lastImu;      ///< 最后一次IMU数据
    
    // 位置滤波相关
    FilteredPosition m_filteredState;       ///< 滤波状态
    
    static constexpr double Q_move{0.5};    ///< 运动噪声
    static constexpr double Q_static{0.01}; ///< 静止噪声
    static constexpr double R_gps{4.0};     ///< GPS 测量噪声（约 2m²）
};

#endif // Q_SUTOPILOT_PRIVATE_H
