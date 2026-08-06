// SPDX-License-Identifier: MIT
// Minimal FirmwareAdapter interface for MiniGCS
#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct MissionItem {
    uint16_t command{};
    uint8_t frame{};
    bool auto_continue{true};
    double param1{0}, param2{0}, param3{0}, param4{0};
    double x{0}, y{0}, z{0};
};

class FirmwareAdapter {
public:
    virtual ~FirmwareAdapter() = default;

    // Human readable firmware name, e.g. "PX4" / "ArduPilot"
    virtual std::string firmware_name() const = 0;

    // Map firmware-specific raw mode -> canonical name
    virtual std::string flight_mode_from_raw(uint8_t base_mode, uint32_t custom_mode) const = 0;

    // Map canonical name -> raw (base_mode/custom_mode)
    virtual bool flight_mode_to_raw(const std::string& mode, uint8_t& out_base_mode, uint32_t& out_custom_mode) const = 0;

    // Called before mission upload: adapter may transform mission items to firmware-specific form
    virtual void adapt_mission_upload(std::vector<MissionItem>& items) const = 0;

    // Whether firmware requires special offboard/guided sequence (adapter will provide helpers)
    virtual bool requires_offboard_arming_sequence() const = 0;
};
