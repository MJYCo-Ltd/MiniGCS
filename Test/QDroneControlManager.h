#ifndef QDRONECONTROLMANAGER_H
#define QDRONECONTROLMANAGER_H

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QVariantList>
#include "AirLine/QGpsPosition.h"

class QAutopilot;
class QGroundControlStation;

class QDroneControlManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList drones READ drones NOTIFY dronesChanged)
    Q_PROPERTY(QVariantList groups READ groups NOTIFY groupsChanged)
    Q_PROPERTY(QStringList businessLogs READ businessLogs NOTIFY businessLogsChanged)
    Q_PROPERTY(QStringList firmwareLogs READ firmwareLogs NOTIFY firmwareLogsChanged)
    Q_PROPERTY(int armCommand READ armCommand CONSTANT)
    Q_PROPERTY(int disarmCommand READ disarmCommand CONSTANT)
    Q_PROPERTY(int takeoffCommand READ takeoffCommand CONSTANT)
    Q_PROPERTY(int landCommand READ landCommand CONSTANT)
    Q_PROPERTY(int returnToLaunchCommand READ returnToLaunchCommand CONSTANT)
    Q_PROPERTY(int downloadMissionCommand READ downloadMissionCommand CONSTANT)
    Q_PROPERTY(int uploadMissionCommand READ uploadMissionCommand CONSTANT)

public:
    enum Command {
        InvalidCommand = -1,
        ArmCommand,
        DisarmCommand,
        TakeoffCommand,
        LandCommand,
        ReturnToLaunchCommand,
        DownloadMissionCommand,
        UploadMissionCommand
    };
    Q_ENUM(Command)

    explicit QDroneControlManager(
        QGroundControlStation *groundStation, QObject *parent = nullptr);

    QVariantList drones() const;
    QVariantList groups() const;
    QStringList businessLogs() const;
    QStringList firmwareLogs() const;
    int armCommand() const { return ArmCommand; }
    int disarmCommand() const { return DisarmCommand; }
    int takeoffCommand() const { return TakeoffCommand; }
    int landCommand() const { return LandCommand; }
    int returnToLaunchCommand() const { return ReturnToLaunchCommand; }
    int downloadMissionCommand() const { return DownloadMissionCommand; }
    int uploadMissionCommand() const { return UploadMissionCommand; }
    Q_INVOKABLE QString commandName(int command) const;
    Q_INVOKABLE QString vehicleIcon(int vehicleType) const;

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
        int systemId, const QVariantList &waypoints);
    Q_INVOKABLE bool uploadMissionGroup(
        const QString &groupName, const QVariantList &waypoints);
    Q_INVOKABLE void clearBusinessLogs();
    Q_INVOKABLE void clearFirmwareLogs();

signals:
    void dronesChanged();
    void groupsChanged();
    void businessLogsChanged();
    void firmwareLogsChanged();
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
    bool parseWaypoints(const QVariantList &values,
                        QList<QGpsPosition> &waypoints,
                        QString &reason) const;

    QPointer<QGroundControlStation> m_groundStation;
    QHash<int, QPointer<QAutopilot>> m_autopilots;
    QStringList m_businessLogs;
    QStringList m_firmwareLogs;
};

#endif // QDRONECONTROLMANAGER_H
