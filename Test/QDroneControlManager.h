#ifndef QDRONECONTROLMANAGER_H
#define QDRONECONTROLMANAGER_H

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QVariantList>
#include <QSet>
#include "AirLine/QMissionPoint.h"

class QAutopilot;
class QGroundControlStation;
class QFlightRecordStore;

class QDroneControlManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList drones READ drones NOTIFY dronesChanged)
    Q_PROPERTY(QVariantList groups READ groups NOTIFY groupsChanged)
    Q_PROPERTY(QVariantList flightRecords READ flightRecords NOTIFY flightRecordsChanged)
    Q_PROPERTY(int armCommand READ armCommand CONSTANT)
    Q_PROPERTY(int disarmCommand READ disarmCommand CONSTANT)
    Q_PROPERTY(int takeoffCommand READ takeoffCommand CONSTANT)
    Q_PROPERTY(int landCommand READ landCommand CONSTANT)
    Q_PROPERTY(int returnToLaunchCommand READ returnToLaunchCommand CONSTANT)
    Q_PROPERTY(int downloadMissionCommand READ downloadMissionCommand CONSTANT)
    Q_PROPERTY(int uploadMissionCommand READ uploadMissionCommand CONSTANT)
    Q_PROPERTY(int startMissionCommand READ startMissionCommand CONSTANT)
    Q_PROPERTY(int pauseMissionCommand READ pauseMissionCommand CONSTANT)

public:
    enum Command {
        InvalidCommand = -1,
        ArmCommand,
        DisarmCommand,
        TakeoffCommand,
        LandCommand,
        ReturnToLaunchCommand,
        DownloadMissionCommand,
        UploadMissionCommand,
        StartMissionCommand,
        PauseMissionCommand
    };
    Q_ENUM(Command)

    explicit QDroneControlManager(
        QGroundControlStation *groundStation, QObject *parent = nullptr);

    QVariantList drones() const;
    QVariantList groups() const;
    QVariantList flightRecords() const;
    int armCommand() const { return ArmCommand; }
    int disarmCommand() const { return DisarmCommand; }
    int takeoffCommand() const { return TakeoffCommand; }
    int landCommand() const { return LandCommand; }
    int returnToLaunchCommand() const { return ReturnToLaunchCommand; }
    int downloadMissionCommand() const { return DownloadMissionCommand; }
    int uploadMissionCommand() const { return UploadMissionCommand; }
    int startMissionCommand() const { return StartMissionCommand; }
    int pauseMissionCommand() const { return PauseMissionCommand; }
    Q_INVOKABLE QString commandName(int command) const;
    Q_INVOKABLE QString vehicleIcon(int vehicleType) const;
    Q_INVOKABLE QVariantList missionActions() const;
    Q_INVOKABLE QString missionActionName(int action) const;

    Q_INVOKABLE void renameDrone(int systemId, const QString &name);
    Q_INVOKABLE bool addGroup(const QString &name);
    Q_INVOKABLE bool removeGroup(const QString &name);
    Q_INVOKABLE QVariantList groupMembers(const QString &name) const;
    Q_INVOKABLE bool setGroupMembers(
        const QString &name, const QVariantList &systemIds);

    Q_INVOKABLE bool executeSingle(int systemId, int command);
    Q_INVOKABLE bool executeGroup(
        const QString &groupName, int command);
    Q_INVOKABLE bool uploadMissionSingle(
        int systemId, const QVariantList &waypoints,
        bool returnHomeAfterMission = true);
    Q_INVOKABLE bool uploadMissionGroup(
        const QString &groupName, const QVariantList &waypoints,
        bool returnHomeAfterMission = true);
    Q_INVOKABLE void clearFlightRecords();
    Q_INVOKABLE bool applyConfiguredLinks();

signals:
    void dronesChanged();
    void groupsChanged();
    void flightRecordsChanged();
    void commandDispatched(
        int command, const QString &target, int count);
    void commandResult(
        int systemId, int command, bool success, const QString &reason);
    void commandRejected(const QString &reason);
    void missionDownloaded(int systemId, const QVariantList &waypoints);
    void missionUploadResult(
        int systemId, bool success, const QString &reason);

private:
    void registerPlatform(QObject *platform);
    QString commandKey(Command command) const;
    bool execute(QAutopilot *autopilot, Command command);
    bool parseMissionPoints(const QVariantList &values,
                            QList<QMissionPoint> &points,
                            QString &reason) const;

    QPointer<QGroundControlStation> m_groundStation;
    QHash<int, QPointer<QAutopilot>> m_autopilots;
    QHash<int, QVariantList> m_uploadedMissionPoints;
    QSet<int> m_startMissionAfterArm;
    QFlightRecordStore *m_flightRecordStore{nullptr};
};

#endif // QDRONECONTROLMANAGER_H
