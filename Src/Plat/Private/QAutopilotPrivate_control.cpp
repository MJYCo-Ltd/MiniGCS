#include <cmath>
#include <QCoreApplication>
#include <QPointer>
#include <QThreadPool>
#include <memory>
#include <utility>

#include "Plat/Private/QAutopilotPrivate.h"
#include "Plat/QAutopilot.h"
#include "Private/QMavsdkTextCatalog.h"

namespace {
QMissionPoint missionPointFromMavsdk(
    const mavsdk::Mission::MissionItem &item)
{
    QMissionPoint::Action action = QMissionPoint::ContinueAction;
    double durationS = 0.0;
    if (item.vehicle_action ==
        mavsdk::Mission::MissionItem::VehicleAction::Land) {
        action = QMissionPoint::LandAction;
    } else if (item.camera_action ==
               mavsdk::Mission::MissionItem::CameraAction::TakePhoto) {
        action = QMissionPoint::TakePhotoAction;
    } else if (item.camera_action ==
               mavsdk::Mission::MissionItem::CameraAction::StartVideo) {
        action = QMissionPoint::RecordVideoAction;
        durationS = std::isfinite(item.loiter_time_s)
            ? item.loiter_time_s : 0.0;
    } else if (std::isfinite(item.loiter_time_s) &&
               item.loiter_time_s > 0.0f) {
        action = QMissionPoint::WaitAction;
        durationS = item.loiter_time_s;
    }

    const double speedMS = std::isfinite(item.speed_m_s)
        ? item.speed_m_s : 0.0;
    return QMissionPoint(
        QGpsPosition(item.longitude_deg, item.latitude_deg,
                     item.relative_altitude_m),
        action, durationS, speedMS, item.is_fly_through);
}

mavsdk::Mission::MissionItem mavsdkItemFromMissionPoint(
    const QMissionPoint &point)
{
    mavsdk::Mission::MissionItem item;
    const QGpsPosition position = point.position();
    item.latitude_deg = position.latitude();
    item.longitude_deg = position.longitude();
    item.relative_altitude_m = position.altitude();
    item.is_fly_through = point.flyThrough();
    if (point.speedMS() > 0.0) {
        item.speed_m_s = static_cast<float>(point.speedMS());
    }

    switch (point.action()) {
    case QMissionPoint::ContinueAction:
        item.is_fly_through = true;
        break;
    case QMissionPoint::WaitAction:
        item.loiter_time_s = static_cast<float>(point.actionDurationS());
        break;
    case QMissionPoint::TakePhotoAction:
        item.camera_action =
            mavsdk::Mission::MissionItem::CameraAction::TakePhoto;
        break;
    case QMissionPoint::RecordVideoAction:
        item.camera_action =
            mavsdk::Mission::MissionItem::CameraAction::StartVideo;
        item.loiter_time_s = static_cast<float>(point.actionDurationS());
        break;
    case QMissionPoint::LandAction:
        item.vehicle_action =
            mavsdk::Mission::MissionItem::VehicleAction::Land;
        break;
    }
    return item;
}
} // namespace

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
            QMetaObject::invokeMethod(autopilot,
                [autopilot, requestId, result]() {
                    if (autopilot) {
                        const QString reason = QMavsdkTextCatalog::text(
                            QStringLiteral("missionResult"),
                            static_cast<int>(result));
                        autopilot->failAirLineDownload(requestId, reason);
                    }
                },
                Qt::QueuedConnection);
            return;
        }

        auto plan = std::make_shared<mavsdk::Mission::MissionPlan>(
            std::move(missionPlan));
        QThreadPool::globalInstance()->start([autopilot, requestId, plan]() {
            QList<QMissionPoint> points;
            points.reserve(
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
                if (item.camera_action ==
                    mavsdk::Mission::MissionItem::CameraAction::StopVideo) {
                    continue;
                }
                points.append(missionPointFromMavsdk(item));
            }

            QMetaObject::invokeMethod(
                autopilot,
                [autopilot, requestId,
                 points = std::move(points)]() {
                if (autopilot) {
                    autopilot->completeAirLineDownload(requestId, points);
                }
            },
            Qt::QueuedConnection);
        });
    });
}

void QAutopilotPrivate::uploadAirLine(
    quint64 requestId, const QList<QMissionPoint> &points,
    bool returnHomeAfterMission)
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
    missionPlan.mission_items.reserve(static_cast<std::size_t>(points.size()));
    for (const QMissionPoint &point : points) {
        missionPlan.mission_items.push_back(
            mavsdkItemFromMissionPoint(point));
        if (point.action() == QMissionPoint::RecordVideoAction) {
            mavsdk::Mission::MissionItem stopItem =
                mavsdkItemFromMissionPoint(point);
            stopItem.camera_action =
                mavsdk::Mission::MissionItem::CameraAction::StopVideo;
            stopItem.loiter_time_s = 0.0f;
            missionPlan.mission_items.push_back(stopItem);
        }
    }

    const std::shared_ptr<mavsdk::System> system = m_pSystem;
    QThreadPool::globalInstance()->start(
        [autopilot, requestId, system,
         returnHomeAfterMission,
         missionPlan = std::move(missionPlan)]() mutable {
        if (!autopilot) {
            return;
        }

        mavsdk::Mission mission(system);
        mavsdk::Mission::Result result =
            mission.set_return_to_launch_after_mission(
                returnHomeAfterMission);
        if (result == mavsdk::Mission::Result::Success) {
            result = mission.upload_mission(std::move(missionPlan));
        }

        if (result != mavsdk::Mission::Result::Success) {
            QMetaObject::invokeMethod(
                autopilot,
                [autopilot, requestId, result]() {
                    if (autopilot) {
                        const QString reason = QMavsdkTextCatalog::text(
                            QStringLiteral("missionResult"),
                            static_cast<int>(result));
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

void QAutopilotPrivate::startAirLine()
{
    QPointer<QAutopilot> autopilot = q_func();
    if (!m_mission) {
        if (autopilot) {
            emit autopilot->airLineStartFailed(
                QCoreApplication::translate(
                    "QAutopilot", "Mission 插件尚未初始化"));
        }
        return;
    }

    m_mission->start_mission_async(
        [autopilot](mavsdk::Mission::Result result) {
        if (!autopilot) {
            return;
        }
        QMetaObject::invokeMethod(
            autopilot,
            [autopilot, result]() {
                if (!autopilot) {
                    return;
                }
                if (result != mavsdk::Mission::Result::Success) {
                    const QString reason = QMavsdkTextCatalog::text(
                        QStringLiteral("missionResult"),
                        static_cast<int>(result));
                    emit autopilot->airLineStartFailed(reason);
                    return;
                }
                emit autopilot->airLineStarted();
            },
            Qt::QueuedConnection);
    });
}

void QAutopilotPrivate::pauseAirLine()
{
    QPointer<QAutopilot> autopilot = q_func();
    if (!m_mission) {
        if (autopilot) {
            emit autopilot->airLinePauseFailed(
                QCoreApplication::translate(
                    "QAutopilot", "Mission 插件尚未初始化"));
        }
        return;
    }

    m_mission->pause_mission_async(
        [autopilot](mavsdk::Mission::Result result) {
        if (!autopilot) {
            return;
        }
        QMetaObject::invokeMethod(
            autopilot,
            [autopilot, result]() {
                if (!autopilot) {
                    return;
                }
                if (result != mavsdk::Mission::Result::Success) {
                    emit autopilot->airLinePauseFailed(
                        QMavsdkTextCatalog::text(
                            QStringLiteral("missionResult"),
                            static_cast<int>(result)));
                    return;
                }
                emit autopilot->airLinePaused();
            },
            Qt::QueuedConnection);
    });
}
