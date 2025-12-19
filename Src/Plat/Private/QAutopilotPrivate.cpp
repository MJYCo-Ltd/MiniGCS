#include <QDebug>
#include <QMetaObject>
#include "Plat/Private/QAutopilotPrivate.h"

QAutopilotPrivate::QAutopilotPrivate() {}

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
}

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
template <> struct fmt::formatter<mavsdk::Telemetry::Result> : ostream_formatter {};

void QAutopilotPrivate::setTelemetryRate(QObject *parent) {
    /// 设置通信频率
    m_telemetry->set_rate_position_async(1,[&](mavsdk::Telemetry::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} set_rate_position_async:{}", m_pSystem->get_system_id(),
                      reqult);
    });

    m_telemetry->set_rate_gps_info_async(1,[&](mavsdk::Telemetry::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} set_rate_gps_info_async:{}", m_pSystem->get_system_id(),
                      reqult);
    });

    m_telemetry->set_rate_battery_async(1,[&](mavsdk::Telemetry::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} set_rate_battery_async:{}", m_pSystem->get_system_id(),
                      reqult);
    });

    m_telemetry->set_rate_health_async(1,[&](mavsdk::Telemetry::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} set_rate_health_async:{}", m_pSystem->get_system_id(),
                      reqult);
    });

    m_telemetry->set_rate_imu_async(1,[&](mavsdk::Telemetry::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} set_rate_imu_async:{}", m_pSystem->get_system_id(),
                      reqult);
    });

    m_telemetry->set_rate_distance_sensor_async(1,[&](mavsdk::Telemetry::Result reqult) {
        spdlog::debug("[mavsdk] systemid={} set_rate_distance_sensor_async:{}", m_pSystem->get_system_id(),
                      reqult);
    });
}

template <> struct fmt::formatter<mavsdk::Telemetry::Position> : ostream_formatter {};
template <> struct fmt::formatter<mavsdk::Telemetry::Heading> : ostream_formatter {};
template <> struct fmt::formatter<mavsdk::Telemetry::Battery> : ostream_formatter {};
template <> struct fmt::formatter<mavsdk::Telemetry::FlightMode> : ostream_formatter {};
template <> struct fmt::formatter<mavsdk::Telemetry::Health> : ostream_formatter {};
template <> struct fmt::formatter<mavsdk::Telemetry::GpsInfo> : ostream_formatter {};
template <> struct fmt::formatter<mavsdk::Telemetry::Imu> : ostream_formatter {};
template <> struct fmt::formatter<mavsdk::Telemetry::PositionVelocityNed> : ostream_formatter {};
template <> struct fmt::formatter<mavsdk::Telemetry::DistanceSensor> : ostream_formatter {};

void QAutopilotPrivate::setupMessageHandling(QObject *parent) {
    if (!m_telemetry || !parent) {
        return;
    }
    QPlatPrivate::setupMessageHandling(parent);
    setTelemetryRate(parent);

    /// 位置信息
    m_telemetry->subscribe_position([this, parent](mavsdk::Telemetry::Position position) {
        spdlog::info("[Position] systemid={} {}",m_pSystem->get_system_id(),position);
        
        // 通过Qt元系统调用parent的positionUpdate方法
        QMetaObject::invokeMethod(parent, "positionUpdate", Qt::AutoConnection,
                                  position.longitude_deg,
                                  position.latitude_deg,
                                  position.absolute_altitude_m);
    });

    /// 航向
    m_telemetry->subscribe_heading([&](mavsdk::Telemetry::Heading heading) {
        spdlog::info("[Heading] systemid={} {}", m_pSystem->get_system_id(),
                      heading);
    });

    /// 电池状态
    m_telemetry->subscribe_battery([&](mavsdk::Telemetry::Battery battery) {
        spdlog::info("[Battery] systemid={} {}", m_pSystem->get_system_id(),
                      battery);
    });

    /// 飞行状态
    m_telemetry->subscribe_flight_mode(
        [&](mavsdk::Telemetry::FlightMode flightMode) {
        spdlog::info("[FlightMode] systemid={} {}", m_pSystem->get_system_id(),
                          flightMode);
        });

    /// 健康状态
    m_telemetry->subscribe_health([&](mavsdk::Telemetry::Health h) {
        spdlog::info("[Health] systemid={} {}", m_pSystem->get_system_id(), h);
    });

    /// GPS状态
    m_telemetry->subscribe_gps_info([&](mavsdk::Telemetry::GpsInfo gps) {
        spdlog::info("[GpsInfo] systemid={} {}", m_pSystem->get_system_id(), gps);
    });

    /// 陀螺仪状态
    m_telemetry->subscribe_imu([&](mavsdk::Telemetry::Imu imu) {
        spdlog::info("[Imu] systemid={} {}", m_pSystem->get_system_id(), imu);
    });

    /// 本地坐标
    m_telemetry->subscribe_position_velocity_ned([this, parent](mavsdk::Telemetry::PositionVelocityNed pvNed) {
        spdlog::info("[PositionVelocityNed] systemid={} {}",
                      m_pSystem->get_system_id(),pvNed);
        
        // 通过Qt元系统调用parent的nedUpdate方法
        QMetaObject::invokeMethod(parent, "nedUpdate", Qt::AutoConnection,
                                  pvNed.position.north_m,
                                  pvNed.position.east_m,
                                  pvNed.position.down_m);
    });

    /// 距离传感器
    m_telemetry->subscribe_distance_sensor([&](mavsdk::Telemetry::DistanceSensor sensor) {
        spdlog::info("[DistanceSensor] systemid={} {}", m_pSystem->get_system_id(),
                      sensor);
    });

    m_telemetry->subscribe_home([&](mavsdk::Telemetry::Position home){
        spdlog::info("[Home] systemid={} {}", m_pSystem->get_system_id(),
                      home);
    });
}
