#include "Plat/QAutopilot.h"
#include "Plat/Private/QAutopilotPrivate.h"
#include <QPointer>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <sstream>

#include "Extern/XmlToMavSDK.h"
#include "QGCSConfig.h"
#include "Private/QGCSConfigInternal.h"
#include "Private/QGCSLog.h"
#include "Private/QMavsdkTextCatalog.h"
#include "Plat/Private/QMavsdkTypeMap.h"

template<>struct fmt::formatter<mavsdk::Action::Result>:ostream_formatter{};

namespace {
bool hasFixedWingMetrics(QAutoVehicleType::Vehicle vehicle)
{
    switch (vehicle) {
    case QAutoVehicleType::FixedWing:
    case QAutoVehicleType::VtolTailsitterDuorotor:
    case QAutoVehicleType::VtolTailsitterQuadrotor:
    case QAutoVehicleType::VtolTiltrotor:
    case QAutoVehicleType::VtolFixedrotor:
    case QAutoVehicleType::VtolTailsitter:
    case QAutoVehicleType::VtolTiltwing:
        return true;
    default:
        return false;
    }
}

void dispatchActionResult(
    const QPointer<QAutopilot> &autopilot,
    QAutopilot::ActionCommand command,
    mavsdk::Action::Result result,
    uint8_t systemId,
    const char *operation)
{
    if (result != mavsdk::Action::Result::Success) {
        spdlog::error(PLAT_FMT_STR, systemId, operation, result);
    }
    if (!autopilot) {
        return;
    }
    QMetaObject::invokeMethod(
        autopilot,
        [autopilot, command, result]() {
            if (!autopilot) {
                return;
            }
            const QString reason = QMavsdkTextCatalog::text(
                QStringLiteral("actionResult"), static_cast<int>(result));
            emit autopilot->actionCommandFinished(
                command, result == mavsdk::Action::Result::Success, reason);
        },
        Qt::QueuedConnection);
}
} // namespace

QAutopilotPrivate::QAutopilotPrivate(QPlat *pPlat)
    : QPlatPrivate(pPlat)
{}

QAutopilotPrivate::~QAutopilotPrivate()
{
    if (m_mission) {
        m_mission->cancel_mission_download();
        m_mission->cancel_mission_upload();
    }
    clearExternalCommandSubscription();
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
    clearExternalCommandSubscription();
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
    setupExternalCommandSubscription();

    q_func()->setAutopilotType(
        MavsdkTypeMap::toAutopilot(system->autopilot_type()));
    q_func()->setVehicleType(
        MavsdkTypeMap::toVehicle(system->vehicle_type()));

    // arm();
}

void QAutopilotPrivate::arm() {
    if (!m_action || !m_pSystem) {
        return;
    }
    const uint8_t systemId = m_pSystem->get_system_id();
    const QPointer<QAutopilot> autopilot(q_func());
    m_action->arm_async([autopilot, systemId](mavsdk::Action::Result result) {
        dispatchActionResult(autopilot, QAutopilot::ArmAction,
                             result, systemId, "arm");
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
    if (m_rawGpsHandle.valid()) {
        m_telemetry->unsubscribe_raw_gps(m_rawGpsHandle);
        m_rawGpsHandle = {};
    }
    if (m_attitudeEulerHandle.valid()) {
        m_telemetry->unsubscribe_attitude_euler(m_attitudeEulerHandle);
        m_attitudeEulerHandle = {};
    }
    if (m_flightModeHandle.valid()) {
        m_telemetry->unsubscribe_flight_mode(m_flightModeHandle);
        m_flightModeHandle = {};
    }
    if (m_landedStateHandle.valid()) {
        m_telemetry->unsubscribe_landed_state(m_landedStateHandle);
        m_landedStateHandle = {};
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
    const QPointer<QAutopilot> autopilot(q_func());
    m_action->disarm_async([autopilot, systemId](mavsdk::Action::Result result) {
        dispatchActionResult(autopilot, QAutopilot::DisarmAction,
                             result, systemId, "disarm");
    });
}

void QAutopilotPrivate::takeoff()
{
    if (!m_action || !m_pSystem) {
        return;
    }
    const uint8_t systemId = m_pSystem->get_system_id();
    const QPointer<QAutopilot> autopilot(q_func());
    m_action->takeoff_async([autopilot, systemId](mavsdk::Action::Result result) {
        dispatchActionResult(autopilot, QAutopilot::TakeoffAction,
                             result, systemId, "takeoff");
    });
}

void QAutopilotPrivate::land()
{
    if (!m_action || !m_pSystem) {
        return;
    }
    const uint8_t systemId = m_pSystem->get_system_id();
    const QPointer<QAutopilot> autopilot(q_func());
    m_action->land_async([autopilot, systemId](mavsdk::Action::Result result) {
        dispatchActionResult(autopilot, QAutopilot::LandAction,
                             result, systemId, "land");
    });
}

void QAutopilotPrivate::returnToLaunch()
{
    if (!m_action || !m_pSystem) {
        return;
    }
    const uint8_t systemId = m_pSystem->get_system_id();
    const QPointer<QAutopilot> autopilot(q_func());
    m_action->return_to_launch_async(
        [autopilot, systemId](mavsdk::Action::Result result) {
            dispatchActionResult(autopilot, QAutopilot::ReturnToLaunchAction,
                                 result, systemId, "return_to_launch");
        });
}

template<>struct fmt::formatter<mavsdk::MavlinkDirect::Result>:ostream_formatter{};

void QAutopilotPrivate::clearExternalCommandSubscription()
{
    if (m_pMavlinkDirect && m_commandAckHandle.valid()) {
        m_pMavlinkDirect->unsubscribe_message(m_commandAckHandle);
        m_commandAckHandle = {};
    }
    m_pendingExternalCommand.reset();
    ++m_externalCommandGeneration;
}

void QAutopilotPrivate::setupExternalCommandSubscription()
{
    if (!m_pMavlinkDirect || !m_pSystem) {
        return;
    }

    const QPointer<QAutopilot> autopilot(q_func());
    const std::weak_ptr<mavsdk::System> weakSystem(m_pSystem);
    m_commandAckHandle = m_pMavlinkDirect->subscribe_message(
        "COMMAND_ACK",
        [autopilot, weakSystem](mavsdk::MavlinkDirect::MavlinkMessage message) {
            if (!autopilot) {
                return;
            }
            const uint32_t sourceComponentId = message.component_id;
            std::string fieldsJson = std::move(message.fields_json);
            QMetaObject::invokeMethod(
                autopilot,
                [autopilot, weakSystem,
                 sourceComponentId,
                 fieldsJson = std::move(fieldsJson)]() {
                    const auto system = weakSystem.lock();
                    if (!autopilot || !system || !autopilot->d_func() ||
                        autopilot->d_func()->getSystem() != system) {
                        return;
                    }
                    autopilot->d_func()->handleExternalCommandAck(
                        sourceComponentId, fieldsJson);
                },
                Qt::QueuedConnection);
        });
}

void QAutopilotPrivate::handleExternalCommandAck(
    uint32_t sourceComponentId, const std::string &fieldsJson)
{
    if (!m_pendingExternalCommand) {
        return;
    }
    if (m_pendingExternalCommand->componentId != 0 &&
        m_pendingExternalCommand->componentId != sourceComponentId) {
        return;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(
        QByteArray::fromStdString(fieldsJson), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        return;
    }

    const QJsonObject object = document.object();
    const int commandId = object.value(QStringLiteral("command")).toInt(-1);
    if (commandId != m_pendingExternalCommand->commandId) {
        return;
    }

    const int mavResult = object.value(QStringLiteral("result")).toInt(-1);
    if (mavResult == MAV_RESULT_IN_PROGRESS) {
        const quint64 generation = ++m_externalCommandGeneration;
        m_pendingExternalCommand->generation = generation;
        scheduleExternalCommandTimeout(generation);
        return;
    }

    const QString name = m_pendingExternalCommand->name;
    m_pendingExternalCommand.reset();
    ++m_externalCommandGeneration;
    const bool success = mavResult == MAV_RESULT_ACCEPTED;
    const QString reason = QMavsdkTextCatalog::text(
        QStringLiteral("commandAckResult"), mavResult);
    if (success) {
        spdlog::info(PLAT_FMT_STR, m_pSystem->get_system_id(),
                     "externCommand", name.toUtf8().constData());
    } else {
        spdlog::warn(PLAT_FMT_STR, m_pSystem->get_system_id(),
                     "externCommand", reason.toUtf8().constData());
    }
}

void QAutopilotPrivate::scheduleExternalCommandTimeout(quint64 generation)
{
    const QPointer<QAutopilot> autopilot(q_func());
    if (!autopilot) {
        return;
    }
    QTimer::singleShot(
        QGCSConfigInternal::commandAckTimeoutMs(), autopilot.data(),
        [autopilot, generation]() {
            if (!autopilot || !autopilot->d_func()) {
                return;
            }
            QAutopilotPrivate *implementation = autopilot->d_func();
            if (!implementation->m_pendingExternalCommand ||
                implementation->m_pendingExternalCommand->generation !=
                    generation) {
                return;
            }
            const QString name =
                implementation->m_pendingExternalCommand->name;
            implementation->m_pendingExternalCommand.reset();
            ++implementation->m_externalCommandGeneration;
            spdlog::warn(PLAT_FMT_STR, autopilot->vehicleId(),
                         "externCommandTimeout",
                         name.toUtf8().constData());
        });
}

bool QAutopilotPrivate::sendExternCommand(const QString &name,
                                          quint32 componentId,
                                          const QVector<float> &params)
{
    if (!m_xmlExtension || !m_pMavlinkDirect || !m_pSystem) {
        spdlog::error(SYS_FMT_STR, "sendExternCommand",
                      "extension or system not ready");
        return false;
    }
    if (m_pendingExternalCommand) {
        spdlog::warn(PLAT_FMT_STR, m_pSystem->get_system_id(),
                     "sendExternCommand", "another command is pending");
        return false;
    }
    if (componentId > 255 || params.size() > 7) {
        spdlog::error(PLAT_FMT_STR, m_pSystem->get_system_id(),
                      "sendExternCommand", "invalid component or params");
        return false;
    }
    if (!m_xmlExtension->isCmdTableLoaded()) {
        spdlog::error(SYS_FMT_STR, "sendExternCommand",
                      "MAV_CMD catalog not loaded");
        return false;
    }

    const XmlToMavSDK::ExternCmd *command = m_xmlExtension->findCmd(name);
    if (!command) {
        spdlog::error(PLAT_FMT_STR, m_pSystem->get_system_id(),
                      "sendExternCommand", "command not found");
        return false;
    }

    const quint64 generation = ++m_externalCommandGeneration;
    m_pendingExternalCommand = PendingExternalCommand{
        name, static_cast<int>(command->value), componentId, generation};

    const auto result = m_xmlExtension->sendCmd(
        *m_pMavlinkDirect, *m_pSystem, name, componentId, params);
    if (mavsdk::MavlinkDirect::Result::Success != result) {
        m_pendingExternalCommand.reset();
        ++m_externalCommandGeneration;
        spdlog::error(PLAT_FMT_STR, m_pSystem->get_system_id(),
                      name.toUtf8().constData(), result);
        return false;
    }
    scheduleExternalCommandTimeout(generation);
    return true;
}

template<>struct fmt::formatter<mavsdk::Telemetry::Result>:ostream_formatter{};

void QAutopilotPrivate::setTelemetryRate() {
    if (!m_telemetry || !m_pSystem) {
        return;
    }
    const uint8_t systemId = m_pSystem->get_system_id();
    /// 设置 位置信息 频率
    m_telemetry->set_rate_position_async(
        QGCSConfigInternal::telemetryPositionHz(),
        [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_position", result);
            }
        });

    m_telemetry->set_rate_position_velocity_ned_async(
        QGCSConfigInternal::telemetryPositionVelocityNedHz(),
        [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_position_velocity_ned", result);
            }
        });

    /// 设置 gps 状态 发送频率
    m_telemetry->set_rate_gps_info_async(
        QGCSConfigInternal::telemetryGpsInfoHz(),
        [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_gps_info", result);
            }
        });

    /// 设置 电池信息 发送频率
    m_telemetry->set_rate_battery_async(
        QGCSConfigInternal::telemetryBatteryHz(),
        [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_battery", result);
            }
        });

    m_telemetry->set_rate_raw_gps_async(
        QGCSConfigInternal::telemetryRawGpsHz(),
        [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_raw_gps", result);
            }
        });

    m_telemetry->set_rate_attitude_euler_async(
        QGCSConfigInternal::telemetryAttitudeHz(),
        [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_attitude_euler", result);
            }
        });

    m_telemetry->set_rate_landed_state_async(
        QGCSConfigInternal::telemetryLandedStateHz(),
        [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_landed_state", result);
            }
        });

    /// 设置 健康度 发送频率
    m_telemetry->set_rate_health_async(
        QGCSConfigInternal::telemetryHealthHz(),
        [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_health", result);
            }
        });

    /// 设置 home 发送频率
    m_telemetry->set_rate_home_async(
        QGCSConfigInternal::telemetryHomeHz(),
        [systemId](mavsdk::Telemetry::Result result) {
            if (mavsdk::Telemetry::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, systemId,
                              "set_rate_home", result);
            }
        });

    if (hasFixedWingMetrics(q_func()->vehicleType())) {
        m_telemetry->set_rate_fixedwing_metrics_async(
            QGCSConfigInternal::telemetryFixedwingMetricsHz(),
            [systemId](mavsdk::Telemetry::Result result) {
                if (mavsdk::Telemetry::Result::Success != result) {
                    spdlog::error(PLAT_FMT_STR, systemId,
                                  "set_rate_fixedwing_metrics", result);
                }
            });
    }

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
                Q_ARG(float, position.absolute_altitude_m),
                Q_ARG(float, position.relative_altitude_m));
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
                Q_ARG(int, static_cast<int>(battery.id)),
                Q_ARG(float, battery.temperature_degc),
                Q_ARG(float, battery.voltage_v),
                Q_ARG(float, battery.current_battery_a),
                Q_ARG(float, battery.capacity_consumed_ah),
                Q_ARG(float, battery.remaining_percent),
                Q_ARG(float, battery.time_remaining_s),
                Q_ARG(int, static_cast<int>(battery.battery_function)));
        }
    });

    m_rawGpsHandle = m_telemetry->subscribe_raw_gps(
        [autopilot](mavsdk::Telemetry::RawGps gps) {
            if (autopilot) {
                QMetaObject::invokeMethod(
                    autopilot, "rawGpsUpdate", Qt::QueuedConnection,
                    Q_ARG(float, gps.hdop),
                    Q_ARG(float, gps.vdop),
                    Q_ARG(float, gps.velocity_m_s),
                    Q_ARG(float, gps.cog_deg),
                    Q_ARG(float, gps.horizontal_uncertainty_m),
                    Q_ARG(float, gps.vertical_uncertainty_m),
                    Q_ARG(float, gps.velocity_uncertainty_m_s),
                    Q_ARG(float, gps.heading_uncertainty_deg));
            }
        });

    m_attitudeEulerHandle = m_telemetry->subscribe_attitude_euler(
        [autopilot](mavsdk::Telemetry::EulerAngle attitude) {
            if (autopilot) {
                QMetaObject::invokeMethod(
                    autopilot, "attitudeUpdate", Qt::QueuedConnection,
                    Q_ARG(float, attitude.roll_deg),
                    Q_ARG(float, attitude.pitch_deg),
                    Q_ARG(float, attitude.yaw_deg));
            }
        });

    m_flightModeHandle = m_telemetry->subscribe_flight_mode(
        [autopilot](mavsdk::Telemetry::FlightMode mode) {
            if (autopilot) {
                std::ostringstream fallback;
                fallback << mode;
                QMetaObject::invokeMethod(
                    autopilot, "flightModeUpdate", Qt::QueuedConnection,
                    Q_ARG(QAutopilot::FlightMode,
                          MavsdkTypeMap::toFlightMode(mode)),
                    Q_ARG(QString, QString::fromStdString(fallback.str())));
            }
        });

    m_landedStateHandle = m_telemetry->subscribe_landed_state(
        [autopilot](mavsdk::Telemetry::LandedState state) {
            if (autopilot) {
                std::ostringstream fallback;
                fallback << state;
                QMetaObject::invokeMethod(
                    autopilot, "landedStateUpdate", Qt::QueuedConnection,
                    Q_ARG(QAutopilot::LandedState,
                          MavsdkTypeMap::toLandedState(state)),
                    Q_ARG(QString, QString::fromStdString(fallback.str())));
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

    if (hasFixedWingMetrics(q_func()->vehicleType())) {
        m_fixedwingMetricsHandle = m_telemetry->subscribe_fixedwing_metrics(
            [autopilot](mavsdk::Telemetry::FixedwingMetrics metrics) {
                if (autopilot) {
                    QMetaObject::invokeMethod(
                        autopilot, "fixedwingUpdate", Qt::QueuedConnection,
                        Q_ARG(float, metrics.airspeed_m_s),
                        Q_ARG(float, metrics.throttle_percentage),
                        Q_ARG(float, metrics.climb_rate_m_s),
                        Q_ARG(float, metrics.groundspeed_m_s),
                        Q_ARG(float, metrics.heading_deg),
                        Q_ARG(float, metrics.absolute_altitude_m));
                }
            });
    }

    /// 开始订阅消息
    setTelemetryRate();
}
