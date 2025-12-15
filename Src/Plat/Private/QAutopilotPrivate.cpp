
#include <QDebug>
#include "Private/QAutopilotPrivate.h"

QAutopilotPrivate::QAutopilotPrivate()
{
}

QAutopilotPrivate::~QAutopilotPrivate()
{
}

void QAutopilotPrivate::setAutopilotType(const QString &autopilotType)
{
    m_autopilotType = autopilotType;
}

QString QAutopilotPrivate::getAutopilotType() const
{
    return m_autopilotType;
}

void QAutopilotPrivate::setVehicleType(const QString &vehicleType)
{
    m_vehicleType = vehicleType;
}

QString QAutopilotPrivate::getVehicleType() const
{
    return m_vehicleType;
}

void QAutopilotPrivate::setSystem(std::shared_ptr<mavsdk::System> system)
{
    QPlatPrivate::setSystem(system);
    m_telemetry = std::make_unique<mavsdk::Telemetry>(*system);
}

void QAutopilotPrivate::setTelemetryRate(QObject* parent)
{
    /// 设置通信频率
    m_telemetry->set_rate_position_async(1, [](mavsdk::Telemetry::Result reqult) {
    });
    m_telemetry->set_rate_gps_info_async(1, [](mavsdk::Telemetry::Result reqult) {
    });
    m_telemetry->set_rate_battery_async(1, [](mavsdk::Telemetry::Result reqult) {
    });
    m_telemetry->set_rate_health_async(1, [](mavsdk::Telemetry::Result reqult) {
    });
}

void QAutopilotPrivate::setupMessageHandling(QObject *parent) {
    if (!m_telemetry || !parent) {
        return;
    }
    QPlatPrivate::setupMessageHandling(parent);

    // 订阅位置信息
    m_telemetry->subscribe_position(
        [parent](mavsdk::Telemetry::Position position) {
        std::ostringstream oss;
        oss << position;
        qDebug() << "position:" << oss.str();
        });

    // 订阅姿态信息
    m_telemetry->subscribe_attitude_euler(
        [parent](mavsdk::Telemetry::EulerAngle attitude) {
        // 这里可以发射姿态更新信号
        // QMetaObject::invokeMethod(parent, "attitudeUpdated",
        // Qt::QueuedConnection, ...);
        });

    // 订阅电池状态
    m_telemetry->subscribe_battery([parent](mavsdk::Telemetry::Battery battery) {
        // 这里可以发射电池状态更新信号
        std::ostringstream oss;
        oss << battery;
        qDebug() << "battery:" << oss.str();
        // QMetaObject::invokeMethod(parent, "batteryUpdated", Qt::QueuedConnection,
        // ...);
    });

    // 订阅飞行状态
    m_telemetry->subscribe_flight_mode(
        [parent](mavsdk::Telemetry::FlightMode flightMode) {
        // std::ostringstream oss;
        // oss << flightMode;
        // qDebug() << "flightMode:" << oss.str();
        // 这里可以发射飞行状态更新信号
        // QMetaObject::invokeMethod(parent, "flightModeUpdated",
        // Qt::QueuedConnection, ...);
        });

    m_telemetry->subscribe_health([](mavsdk::Telemetry::Health h) {
        std::ostringstream oss;
        oss << h;
        qDebug() << "Health:" << oss.str();
    });
    m_telemetry->subscribe_gps_info([](mavsdk::Telemetry::GpsInfo gps) {
        std::ostringstream oss;
        oss << gps;
        qDebug() << "GpsInfo:" << oss.str();
    });
}
