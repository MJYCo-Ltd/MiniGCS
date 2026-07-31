#include "QDroneControlManager.h"

#include "Plat/QAutopilot.h"
#include "QGroundControlStation.h"
#include "QTestGCSConfig.h"

#include <algorithm>

QDroneControlManager::QDroneControlManager(
    QGroundControlStation *groundStation, QObject *parent)
    : QObject(parent)
    , m_groundStation(groundStation)
{
    if (!m_groundStation) {
        return;
    }

    for (QObject *platform : m_groundStation->plats()) {
        registerPlatform(platform);
    }
    connect(m_groundStation, &QGroundControlStation::newPlatFind,
            this, [this](QPlat *platform) {
                registerPlatform(platform);
            });
}

void QDroneControlManager::registerPlatform(QObject *platform)
{
    auto *autopilot = qobject_cast<QAutopilot *>(platform);
    if (!autopilot || autopilot->systemId() < 0) {
        return;
    }

    const int systemId = autopilot->systemId();
    if (m_autopilots.value(systemId) == autopilot) {
        return;
    }
    m_autopilots.insert(systemId, autopilot);
    connect(autopilot, &QPlat::connectionStatusChanged,
            this, &QDroneControlManager::dronesChanged);
    connect(autopilot, &QObject::destroyed, this,
            [this, systemId](QObject *destroyedObject) {
        const QPointer<QAutopilot> current = m_autopilots.value(systemId);
        if (!current || current.data() == destroyedObject) {
            m_autopilots.remove(systemId);
            emit dronesChanged();
        }
    });
    emit dronesChanged();
}

QVariantList QDroneControlManager::drones() const
{
    QList<int> systemIds = m_autopilots.keys();
    std::sort(systemIds.begin(), systemIds.end());

    QVariantList result;
    for (int systemId : systemIds) {
        const QPointer<QAutopilot> autopilot = m_autopilots.value(systemId);
        if (!autopilot) {
            continue;
        }
        QVariantMap drone;
        drone.insert("systemId", systemId);
        drone.insert("name", QTestGCSConfig::instance()->droneName(systemId));
        drone.insert("connected", autopilot->isConnected());
        drone.insert("vehicle", QVariant::fromValue(
            static_cast<QObject *>(autopilot.data())));
        result.append(drone);
    }
    return result;
}

QVariantList QDroneControlManager::groups() const
{
    return QTestGCSConfig::instance()->droneGroupList();
}

void QDroneControlManager::renameDrone(int systemId, const QString &name)
{
    if (!m_autopilots.contains(systemId)) {
        emit commandRejected(QString("未找到系统 ID %1").arg(systemId));
        return;
    }
    QTestGCSConfig::instance()->setDroneName(systemId, name);
    emit dronesChanged();
}

bool QDroneControlManager::addGroup(const QString &name)
{
    const bool added = QTestGCSConfig::instance()->addDroneGroup(name);
    if (added) {
        emit groupsChanged();
    } else {
        emit commandRejected("编组名称为空或已经存在");
    }
    return added;
}

bool QDroneControlManager::removeGroup(const QString &name)
{
    const bool removed = QTestGCSConfig::instance()->removeDroneGroup(name);
    if (removed) {
        emit groupsChanged();
    }
    return removed;
}

QVariantList QDroneControlManager::groupMembers(const QString &name) const
{
    const QVariantList configuredGroups = groups();
    for (const QVariant &value : configuredGroups) {
        const QVariantMap group = value.toMap();
        if (group.value("name").toString() == name) {
            return group.value("members").toList();
        }
    }
    return {};
}

bool QDroneControlManager::setGroupMembers(
    const QString &name, const QVariantList &systemIds)
{
    const bool saved =
        QTestGCSConfig::instance()->setDroneGroupMembers(name, systemIds);
    if (saved) {
        emit groupsChanged();
    }
    return saved;
}

bool QDroneControlManager::execute(
    QAutopilot *autopilot, const QString &command)
{
    if (!autopilot || !autopilot->isConnected()) {
        return false;
    }
    if (command == "arm") {
        autopilot->arm();
        return true;
    }
    if (command == "disarm") {
        autopilot->disarm();
        return true;
    }
    if (command == "takeoff") {
        autopilot->takeoff();
        return true;
    }
    if (command == "land") {
        autopilot->land();
        return true;
    }
    if (command == "returnToLaunch") {
        autopilot->returnToLaunch();
        return true;
    }
    if (command == "downloadMission") {
        if (autopilot->airLineDownloading()) {
            return false;
        }
        autopilot->downloadAirLine();
        return true;
    }
    return false;
}

bool QDroneControlManager::executeSingle(
    int systemId, const QString &command)
{
    QAutopilot *autopilot = m_autopilots.value(systemId);
    if (!execute(autopilot, command)) {
        emit commandRejected(
            QString("无人机 %1 离线或命令不受支持").arg(systemId));
        return false;
    }
    emit commandDispatched(
        command, QTestGCSConfig::instance()->droneName(systemId), 1);
    return true;
}

bool QDroneControlManager::executeGroup(
    const QString &groupName, const QString &command)
{
    const QVariantList members = groupMembers(groupName);
    int dispatched = 0;
    for (const QVariant &member : members) {
        QAutopilot *autopilot = m_autopilots.value(member.toInt());
        if (execute(autopilot, command)) {
            ++dispatched;
        }
    }

    if (dispatched == 0) {
        emit commandRejected("编组中没有可执行该命令的在线无人机");
        return false;
    }
    emit commandDispatched(command, groupName, dispatched);
    return true;
}
