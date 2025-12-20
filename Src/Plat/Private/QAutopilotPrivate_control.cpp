#include "Plat/Private/QAutopilotPrivate.h"
#include "QGCSLog.h"
#include <spdlog/spdlog.h>

template<>struct fmt::formatter<mavsdk::Mission::Result>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Mission::ProgressData>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Mission::MissionItem>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::MissionRaw::Result>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::MissionRaw::MissionItem>:ostream_formatter{};

void QAutopilotPrivate::updateAirLine()
{
}

void QAutopilotPrivate::downAirLine() {
    if (!m_mission) {
        spdlog::error(PLAT_FMT_STR, m_pSystem->get_system_id(),
                     "downAirLine", "Mission plugin not initialized");
        return;
    }

    spdlog::info(PLAT_FMT_STR, m_pSystem->get_system_id(),
                 "downAirLine", "Starting mission download");

    m_mission->download_mission_with_progress_async(
        [this](mavsdk::Mission::Result result,
               mavsdk::Mission::ProgressDataOrMission progress) {
            if (result != mavsdk::Mission::Result::Success) {
                spdlog::error(PLAT_FMT_STR, m_pSystem->get_system_id(),
                             "downAirLineResult", result);
                return;
            }

            // 检查是进度数据还是任务数据
            if (progress.has_progress) {
                // 进度更新
                auto progressData = progress.progress;
                spdlog::info(PLAT_FMT_STR, m_pSystem->get_system_id(),
                            "downAirLineProgress", progressData);
            } else if (progress.has_mission) {
                // 任务下载完成
                auto missionPlan = progress.mission_plan;
                spdlog::info(PLAT_FMT_STR, m_pSystem->get_system_id(),
                            "downMission", missionPlan.mission_items.size());
                foreach (auto& item, missionPlan.mission_items) {
                    spdlog::info(PLAT_FMT_STR, m_pSystem->get_system_id(),
                                 "missionItem", item);
                }
                
                // TODO: 将任务项转换为航线并更新到 QAutopilot
                // 这里可以调用 updateAirLine() 来处理下载的任务
                // 或者通过信号通知 QAutopilot 任务已下载
            }
        });

    if (!m_rawMission) {
        spdlog::error(PLAT_FMT_STR, m_pSystem->get_system_id(),
                     "downAirLine", "MissionRaw plugin not initialized");
        return;
    }

    spdlog::info(PLAT_FMT_STR, m_pSystem->get_system_id(),
                 "downAirLine", "Starting raw mission download");

    m_rawMission->download_mission_async(
        [this](mavsdk::MissionRaw::Result result,
               std::vector<mavsdk::MissionRaw::MissionItem> missionItems) {
            if (result != mavsdk::MissionRaw::Result::Success) {
                spdlog::error(PLAT_FMT_STR, m_pSystem->get_system_id(),
                             "downRawMissionResult", result);
                return;
            }

            spdlog::info(PLAT_FMT_STR, m_pSystem->get_system_id(),
                        "downRawMission successfully, items:", missionItems.size());
            
            for (const auto& item : missionItems) {
                spdlog::info(PLAT_FMT_STR, m_pSystem->get_system_id(),
                            "rawMissionItem", item);
            }
            
            // TODO: 将原始任务项转换为航线并更新到 QAutopilot
        });
}
