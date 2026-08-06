// SPDX-License-Identifier: MIT
#pragma once

#include "../FirmwareAdapter.h"
#include <unordered_map>
#include <vector>

class ArduPilotAdapter : public FirmwareAdapter {
public:
    ArduPilotAdapter();
    ~ArduPilotAdapter() override = default;

    std::string firmware_name() const override { return "ArduPilot"; }
    std::string flight_mode_from_raw(uint8_t base_mode, uint32_t custom_mode) const override;
    bool flight_mode_to_raw(const std::string& mode, uint8_t& out_base_mode, uint32_t& out_custom_mode) const override;
    void adapt_mission_upload(std::vector<MissionItem>& items) const override;
    bool requires_offboard_arming_sequence() const override { return true; }

private:
    void init_mode_map();
    std::unordered_map<uint32_t, std::string> _mode_map;
};
