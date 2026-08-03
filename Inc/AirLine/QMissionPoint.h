#ifndef QMISSIONPOINT_H
#define QMISSIONPOINT_H

#include "Common/QGpsPosition.h"
#include "MiniGCSExport.h"

#include <QList>
#include <QMetaType>

/**
 * @brief 面向任务规划的航点。
 *
 * 该类型是任务点位置、到达动作和飞行方式的单一事实来源；协议层负责
 * 将其转换为 MAVSDK MissionItem，界面层不接触协议枚举。
 */
class MINIGCS_EXPORT QMissionPoint
{
    Q_GADGET
    Q_PROPERTY(QGpsPosition position READ position WRITE setPosition)
    Q_PROPERTY(Action action READ action WRITE setAction)
    Q_PROPERTY(double actionDurationS READ actionDurationS WRITE setActionDurationS)
    Q_PROPERTY(double speedMS READ speedMS WRITE setSpeedMS)
    Q_PROPERTY(bool flyThrough READ flyThrough WRITE setFlyThrough)

public:
    enum Action {
        ContinueAction = 0,
        WaitAction,
        TakePhotoAction,
        RecordVideoAction,
        LandAction
    };
    Q_ENUM(Action)

    QMissionPoint() = default;
    explicit QMissionPoint(const QGpsPosition &position,
                           Action action = ContinueAction,
                           double actionDurationS = 0.0,
                           double speedMS = 0.0,
                           bool flyThrough = false);

    QGpsPosition position() const { return m_position; }
    void setPosition(const QGpsPosition &position) { m_position = position; }

    Action action() const { return m_action; }
    void setAction(Action action) { m_action = action; }

    double actionDurationS() const { return m_actionDurationS; }
    void setActionDurationS(double durationS) { m_actionDurationS = durationS; }

    double speedMS() const { return m_speedMS; }
    void setSpeedMS(double speedMS) { m_speedMS = speedMS; }

    bool flyThrough() const { return m_flyThrough; }
    void setFlyThrough(bool flyThrough) { m_flyThrough = flyThrough; }

    bool operator==(const QMissionPoint &other) const;
    bool operator!=(const QMissionPoint &other) const { return !(*this == other); }

private:
    QGpsPosition m_position;
    Action m_action{ContinueAction};
    double m_actionDurationS{0.0};
    double m_speedMS{0.0};
    bool m_flyThrough{false};
};

Q_DECLARE_METATYPE(QMissionPoint)
Q_DECLARE_METATYPE(QList<QMissionPoint>)

#endif // QMISSIONPOINT_H
