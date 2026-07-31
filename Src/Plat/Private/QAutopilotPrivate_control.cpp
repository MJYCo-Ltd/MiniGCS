#include <cmath>
#include <QCoreApplication>
#include <QPointer>
#include <QThreadPool>
#include <memory>
#include <utility>

#include "Plat/Private/QAutopilotPrivate.h"
#include "Plat/QAutopilot.h"
#include "Private/QMavsdkTextCatalog.h"

void QAutopilotPrivate::downloadAirLine(quint64 requestId)
{
    QPointer<QAutopilot> autopilot = q_func();
    if (!m_mission) {
        if (autopilot) {
            autopilot->failAirLineDownload(
                requestId, QCoreApplication::translate(
                    "QAutopilot", "Mission 插件尚未初始化"));
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

        auto plan = std::make_shared<mavsdk::Mission::MissionPlan>(
            std::move(missionPlan));
        QThreadPool::globalInstance()->start([autopilot, requestId, plan]() {
            QList<QGpsPosition> waypoints;
            waypoints.reserve(
                static_cast<qsizetype>(plan->mission_items.size()));
            for (const mavsdk::Mission::MissionItem &item :
                 plan->mission_items) {
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

            QMetaObject::invokeMethod(
                autopilot,
                [autopilot, requestId,
                 waypoints = std::move(waypoints)]() {
                if (autopilot) {
                    autopilot->completeAirLineDownload(requestId, waypoints);
                }
            },
            Qt::QueuedConnection);
        });
    });
}

void QAutopilotPrivate::uploadAirLine(
    quint64 requestId, const QList<QGpsPosition> &waypoints)
{
    QPointer<QAutopilot> autopilot = q_func();
    if (!m_mission) {
        if (autopilot) {
            autopilot->failAirLineUpload(
                requestId, QCoreApplication::translate(
                    "QAutopilot", "Mission 插件尚未初始化"));
        }
        return;
    }

    mavsdk::Mission::MissionPlan missionPlan;
    missionPlan.mission_items.reserve(
        static_cast<std::size_t>(waypoints.size()));
    for (const QGpsPosition &waypoint : waypoints) {
        mavsdk::Mission::MissionItem item;
        item.latitude_deg = waypoint.latitude();
        item.longitude_deg = waypoint.longitude();
        item.relative_altitude_m = waypoint.altitude();
        missionPlan.mission_items.push_back(item);
    }

    m_mission->upload_mission_async(
        std::move(missionPlan),
        [autopilot, requestId](mavsdk::Mission::Result result) {
        if (!autopilot) {
            return;
        }

        if (result != mavsdk::Mission::Result::Success) {
            const QString reason = QMavsdkTextCatalog::text(
                QStringLiteral("missionResult"), static_cast<int>(result));
            QMetaObject::invokeMethod(
                autopilot,
                [autopilot, requestId, reason]() {
                    if (autopilot) {
                        autopilot->failAirLineUpload(requestId, reason);
                    }
                },
                Qt::QueuedConnection);
            return;
        }

        QMetaObject::invokeMethod(
            autopilot,
            [autopilot, requestId]() {
                if (autopilot) {
                    autopilot->completeAirLineUpload(requestId);
                }
            },
            Qt::QueuedConnection);
    });
}
