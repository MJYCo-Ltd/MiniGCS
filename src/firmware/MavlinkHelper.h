// SPDX-License-Identifier: MIT
#pragma once

#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include <mavlink/v2.0/common/mavlink.h>
#include <functional>
#include <future>
#include <mutex>
#include <condition_variable>
#include <vector>

// Helper that wraps MAVSDK MavlinkPassthrough for common patterns: send raw message,
// wait for COMMAND_ACK, minimal mission upload. Designed as a lightweight helper.

class MavlinkHelper {
public:
    using Passthrough = mavsdk::MavlinkPassthrough;

    MavlinkHelper(mavsdk::System& system);
    ~MavlinkHelper();

    // Send an encoded mavlink_message_t via passthrough
    bool send_message(const mavlink_message_t& msg);

    // Send a COMMAND_LONG and wait for COMMAND_ACK (timeout in ms)
    // Returns true on MAV_RESULT_ACCEPTED (0)
    bool send_command_and_wait_ack(uint8_t target_sys, uint8_t target_comp,
                                   uint16_t command, const std::array<float,7>& params,
                                   int timeout_ms = 3000);

    // Minimal mission upload using MISSION_COUNT / MISSION_REQUEST / MISSION_ITEM / MISSION_ACK
    // items are adapted by caller when needed
    bool upload_mission(uint8_t target_sys, uint8_t target_comp,
                        const std::vector<MissionItem>& items,
                        int timeout_ms = 5000);

private:
    Passthrough _passthrough;
    std::mutex _mutex;
    std::condition_variable _cv;

    // State for waiting ack
    uint16_t _awaiting_command{0};
    int _last_ack_result{-1};

    // For mission upload
    int _awaiting_request_seq{-1};
    bool _mission_ack_received{false};

    void _on_passthrough_message(const mavsdk::MavlinkPassthrough::Message& m);
};
