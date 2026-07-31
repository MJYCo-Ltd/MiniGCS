#include "Plat/QAutopilot.h"
#include "Plat/Private/QAutopilotPrivate.h"
#include <QPointer>

QAutopilotPrivate::QAutopilotPrivate(QPlat *pPlat)
    : QPlatPrivate(pPlat)
{}

QAutopilotPrivate::~QAutopilotPrivate()
{
    if (m_mission) {
        m_mission->cancel_mission_download();
        m_mission->cancel_mission_upload();
    }
    clearTelemetrySubscriptions();
    m_mission.reset();
    m_action.reset();
    m_telemetry.reset();
    QPlatPrivate::setSystem(nullptr);
}

QAutopilot *QAutopilotPrivate::q_func() {
    return static_cast<QAutopilot *>(q_ptr);
}

const QAutopilot *QAutopilotPrivate::q_func() const {
    return static_cast<const QAutopilot *>(q_ptr);
}

/// 设置mavsdk的飞控系统
void QAutopilotPrivate::setSystem(std::shared_ptr<mavsdk::System> system) {
    if (m_mission) {
        m_mission->cancel_mission_download();
        m_mission->cancel_mission_upload();
        q_func()->cancelAirLineDownload();
        q_func()->cancelAirLineUpload();
    }
    clearTelemetrySubscriptions();
    m_mission.reset();
    m_action.reset();
    m_telemetry.reset();

    QPlatPrivate::setSystem(system);
    if (!system) {
        m_telemetry.reset();
        m_action.reset();
        m_mission.reset();
        return;
    }

    m_telemetry = std::make_unique<mavsdk::Telemetry>(*system);
    m_action = std::make_unique<mavsdk::Action>(*system);
    m_mission = std::make_unique<mavsdk::Mission>(*system);

    q_func()->setAutopilotType(static_cast<QAutoVehicleType::Autopilot>(system->autopilot_type()));
    q_func()->setVehicleType(static_cast<QAutoVehicleType::Vehicle>(system->vehicle_type()));

    // arm();
}

#include "Private/QGCSLog.h"

template<>struct fmt::formatter<mavsdk::Action::Result>:ostream_formatter{};

void QAutopilotPrivate::arm() {
    if (!m_action || !m_pSystem) {
        return;
    }
    const uint8_t systemId = m_pSystem->get_system_id();
    m_action->arm_async([systemId](mavsdk::Action::Result result) {
        if (mavsdk::Action::Result::Success != result) {
            spdlog::error(PLAT_FMT_STR, systemId, "arm", result);
        }
    });
}

void QAutopilotPrivate::clearTelemetrySubscriptions()
{
    if (!m_telemetry) {
        return;
    }

    if (m_positionHandle.valid()) {
        m_telemetry->unsubscribe_position(m_positionHandle);
        m_positionHandle = {};
    }
    if (m_headingHandle.valid()) {
        m_telemetry->unsubscribe_heading(m_headingHandle);
        m_headingHandle = {};
    }
    if (m_batteryHandle.valid()) {
        m_telemetry->unsubscribe_battery(m_batteryHandle);
        m_batteryHandle = {};
    }
    if (m_flightModeHandle.valid()) {
        m_telemetry->unsubscribe_flight_mode(m_flightModeHandle);
        m_flightModeHandle = {};
    }
    if (m_healthHandle.valid()) {
        m_telemetry->unsubscribe_health(m_healthHandle);
        m_healthHandle = {};
    }
    if (m_gpsInfoHandle.valid()) {
        m_telemetry->unsubscribe_gps_info(m_gpsInfoHandle);
        m_gpsInfoHandle = {};
    }
    if (m_positionVelocityHandle.valid()) {
        m_telemetry->unsubscribe_position_velocity_ned(
            m_positionVelocityHandle);
        m_positionVelocityHandle = {};
    }
    if (m_armedHandle.valid()) {
        m_telemetry->unsubscribe_armed(m_armedHandle);
        m_armedHandle = {};
    }
    if (m_inAirHandle.valid()) {
        m_telemetry->unsubscribe_in_air(m_inAirHandle);
        m_inAirHandle = {};
    }
    if (m_distanceSensorHandle.valid()) {
        m_telemetry->unsubscribe_distance_sensor(m_distanceSensorHandle);
        m_distanceSensorHandle = {};
    }
    if (m_homeHandle.valid()) {
        m_telemetry->unsubscribe_home(m_homeHandle);
        m_homeHandle = {};
    }
    if (m_rcStatusHandle.valid()) {
        m_telemetry->unsubscribe_rc_status(m_rcStatusHandle);
        m_rcStatusHandle = {};
    }
    if (m_fixedwingMetricsHandle.valid()) {
        m_telemetry->unsubscribe_fixedwing_metrics(
            m_fixedwingMetricsHandle);
        m_fixedwingMetricsHandle = {};
    }
}

void QAutopilotPrivate::disarm()
{
    if (!m_action || !m_pSystem) {
        return;
    }
    const uint8_t systemId = m_pSystem->get_system_id();
    m_action->disarm_async([systemId](mavsdk::Action::Result result) {
        if (mavsdk::Action::Result::Success != result) {
            spdlog::error(PLAT_FMT_STR, systemId, "disarm", result);
        }
    });
}

void QAutopilotPrivate::takeoff()
{
    if (!m_action || !m_pSystem) {
        return;
    }
    const uint8_t systemId = m_pSystem->get_system_id();
    m_action->takeoff_async([systemId](mavsdk::Action::Result result) {
        if (mavsdk::Action::Result::Success != result) {
            spdlog::error(PLAT_FMT_STR, systemId, "takeoff", result);
        }
    });
}

void QAutopilotPrivate::land()
{
    if (!m_action || !m_pSystem) {
        return;
    }
    const uint8_t systemId = m_pSystem->get_system_id();
    m_action->land_async([systemId](mavsdk::Action::Result result) {
        if (mavsdk::Action::Result::Success != result) {
            spdlog::error(PLAT_FMT_STR, systemId, "land", result);
        }
    });
}

void QAutopilotPrivate::returnToLaunch()
{
    if (!m_action || !m_pSystem) {
        return;
    }
    const uint8_t systemId = m_pSystem->get_system_id();
    m_action->return_to_launch_async(
        [systemId](mavsdk::Action::Result result) {
            if (mavsdk::Action::Result::Success != result) {
                spdlog::error(
                    PLAT_FMT_STR, systemId, "return_to_launch", result);
            }
        });
}

template<>struct fmt::formatter<mavsdk::Telemetry::Result>:ostream_formatter{};

void QAutopilotPrivate::setTelemetryRate() {
    if (!m_telemetry || !m_pSystem) {
        return;
    }
    const uint8_t systemId = m_pSystem->get_system_id();
    /// 设置 位置信息 频率
    m_telemetry->set_rate_position_async(
        1, [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_position", result);
            }
        });

    m_telemetry->set_rate_position_velocity_ned_async(
        1, [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_position_velocity_ned", result);
            }
        });

    /// 设置 gps 状态 发送频率
    m_telemetry->set_rate_gps_info_async(
        1, [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_gps_info", result);
            }
        });

    /// 设置 电池信息 发送频率
    m_telemetry->set_rate_battery_async(
        1, [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_battery", result);
            }
        });

    /// 设置 健康度 发送频率
    m_telemetry->set_rate_health_async(
        0.5, [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_health", result);
            }
        });

    /// 设置 home 发送频率
    m_telemetry->set_rate_home_async(
        0.1, [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_home", result);
            }
        });

    /// 设置空速
    m_telemetry->set_rate_fixedwing_metrics_async(1,[systemId](mavsdk::Telemetry::Result result) {
        if (mavsdk::Telemetry::Result::Success != result) {
            spdlog::error(PLAT_FMT_STR, systemId,
                          "set_rate_fixedwing_metrics", result);
        }
    });

    /// 设置 遥控器状态 发送频率 Unsupported and System status is usually fixed at
    /// 1 Hz
    // m_telemetry->set_rate_rc_status_async(
    //     0.2, [this](mavsdk::Telemetry::Result reqult) {
    //     spdlog::debug("[mavsdk] systemid={} set_rate_rc_status_async:{}",
    //                   m_pSystem->get_system_id(), reqult);
    //     });
}

void QAutopilotPrivate::setupMessageHandling() {
    if (!m_telemetry || !m_pSystem) {
        return;
    }

    clearTelemetrySubscriptions();
    QPlatPrivate::setupMessageHandling();
    const QPointer<QAutopilot> autopilot(q_func());
    const uint8_t systemId = m_pSystem->get_system_id();

    /// 位置信息
    m_positionHandle = m_telemetry->subscribe_position([autopilot](
                                        mavsdk::Telemetry::Position position) {
        if (autopilot) {
            QMetaObject::invokeMethod(
                autopilot, "positionUpdate", Qt::QueuedConnection,
                Q_ARG(double, position.longitude_deg),
                Q_ARG(double, position.latitude_deg),
                Q_ARG(float, position.absolute_altitude_m));
        }
    });

    /// 航向
    m_headingHandle = m_telemetry->subscribe_heading([autopilot](mavsdk::Telemetry::Heading heading) {
        if (autopilot) {
            QMetaObject::invokeMethod(
                autopilot, "headingUpdate", Qt::QueuedConnection,
                Q_ARG(double, heading.heading_deg));
        }
    });

    /// 电池状态
    m_batteryHandle = m_telemetry->subscribe_battery([autopilot](mavsdk::Telemetry::Battery battery) {
        if (autopilot) {
            QMetaObject::invokeMethod(
                autopilot, "batteryUpdate", Qt::QueuedConnection,
                Q_ARG(float, battery.voltage_v),
                Q_ARG(float, battery.remaining_percent));
        }
    });

    m_telemetry->set_rate_in_air_async(
        1, [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_in_air", result);
            }
        });

    /// 健康状态
    m_healthHandle = m_telemetry->subscribe_health([autopilot](mavsdk::Telemetry::Health h) {
        if (autopilot) {
            QMetaObject::invokeMethod(
                autopilot, "healthUpdate", Qt::QueuedConnection,
                Q_ARG(bool, h.is_gyrometer_calibration_ok),
                Q_ARG(bool, h.is_accelerometer_calibration_ok),
                Q_ARG(bool, h.is_magnetometer_calibration_ok),
                Q_ARG(bool, h.is_local_position_ok),
                Q_ARG(bool, h.is_global_position_ok),
                Q_ARG(bool, h.is_home_position_ok),
                Q_ARG(bool, h.is_armable));
        }
    });

    /// GPS状态
    m_gpsInfoHandle = m_telemetry->subscribe_gps_info(
        [autopilot](mavsdk::Telemetry::GpsInfo gps) {
        if (autopilot) {
            QMetaObject::invokeMethod(
                autopilot, "gpsInfoUpdate", Qt::QueuedConnection,
                Q_ARG(int, gps.num_satellites),
                Q_ARG(int, static_cast<int>(gps.fix_type)));
        }
        });

    /// 本地坐标
    m_positionVelocityHandle =
        m_telemetry->subscribe_position_velocity_ned(
        [autopilot](mavsdk::Telemetry::PositionVelocityNed pvNed) {
            // 通过Qt元系统调用parent的nedUpdate方法
            if (autopilot) {
                QMetaObject::invokeMethod(
                    autopilot, "nedUpdate", Qt::QueuedConnection,
                    Q_ARG(float, pvNed.position.north_m),
                    Q_ARG(float, pvNed.position.east_m),
                    Q_ARG(float, pvNed.position.down_m),
                    Q_ARG(float, pvNed.velocity.north_m_s),
                    Q_ARG(float, pvNed.velocity.east_m_s),
                    Q_ARG(float, pvNed.velocity.down_m_s));
            }
        });

    m_armedHandle = m_telemetry->subscribe_armed(
        [autopilot](bool armed) {
            if (autopilot) {
                QMetaObject::invokeMethod(
                    autopilot, "armedUpdate", Qt::QueuedConnection,
                    Q_ARG(bool, armed));
            }
        });

    m_inAirHandle = m_telemetry->subscribe_in_air(
        [autopilot](bool inAir) {
            if (autopilot) {
                QMetaObject::invokeMethod(
                    autopilot, "inAirUpdate", Qt::QueuedConnection,
                    Q_ARG(bool, inAir));
            }
        });

    /// 订阅home点
    m_homeHandle = m_telemetry->subscribe_home([autopilot](mavsdk::Telemetry::Position home) {
        // 通过Qt元系统调用parent的homeUpdate方法
        if (autopilot) {
            QMetaObject::invokeMethod(
                autopilot, "homeUpdate", Qt::QueuedConnection,
                Q_ARG(double, home.longitude_deg),
                Q_ARG(double, home.latitude_deg),
                Q_ARG(float, home.absolute_altitude_m));
        }
    });

    /// 订阅 rc状态
    m_rcStatusHandle = m_telemetry->subscribe_rc_status([autopilot](
                                         mavsdk::Telemetry::RcStatus rcStatus) {
        // 通过Qt元系统调用parent的rcStatusUpdate方法
        if (autopilot) {
            QMetaObject::invokeMethod(
                autopilot, "rcStatusUpdate", Qt::QueuedConnection,
                Q_ARG(bool, rcStatus.is_available),
                Q_ARG(float, rcStatus.signal_strength_percent));
        }
    });

    /// 订阅固定翼指标
    m_fixedwingMetricsHandle = m_telemetry->subscribe_fixedwing_metrics([autopilot](mavsdk::Telemetry::FixedwingMetrics fixMetrics){
        // 通过Qt元系统调用parent的fixedwingUpdate方法
        if (autopilot) {
            QMetaObject::invokeMethod(
                autopilot, "fixedwingUpdate", Qt::QueuedConnection,
                Q_ARG(float, fixMetrics.airspeed_m_s),
                Q_ARG(float, fixMetrics.throttle_percentage),
                Q_ARG(float, fixMetrics.climb_rate_m_s),
                Q_ARG(float, fixMetrics.groundspeed_m_s),
                Q_ARG(float, fixMetrics.heading_deg),
                Q_ARG(float, fixMetrics.absolute_altitude_m));
        }
    });

    /// 开始订阅消息
    setTelemetryRate();
}
