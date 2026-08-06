// SPDX-License-Identifier: MIT
#include "ArduPilotAdapterQt.h"
#include <QVariant>
#include <array>

ArduPilotAdapterQt::ArduPilotAdapterQt(mavsdk::System& system, QObject* parent)
    : QObject(parent)
{
    _helper = std::make_unique<MavlinkHelper>(system);
}

ArduPilotAdapterQt::~ArduPilotAdapterQt() = default;

bool ArduPilotAdapterQt::arm(uint8_t target_sys, uint8_t target_comp)
{
    std::array<float,7> params{}; params[0] = 1.0f; // arm
    return _helper->send_command_and_wait_ack(target_sys, target_comp, MAV_CMD_COMPONENT_ARM_DISARM, params);
}

bool ArduPilotAdapterQt::disarm(uint8_t target_sys, uint8_t target_comp)
{
    std::array<float,7> params{}; params[0] = 0.0f; // disarm
    return _helper->send_command_and_wait_ack(target_sys, target_comp, MAV_CMD_COMPONENT_ARM_DISARM, params);
}

bool ArduPilotAdapterQt::set_flight_mode(const QString& mode, uint8_t target_sys, uint8_t target_comp)
{
    uint8_t base_mode = 0; uint32_t custom_mode = 0;
    if (!flight_mode_to_raw(mode.toStdString(), base_mode, custom_mode)) return false;
    mavlink_message_t msg{};
    // pack SET_MODE: target_system, base_mode, custom_mode
    mavlink_msg_set_mode_pack(255, 190, &msg, target_sys, base_mode, custom_mode);
    return _helper->send_message(msg);
}

bool ArduPilotAdapterQt::upload_mission(const QVariantList& mission_items, uint8_t target_sys, uint8_t target_comp)
{
    std::vector<MissionItem> items;
    for (const QVariant& v : mission_items) {
        QVariantMap m = v.toMap();
        MissionItem it;
        it.command = static_cast<uint16_t>(m.value("command").toInt());
        it.frame = static_cast<uint8_t>(m.value("frame").toInt());
        it.auto_continue = m.value("autocontinue", true).toBool();
        it.param1 = m.value("param1", 0.0).toDouble();
        it.param2 = m.value("param2", 0.0).toDouble();
        it.param3 = m.value("param3", 0.0).toDouble();
        it.param4 = m.value("param4", 0.0).toDouble();
        it.x = m.value("x", 0.0).toDouble();
        it.y = m.value("y", 0.0).toDouble();
        it.z = m.value("z", 0.0).toDouble();
        items.push_back(it);
    }

    adapt_mission_upload(items);
    bool ok = _helper->upload_mission(target_sys, target_comp, items);
    emit missionUploadCompleted(ok);
    return ok;
}
