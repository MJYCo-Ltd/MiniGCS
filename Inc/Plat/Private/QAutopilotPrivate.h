#ifndef Q_SUTOPILOT_PRIVATE_H
#define Q_SUTOPILOT_PRIVATE_H

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
    QString m_autopilotType;                ///< 自动驾驶仪类型
    QString m_vehicleType;                  ///< 载具类型
    std::unique_ptr<mavsdk::Telemetry> m_telemetry; ///< 遥测插件
    mavsdk::Telemetry::Imu m_lastImu;      ///< 最后一次IMU数据
    
    // 位置滤波相关
    FilteredPosition m_filteredState;       ///< 滤波状态
    
    static constexpr double Q_move{0.5};    ///< 运动噪声
    static constexpr double Q_static{0.01}; ///< 静止噪声
    static constexpr double R_gps{4.0};     ///< GPS 测量噪声（约 2m²）
};

#endif // Q_SUTOPILOT_PRIVATE_H
