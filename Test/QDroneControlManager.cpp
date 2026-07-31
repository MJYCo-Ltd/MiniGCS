#include "QDroneControlManager.h"

#include "Plat/QAutopilot.h"
#include "Link/QDataLink.h"
#include "Link/QLinkManager.h"
#include "QGCSConfig.h"
#include "QGroundControlStation.h"
#include "QTestGCSConfig.h"

#include <algorithm>
#include <cmath>
#include <QSet>

namespace {
bool parseLinkConfiguration(const QVariantMap &config,
                            LinkKind &kind, LinkParams &params)
{
    const QString type = config.value(LinkConfigKeys::Type).toString();
    if (type == LinkType::Serial) {
        kind = LinkKind::Serial;
        params.portName = config.value(LinkConfigKeys::PortName).toString();
        params.baudRate = config.value(LinkConfigKeys::BaudRate).toInt();
        return !params.portName.isEmpty() && params.baudRate > 0;
    }
    if (type == LinkType::TcpServer || type == LinkType::UdpServer) {
        kind = type == LinkType::TcpServer
            ? LinkKind::TcpServer : LinkKind::UdpServer;
        params.hostName = config.value(LinkConfigKeys::HostName)
                              .toString().trimmed();
        bool portOk = false;
        const uint port = config.value(LinkConfigKeys::Port).toUInt(&portOk);
        if (!portOk || port == 0 || port > 65535) {
            return false;
        }
        params.port = static_cast<quint16>(port);
        return true;
    }
    if (type == LinkType::TcpClient || type == LinkType::UdpClient) {
        kind = type == LinkType::TcpClient
            ? LinkKind::TcpClient : LinkKind::UdpClient;
        params.hostName = config.value(LinkConfigKeys::HostName)
                              .toString().trimmed();
        bool portOk = false;
        const uint port = config.value(LinkConfigKeys::Port).toUInt(&portOk);
        if (!portOk || port == 0 || port > 65535 ||
            params.hostName.isEmpty()) {
            return false;
        }
        params.port = static_cast<quint16>(port);
        return true;
    }
    return false;
}
} // namespace

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
    connect(QGCSConfig::instance(), &QGCSConfig::warningLogMessage,
            this, [this](int, const QString &message) {
                m_businessLogs.append(message);
                constexpr qsizetype MaximumVisibleLogCount = 500;
                if (m_businessLogs.size() > MaximumVisibleLogCount) {
                    m_businessLogs.remove(
                        0, m_businessLogs.size() - MaximumVisibleLogCount);
                }
                emit businessLogsChanged();
            });
    connect(QGCSConfig::instance(), &QGCSConfig::firmwareWarningMessage,
            this, [this](quint32, int, const QString &message) {
                m_firmwareLogs.append(message);
                constexpr qsizetype MaximumVisibleLogCount = 500;
                if (m_firmwareLogs.size() > MaximumVisibleLogCount) {
                    m_firmwareLogs.remove(
                        0, m_firmwareLogs.size() - MaximumVisibleLogCount);
                }
                emit firmwareLogsChanged();
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
    connect(autopilot, &QAutopilot::actionCommandFinished,
            this, [this, systemId](QAutopilot::ActionCommand action,
                                   bool success, int,
                                   const QString &reason) {
                Command command = InvalidCommand;
                switch (action) {
                case QAutopilot::ArmAction:
                    command = ArmCommand;
                    break;
                case QAutopilot::DisarmAction:
                    command = DisarmCommand;
                    break;
                case QAutopilot::TakeoffAction:
                    command = TakeoffCommand;
                    break;
                case QAutopilot::LandAction:
                    command = LandCommand;
                    break;
                case QAutopilot::ReturnToLaunchAction:
                    command = ReturnToLaunchCommand;
                    break;
                }
                emit commandResult(systemId, command, success, reason);
            });
    connect(autopilot, &QAutopilot::airLineDownloaded,
            this, [this, systemId](const QList<QGpsPosition> &waypoints) {
                QVariantList values;
                values.reserve(waypoints.size());
                for (const QGpsPosition &waypoint : waypoints) {
                    QVariantMap value;
                    value.insert("latitude", waypoint.latitude());
                    value.insert("longitude", waypoint.longitude());
                    value.insert("altitude", waypoint.altitude());
                    values.append(value);
                }
                emit missionDownloaded(systemId, values);
            });
    connect(autopilot, &QAutopilot::airLineUploaded,
            this, [this, systemId]() {
                emit missionUploadResult(systemId, true, QString());
            });
    connect(autopilot, &QAutopilot::airLineUploadFailed,
            this, [this, systemId](const QString &reason) {
                qWarning() << tr("无人机") << systemId
                           << tr("航线上传失败:")
                           << reason;
                emit missionUploadResult(systemId, false, reason);
            });
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

QStringList QDroneControlManager::businessLogs() const
{
    return m_businessLogs;
}

QStringList QDroneControlManager::firmwareLogs() const
{
    return m_firmwareLogs;
}

QString QDroneControlManager::commandKey(Command command) const
{
    switch (command) {
    case ArmCommand:
        return QStringLiteral("arm");
    case DisarmCommand:
        return QStringLiteral("disarm");
    case TakeoffCommand:
        return QStringLiteral("takeoff");
    case LandCommand:
        return QStringLiteral("land");
    case ReturnToLaunchCommand:
        return QStringLiteral("returnToLaunch");
    case DownloadMissionCommand:
        return QStringLiteral("downloadMission");
    case UploadMissionCommand:
        return QStringLiteral("uploadMission");
    case InvalidCommand:
        break;
    }
    return {};
}

QString QDroneControlManager::commandName(int command) const
{
    const QString key = commandKey(static_cast<Command>(command));
    if (key.isEmpty()) {
        return QGCSConfig::instance()->mavsdkText(
            QStringLiteral("command"), QStringLiteral("default"));
    }
    return QGCSConfig::instance()->mavsdkText(
        QStringLiteral("command"), key);
}

QString QDroneControlManager::vehicleIcon(int vehicleType) const
{
    return QGCSConfig::instance()->mavsdkText(
        QStringLiteral("vehicleIcon"), QString::number(vehicleType));
}

void QDroneControlManager::clearBusinessLogs()
{
    if (m_businessLogs.isEmpty()) {
        return;
    }
    m_businessLogs.clear();
    emit businessLogsChanged();
}

void QDroneControlManager::clearFirmwareLogs()
{
    if (m_firmwareLogs.isEmpty()) {
        return;
    }
    m_firmwareLogs.clear();
    emit firmwareLogsChanged();
}

bool QDroneControlManager::applyConfiguredLinks()
{
    if (!m_groundStation || !m_groundStation->linkManager()) {
        emit commandRejected(tr("链路管理器尚未就绪"));
        return false;
    }

    QList<QPair<LinkKind, LinkParams>> parsedLinks;
    QSet<QString> connectionStrings;
    const QVariantList configurations =
        QTestGCSConfig::instance()->linkConfigList();
    for (qsizetype index = 0; index < configurations.size(); ++index) {
        LinkKind kind = LinkKind::Raw;
        LinkParams params;
        if (!parseLinkConfiguration(configurations.at(index).toMap(),
                                    kind, params)) {
            emit commandRejected(
                tr("第 %1 条链路配置无效").arg(index + 1));
            return false;
        }
        const QString connectionString =
            QLinkManager::buildConnectionString(kind, params);
        if (connectionString.isEmpty() ||
            connectionStrings.contains(connectionString)) {
            emit commandRejected(
                tr("第 %1 条链路为空或与其他链路重复").arg(index + 1));
            return false;
        }
        connectionStrings.insert(connectionString);
        parsedLinks.append(qMakePair(kind, params));
    }

    QLinkManager *const manager = m_groundStation->linkManager();
    manager->clearAll();
    for (const auto &entry : parsedLinks) {
        if (!manager->addLink(entry.first, entry.second)) {
            emit commandRejected(tr("应用链路配置失败，请检查日志"));
            return false;
        }
    }

    m_businessLogs.append(
        tr("已应用 %1 条链路配置并重新连接").arg(parsedLinks.size()));
    emit businessLogsChanged();
    return true;
}

void QDroneControlManager::renameDrone(int systemId, const QString &name)
{
    if (!m_autopilots.contains(systemId)) {
        emit commandRejected(tr("未找到系统 ID %1").arg(systemId));
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
        emit commandRejected(tr("编组名称为空或已经存在"));
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
    QAutopilot *autopilot, Command command)
{
    if (!autopilot || !autopilot->isConnected()) {
        return false;
    }
    switch (command) {
    case ArmCommand:
        autopilot->arm();
        return true;
    case DisarmCommand:
        autopilot->disarm();
        return true;
    case TakeoffCommand:
        autopilot->takeoff();
        return true;
    case LandCommand:
        autopilot->land();
        return true;
    case ReturnToLaunchCommand:
        autopilot->returnToLaunch();
        return true;
    case DownloadMissionCommand:
        if (autopilot->airLineDownloading() ||
            autopilot->airLineUploading()) {
            return false;
        }
        autopilot->downloadAirLine();
        return true;
    case UploadMissionCommand:
    case InvalidCommand:
        return false;
    }
    return false;
}

bool QDroneControlManager::executeSingle(
    int systemId, int commandValue)
{
    const Command command = static_cast<Command>(commandValue);
    QAutopilot *autopilot = m_autopilots.value(systemId);
    if (!execute(autopilot, command)) {
        emit commandRejected(
            tr("无人机 %1 离线或命令不受支持").arg(systemId));
        return false;
    }
    emit commandDispatched(
        command, QTestGCSConfig::instance()->droneName(systemId), 1);
    return true;
}

bool QDroneControlManager::executeGroup(
    const QString &groupName, int commandValue)
{
    const Command command = static_cast<Command>(commandValue);
    const QVariantList members = groupMembers(groupName);
    int dispatched = 0;
    for (const QVariant &member : members) {
        QAutopilot *autopilot = m_autopilots.value(member.toInt());
        if (execute(autopilot, command)) {
            ++dispatched;
        }
    }

    if (dispatched == 0) {
        emit commandRejected(tr("编组中没有可执行该命令的在线无人机"));
        return false;
    }
    emit commandDispatched(command, groupName, dispatched);
    return true;
}

bool QDroneControlManager::parseWaypoints(
    const QVariantList &values, QList<QGpsPosition> &waypoints,
    QString &reason) const
{
    if (values.isEmpty()) {
        reason = tr("航线至少需要一个航点");
        return false;
    }

    waypoints.clear();
    waypoints.reserve(values.size());
    const auto *config = QTestGCSConfig::instance();
    const double minimumAltitude = config->missionMinimumAltitude();
    const double maximumAltitude = config->missionMaximumAltitude();
    for (qsizetype index = 0; index < values.size(); ++index) {
        const QVariantMap value = values.at(index).toMap();
        bool latitudeOk = false;
        bool longitudeOk = false;
        bool altitudeOk = false;
        const double latitude =
            value.value("latitude").toDouble(&latitudeOk);
        const double longitude =
            value.value("longitude").toDouble(&longitudeOk);
        const double altitude =
            value.value("altitude").toDouble(&altitudeOk);
        if (!latitudeOk || !longitudeOk || !altitudeOk ||
            !std::isfinite(latitude) || !std::isfinite(longitude) ||
            !std::isfinite(altitude) ||
            latitude < -90.0 || latitude > 90.0 ||
            longitude < -180.0 || longitude > 180.0 ||
            altitude < minimumAltitude || altitude > maximumAltitude) {
            reason = tr("第 %1 个航点坐标或高度无效")
                         .arg(index + 1);
            return false;
        }
        waypoints.append(QGpsPosition(longitude, latitude, altitude));
    }
    return true;
}

bool QDroneControlManager::uploadMissionSingle(
    int systemId, const QVariantList &values)
{
    QList<QGpsPosition> waypoints;
    QString reason;
    if (!parseWaypoints(values, waypoints, reason)) {
        emit commandRejected(reason);
        return false;
    }

    QAutopilot *autopilot = m_autopilots.value(systemId);
    if (!autopilot || !autopilot->isConnected() ||
        autopilot->airLineUploading() ||
        autopilot->airLineDownloading()) {
        emit commandRejected(
            tr("无人机 %1 离线或正在上传航线").arg(systemId));
        return false;
    }

    autopilot->uploadAirLine(waypoints);
    emit commandDispatched(
        UploadMissionCommand,
        QTestGCSConfig::instance()->droneName(systemId), 1);
    return true;
}

bool QDroneControlManager::uploadMissionGroup(
    const QString &groupName, const QVariantList &values)
{
    QList<QGpsPosition> waypoints;
    QString reason;
    if (!parseWaypoints(values, waypoints, reason)) {
        emit commandRejected(reason);
        return false;
    }

    int dispatched = 0;
    for (const QVariant &member : groupMembers(groupName)) {
        QAutopilot *autopilot = m_autopilots.value(member.toInt());
        if (!autopilot || !autopilot->isConnected() ||
            autopilot->airLineUploading() ||
            autopilot->airLineDownloading()) {
            continue;
        }
        autopilot->uploadAirLine(waypoints);
        ++dispatched;
    }
    if (dispatched == 0) {
        emit commandRejected(
            tr("编组中没有可上传航线的在线无人机"));
        return false;
    }
    emit commandDispatched(
        UploadMissionCommand, groupName, dispatched);
    return true;
}
