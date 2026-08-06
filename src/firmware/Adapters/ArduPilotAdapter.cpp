// SPDX-License-Identifier: MIT
#include "ArduPilotAdapter.h"
#include <algorithm>
#include <sstream>

ArduPilotAdapter::ArduPilotAdapter()
{
    init_mode_map();
}

void ArduPilotAdapter::init_mode_map()
{
    // Minimal mode map: expand as needed. Values are ArduPilot custom_mode integers.
    // This is a small illustrative mapping — extend per your needs for ArduPlane/ArduCopter/ArduRover.
    _mode_map[0]  = "STABILIZE";
    _mode_map[3]  = "ALT_HOLD";
    _mode_map[10] = "AUTO";
    _mode_map[15] = "GUIDED";
    _mode_map[11] = "RTL";
    _mode_map[4]  = "GUIDED"; // duplicated for some stacks
}

std::string ArduPilotAdapter::flight_mode_from_raw(uint8_t /*base_mode*/, uint32_t custom_mode) const
{
    auto it = _mode_map.find(custom_mode);
    if (it != _mode_map.end()) return it->second;
    std::ostringstream ss;
    ss << "MODE_" << custom_mode;
    return ss.str();
}

bool ArduPilotAdapter::flight_mode_to_raw(const std::string& mode, uint8_t& out_base_mode, uint32_t& out_custom_mode) const
{
    // find first mapping whose value matches `mode` (case-insensitive)
    std::string m = mode;
    std::transform(m.begin(), m.end(), m.begin(), ::toupper);
    for (const auto& kv : _mode_map) {
        std::string val = kv.second;
        std::transform(val.begin(), val.end(), val.begin(), ::toupper);
        if (val == m) {
            out_custom_mode = kv.first;
            out_base_mode = 0; // ArduPilot uses custom_mode primarily
            return true;
        }
    }
    return false;
}

void ArduPilotAdapter::adapt_mission_upload(std::vector<MissionItem>& items) const
{
    // Example transformations:
    // - Ensure frames/fields are what APM expects (APM often uses MISSION_ITEM rather than MISSION_ITEM_INT).
    // - This MVP simply ensures auto_continue defaults to true and clamps frame to known values.
    for (auto &it : items) {
        if (it.frame == 0) it.frame = 0; // placeholder; adapt as needed
        // Ensure auto_continue explicitly set
        it.auto_continue = true;
        // Further per-command translations can be added here.
    }
}
