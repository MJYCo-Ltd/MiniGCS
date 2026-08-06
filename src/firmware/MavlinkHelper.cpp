// SPDX-License-Identifier: MIT
#include "MavlinkHelper.h"
#include <cstring>
#include <thread>
#include <chrono>

MavlinkHelper::MavlinkHelper(mavsdk::System& system)
    : _passthrough(system)
{
    // subscribe
    _passthrough.subscribe_message([this](const mavsdk::MavlinkPassthrough::Message& m){
        _on_passthrough_message(m);
    });
}

MavlinkHelper::~MavlinkHelper()
{
}

bool MavlinkHelper::send_message(const mavlink_message_t& msg)
{
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    int len = mavlink_msg_to_send_buffer(buf, &msg);
    mavsdk::MavlinkPassthrough::Message out;
    out.payload = std::vector<uint8_t>(buf, buf + len);
    auto res = _passthrough.send_message(out);
    return res == mavsdk::MavlinkPassthrough::Result::Success;
}

bool MavlinkHelper::send_command_and_wait_ack(uint8_t target_sys, uint8_t target_comp,
                                              uint16_t command, const std::array<float,7>& params,
                                              int timeout_ms)
{
    std::unique_lock<std::mutex> lk(_mutex);
    _awaiting_command = command;
    _last_ack_result = -1;

    mavlink_message_t msg{};
    mavlink_command_long_t cmd{};
    cmd.target_system = target_sys;
    cmd.target_component = target_comp;
    cmd.command = command;
    for (size_t i = 0; i < 7; ++i) cmd.param[i] = params[i];
    cmd.confirmation = 0;

    mavlink_msg_command_long_encode(255, 190, &msg, &cmd);
    if (!send_message(msg)) return false;

    // wait for ack
    if (_cv.wait_for(lk, std::chrono::milliseconds(timeout_ms), [this]{ return _last_ack_result != -1; })) {
        return _last_ack_result == MAV_RESULT_ACCEPTED;
    }
    return false; // timeout
}

bool MavlinkHelper::upload_mission(uint8_t target_sys, uint8_t target_comp,
                                   const std::vector<MissionItem>& items,
                                   int timeout_ms)
{
    std::unique_lock<std::mutex> lk(_mutex);
    _awaiting_request_seq = 0;
    _mission_ack_received = false;

    // send MISSION_COUNT
    mavlink_message_t count_msg{};
    mavlink_mission_count_t mc{};
    mc.target_system = target_sys;
    mc.target_component = target_comp;
    mc.count = items.size();
    mavlink_msg_mission_count_encode(255, 190, &count_msg, &mc);
    if (!send_message(count_msg)) return false;

    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    while (!_mission_ack_received && std::chrono::steady_clock::now() < deadline) {
        // wait for MISSION_REQUEST for sequence
        if (_cv.wait_until(lk, deadline) == std::cv_status::timeout) break;

        while (_awaiting_request_seq >= 0 && _awaiting_request_seq < (int)items.size()) {
            int seq = _awaiting_request_seq;
            // build MISSION_ITEM_INT for seq
            const MissionItem& it = items[seq];
            mavlink_mission_item_int_t mi{};
            mi.target_system = target_sys;
            mi.target_component = target_comp;
            mi.seq = seq;
            mi.frame = it.frame;
            mi.command = it.command;
            mi.current = 0;
            mi.autocontinue = it.auto_continue ? 1 : 0;
            // coords scaled by 1e7
            mi.x = static_cast<int32_t>(it.x * 1e7);
            mi.y = static_cast<int32_t>(it.y * 1e7);
            mi.z = static_cast<int32_t>(it.z * 1000.0); // maybe mm
            mi.param1 = it.param1;
            mi.param2 = it.param2;
            mi.param3 = it.param3;
            mi.param4 = it.param4;

            mavlink_message_t out{};
            mavlink_msg_mission_item_int_encode(255, 190, &out, &mi);
            if (!send_message(out)) return false;

            // clear awaiting request so we wait for next
            _awaiting_request_seq = -1;
            // short sleep to avoid flooding
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        if (_mission_ack_received) break;
    }

    return _mission_ack_received;
}

void MavlinkHelper::_on_passthrough_message(const mavsdk::MavlinkPassthrough::Message& m)
{
    // parse bytes into mavlink_message_t(s)
    mavlink_message_t msg{};
    mavlink_status_t status{};
    for (uint8_t b : m.payload) {
        if (mavlink_parse_char(MAVLINK_COMM_0, b, &msg, &status)) {
            std::unique_lock<std::mutex> lk(_mutex);
            if (msg.msgid == MAVLINK_MSG_ID_COMMAND_ACK) {
                mavlink_command_ack_t ack{};
                mavlink_msg_command_ack_decode(&msg, &ack);
                if (ack.command == _awaiting_command) {
                    _last_ack_result = ack.result;
                    _cv.notify_all();
                }
            } else if (msg.msgid == MAVLINK_MSG_ID_MISSION_REQUEST) {
                mavlink_mission_request_t req{};
                mavlink_msg_mission_request_decode(&msg, &req);
                _awaiting_request_seq = req.seq;
                _cv.notify_all();
            } else if (msg.msgid == MAVLINK_MSG_ID_MISSION_ACK) {
                mavlink_mission_ack_t ack{};
                mavlink_msg_mission_ack_decode(&msg, &ack);
                _mission_ack_received = true;
                _cv.notify_all();
            }
            // other handling may be added
        }
    }
}
