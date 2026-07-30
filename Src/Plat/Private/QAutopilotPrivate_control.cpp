#include <cmath>
#include <QPointer>

#include "Plat/Private/QAutopilotPrivate.h"
#include "Plat/QAutopilot.h"
#include "Private/QMavsdkTextCatalog.h"

void QAutopilotPrivate::downloadAirLine(quint64 requestId)
{
    QPointer<QAutopilot> autopilot = q_func();
    if (!m_mission) {
        if (autopilot) {
            autopilot->failAirLineDownload(
                requestId, QStringLiteral("Mission 插件尚未初始化"));
        }
        return;
    }

    m_mission->download_mission_async(
        [autopilot, requestId](mavsdk::Mission::Result result,
                               mavsdk::Mission::MissionPlan missionPlan) {
        if (!autopilot) {
            return;
        }

        if (result != mavsdk::Mission::Result::Success) {
            const QString reason = QMavsdkTextCatalog::text(
                QStringLiteral("missionResult"), static_cast<int>(result));
            QMetaObject::invokeMethod(autopilot,
                [autopilot, requestId, reason]() {
                    if (autopilot) {
                        autopilot->failAirLineDownload(requestId, reason);
                    }
                },
                Qt::QueuedConnection);
            return;
        }

        QList<QGpsPosition> waypoints;
        waypoints.reserve(
            static_cast<qsizetype>(missionPlan.mission_items.size()));
        for (const mavsdk::Mission::MissionItem &item :
             missionPlan.mission_items) {
            if (!std::isfinite(item.longitude_deg) ||
                !std::isfinite(item.latitude_deg) ||
                !std::isfinite(item.relative_altitude_m) ||
                item.longitude_deg < -180.0 ||
                item.longitude_deg > 180.0 ||
                item.latitude_deg < -90.0 ||
                item.latitude_deg > 90.0) {
                continue;
            }
            waypoints.append(QGpsPosition(item.longitude_deg,
                                          item.latitude_deg,
                                          item.relative_altitude_m));
        }

        QMetaObject::invokeMethod(autopilot,
            [autopilot, requestId, waypoints]() {
                if (autopilot) {
                    autopilot->completeAirLineDownload(requestId, waypoints);
                }
            },
            Qt::QueuedConnection);
    });
}
