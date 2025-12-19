#include "Plat/Private/QAutopilotPrivate.h"
#include <QDebug>
#include <QMetaObject>
#include <cmath>

QAutopilotPrivate::QAutopilotPrivate() 
    : m_filteredState{0.0, 0.0, 10.0}  // 初始不确定度（米级）
{
}

QAutopilotPrivate::~QAutopilotPrivate() {}


void QAutopilotPrivate::setAutopilotType(const QString &autopilotType) {
    m_autopilotType = autopilotType;
}

QString QAutopilotPrivate::getAutopilotType() const { return m_autopilotType; }

void QAutopilotPrivate::setVehicleType(const QString &vehicleType) {
    m_vehicleType = vehicleType;
}

QString QAutopilotPrivate::getVehicleType() const { return m_vehicleType; }

/// 设置mavsdk的飞控系统
void QAutopilotPrivate::setSystem(std::shared_ptr<mavsdk::System> system) {
    QPlatPrivate::setSystem(system);
    m_telemetry = std::make_unique<mavsdk::Telemetry>(*system);
    m_action = std::make_unique<mavsdk::Action>(*system);
    arm();
}

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
template<>struct fmt::formatter<mavsdk::Action::Result>:ostream_formatter{};

void QAutopilotPrivate::arm()
{
    m_action->arm_async([this](mavsdk::Action::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} arm_async:{}",
                      m_pSystem->get_system_id(), reqult);
    });
}

template<>struct fmt::formatter<mavsdk::Telemetry::Result>:ostream_formatter{};

void QAutopilotPrivate::setTelemetryRate(QObject *parent) {
    /// 设置 位置信息 频率
    m_telemetry->set_rate_position_async(
        1, [this](mavsdk::Telemetry::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} set_rate_position_async:{}",
                          m_pSystem->get_system_id(), reqult);
        });

    m_telemetry->set_rate_position_velocity_ned_async(
        1, [this](mavsdk::Telemetry::Result reqult) {
        spdlog::debug(
                "[mavsdk] systemid={} set_rate_position_velocity_ned_async:{}",
                m_pSystem->get_system_id(), reqult);
        });

    /// 设置 gps 状态 发送频率
    m_telemetry->set_rate_gps_info_async(
        1, [this](mavsdk::Telemetry::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} set_rate_gps_info_async:{}",
                      m_pSystem->get_system_id(), reqult);
        });

    /// 设置 电池信息 发送频率
    m_telemetry->set_rate_battery_async(1, [this](mavsdk::Telemetry::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} set_rate_battery_async:{}",
                      m_pSystem->get_system_id(), reqult);
    });

    /// 设置 健康度 发送频率
    m_telemetry->set_rate_health_async(
        0.5, [this](mavsdk::Telemetry::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} set_rate_health_async:{}",
                      m_pSystem->get_system_id(), reqult);
        });

    /// 设置 陀螺仪 发送频率
    m_telemetry->set_rate_imu_async(1, [this](mavsdk::Telemetry::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} set_rate_imu_async:{}",
                      m_pSystem->get_system_id(), reqult);
    });

    /// 设置 home 发送频率
    m_telemetry->set_rate_home_async(0.1, [this](mavsdk::Telemetry::Result reqult) {
            spdlog::debug("[mavsdk] systemid={} set_rate_home_async:{}",
                          m_pSystem->get_system_id(), reqult);
        });

    /// 设置 距离传感器 发送频率
    m_telemetry->set_rate_distance_sensor_async(
        1, [this](mavsdk::Telemetry::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} set_rate_distance_sensor_async:{}",
                      m_pSystem->get_system_id(), reqult);
        });

    /// 设置 遥控器状态 发送频率 Unsupported and System status is usually fixed at 1 Hz
    // m_telemetry->set_rate_rc_status_async(
    //     0.2, [this](mavsdk::Telemetry::Result reqult) {
    //     spdlog::debug("[mavsdk] systemid={} set_rate_rc_status_async:{}",
    //                   m_pSystem->get_system_id(), reqult);
    //     });
}

template<>struct fmt::formatter<mavsdk::Telemetry::Position>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Telemetry::Heading>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Telemetry::Battery>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Telemetry::FlightMode>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Telemetry::Health>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Telemetry::GpsInfo>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Telemetry::Imu>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Telemetry::PositionVelocityNed>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Telemetry::DistanceSensor>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Telemetry::RcStatus>:ostream_formatter{};

void QAutopilotPrivate::setupMessageHandling(QObject *parent) {
    if (!m_telemetry || !parent) {
        return;
    }

    QPlatPrivate::setupMessageHandling(parent);
    setTelemetryRate(parent);

    /// 位置信息
    m_telemetry->subscribe_position([this, parent](
                                        mavsdk::Telemetry::Position position) {
        spdlog::info("[Position] systemid={} {}", m_pSystem->get_system_id(),
                     position);

        // 当IMU数值较小时（静止状态）对GPS数据进行滤波
        bool static_state = isStatic(m_lastImu);
        if (static_state) {
            updateFilter(m_filteredState,
                         position.latitude_deg,
                         position.longitude_deg,
                         static_state);
            
            // 使用滤波后的位置
            QMetaObject::invokeMethod(parent, "positionUpdate", Qt::QueuedConnection,
                                      Q_ARG(double, m_filteredState.lon),
                                      Q_ARG(double, m_filteredState.lat),
                                      Q_ARG(float, position.absolute_altitude_m));
        } else {
            // 运动状态直接使用原始GPS数据
            QMetaObject::invokeMethod(parent, "positionUpdate", Qt::QueuedConnection,
                                      Q_ARG(double, position.longitude_deg),
                                      Q_ARG(double, position.latitude_deg),
                                      Q_ARG(float, position.absolute_altitude_m));
            
            // 更新滤波状态（但不使用滤波结果）
            updateFilter(m_filteredState,
                         position.latitude_deg,
                         position.longitude_deg,
                         static_state);
        }
    });

    /// 航向
    m_telemetry->subscribe_heading(
        [this, parent](mavsdk::Telemetry::Heading heading) {
        QMetaObject::invokeMethod(parent, "headingUpdate", Qt::QueuedConnection,
                                  Q_ARG(double, heading.heading_deg));
        spdlog::info("[Heading] systemid={} {}", m_pSystem->get_system_id(),
                     heading);
        });

    /// 电池状态
    m_telemetry->subscribe_battery(
        [this, parent](mavsdk::Telemetry::Battery battery) {
        QMetaObject::invokeMethod(parent, "batteryUpdate", Qt::QueuedConnection,
                                  Q_ARG(float, battery.voltage_v),
                                  Q_ARG(float, battery.remaining_percent));
        spdlog::info("[Battery] systemid={} {}", m_pSystem->get_system_id(),
                     battery);
        });

    /// 飞行状态
    m_telemetry->subscribe_flight_mode(
        [this](mavsdk::Telemetry::FlightMode flightMode) {
        spdlog::info("[FlightMode] systemid={} {}", m_pSystem->get_system_id(),
                         flightMode);
        });

    /// 健康状态
    m_telemetry->subscribe_health([this, parent](mavsdk::Telemetry::Health h) {
        QMetaObject::invokeMethod(parent, "healthUpdate", Qt::QueuedConnection,
                                  Q_ARG(bool, h.is_gyrometer_calibration_ok),
                                  Q_ARG(bool, h.is_accelerometer_calibration_ok),
                                  Q_ARG(bool, h.is_magnetometer_calibration_ok),
                                  Q_ARG(bool, h.is_local_position_ok),
                                  Q_ARG(bool, h.is_global_position_ok),
                                  Q_ARG(bool, h.is_home_position_ok),
                                  Q_ARG(bool, h.is_armable));
        spdlog::info("[Health] systemid={} {}", m_pSystem->get_system_id(), h);
    });

    /// GPS状态
    m_telemetry->subscribe_gps_info([this,
                                     parent](mavsdk::Telemetry::GpsInfo gps) {
        QMetaObject::invokeMethod(parent, "gpsInfoUpdate", Qt::QueuedConnection,
                                  Q_ARG(int, gps.num_satellites),
                                  Q_ARG(int, (int)gps.fix_type));
        spdlog::info("[GpsInfo] systemid={} {}", m_pSystem->get_system_id(), gps);
    });

    /// 陀螺仪状态
    m_telemetry->subscribe_imu([this](mavsdk::Telemetry::Imu imu) {
        spdlog::info("[Imu] systemid={} {}", m_pSystem->get_system_id(), imu);
        // 保存最后一次IMU数据用于滤波
        m_lastImu = imu;
    });

    /// 本地坐标
    m_telemetry->subscribe_position_velocity_ned(
        [this, parent](mavsdk::Telemetry::PositionVelocityNed pvNed) {
        spdlog::info("[PositionVelocityNed] systemid={} {}",
                         m_pSystem->get_system_id(), pvNed);

            // 通过Qt元系统调用parent的nedUpdate方法
        QMetaObject::invokeMethod(parent, "nedUpdate", Qt::QueuedConnection,
                                  Q_ARG(float, pvNed.position.north_m),
                                  Q_ARG(float, pvNed.position.east_m),
                                  Q_ARG(float, pvNed.position.down_m));
        });

    /// 距离传感器
    m_telemetry->subscribe_distance_sensor(
        [this](mavsdk::Telemetry::DistanceSensor sensor) {
        spdlog::info("[DistanceSensor] systemid={} {}",
                     m_pSystem->get_system_id(), sensor);
        });

    /// 订阅home点
    m_telemetry->subscribe_home([this, parent](mavsdk::Telemetry::Position home) {
        spdlog::info("[Home] systemid={} {}", m_pSystem->get_system_id(), home);
        
        // 通过Qt元系统调用parent的homeUpdate方法
        QMetaObject::invokeMethod(parent, "homeUpdate", Qt::QueuedConnection,
                                  Q_ARG(double, home.longitude_deg),
                                  Q_ARG(double, home.latitude_deg),
                                  Q_ARG(float, home.absolute_altitude_m));
    });

    /// 订阅 rc状态
    m_telemetry->subscribe_rc_status(
        [this, parent](mavsdk::Telemetry::RcStatus rcStatus) {
        spdlog::info("[RcStatus] systemid={} {}", m_pSystem->get_system_id(),
                     rcStatus);
        
        // 通过Qt元系统调用parent的rcStatusUpdate方法
        QMetaObject::invokeMethod(parent, "rcStatusUpdate", Qt::QueuedConnection,
                                  Q_ARG(bool, rcStatus.is_available),
                                  Q_ARG(float, rcStatus.signal_strength_percent));
        });
}

bool QAutopilotPrivate::isStatic(const mavsdk::Telemetry::Imu& imu) const
{
    double gyro_norm =
        std::hypot(imu.angular_velocity_frd.forward_rad_s,
                   imu.angular_velocity_frd.right_rad_s,
                   imu.angular_velocity_frd.down_rad_s);

    double accel_norm =
        std::hypot(imu.acceleration_frd.forward_m_s2,
                   imu.acceleration_frd.right_m_s2,
                   imu.acceleration_frd.down_m_s2);

    return gyro_norm < 0.02 &&
           std::abs(accel_norm - 9.81) < 0.1;
}

void QAutopilotPrivate::updateFilter(FilteredPosition& s,
                                     double gps_lat,
                                     double gps_lon,
                                     bool is_static) const
{
    // 预测
    s.P += is_static ? Q_static : Q_move;

    // 卡尔曼增益
    double K = s.P / (s.P + R_gps);

    // 静止时极大削弱 GPS
    if (is_static) {
        K *= 0.1;   // 关键：GPS 权重压低
    }

    // 更新
    s.lat += K * (gps_lat - s.lat);
    s.lon += K * (gps_lon - s.lon);

    s.P *= (1.0 - K);
}
