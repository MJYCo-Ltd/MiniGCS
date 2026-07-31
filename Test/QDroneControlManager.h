#ifndef QDRONECONTROLMANAGER_H
#define QDRONECONTROLMANAGER_H

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QVariantList>

class QAutopilot;
class QGroundControlStation;

class QDroneControlManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList drones READ drones NOTIFY dronesChanged)
    Q_PROPERTY(QVariantList groups READ groups NOTIFY groupsChanged)

public:
    explicit QDroneControlManager(
        QGroundControlStation *groundStation, QObject *parent = nullptr);

    QVariantList drones() const;
    QVariantList groups() const;

    Q_INVOKABLE void renameDrone(int systemId, const QString &name);
    Q_INVOKABLE bool addGroup(const QString &name);
    Q_INVOKABLE bool removeGroup(const QString &name);
    Q_INVOKABLE QVariantList groupMembers(const QString &name) const;
    Q_INVOKABLE bool setGroupMembers(
        const QString &name, const QVariantList &systemIds);

    Q_INVOKABLE bool executeSingle(int systemId, const QString &command);
    Q_INVOKABLE bool executeGroup(
        const QString &groupName, const QString &command);

signals:
    void dronesChanged();
    void groupsChanged();
    void commandDispatched(
        const QString &command, const QString &target, int count);
    void commandRejected(const QString &reason);

private:
    void registerPlatform(QObject *platform);
    bool execute(QAutopilot *autopilot, const QString &command);

    QPointer<QGroundControlStation> m_groundStation;
    QHash<int, QPointer<QAutopilot>> m_autopilots;
};

#endif // QDRONECONTROLMANAGER_H
