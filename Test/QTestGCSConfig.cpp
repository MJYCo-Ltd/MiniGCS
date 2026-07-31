#include <QSettings>
#include "QTestGCSConfig.h"
#include <QtSerialPort/QSerialPortInfo>

namespace {
const char *KEY_MAP_NAME = "Map/Name";
const char *KEY_MAP_CENTER_LATITUDE = "Map/CenterLatitude";
const char *KEY_MAP_CENTER_LONGITUDE = "Map/CenterLongitude";
const char *KEY_MAP_INITIAL_ZOOM = "Map/InitialZoom";
const char *KEY_MAP_VEHICLE_ZOOM = "Map/VehicleZoom";
const char *KEY_MAP_MINIMUM_ZOOM = "Map/MinimumZoom";
const char *KEY_MAP_MAXIMUM_ZOOM = "Map/MaximumZoom";
const char *KEY_MISSION_DEFAULT_ALTITUDE = "Mission/DefaultAltitude";
const char *KEY_MISSION_MINIMUM_ALTITUDE = "Mission/MinimumAltitude";
const char *KEY_MISSION_MAXIMUM_ALTITUDE = "Mission/MaximumAltitude";
const char *KEY_LINKS_COUNT = "Links/Count";
const char *KEY_LINK_GROUP_PREFIX = "Link";
const char *KEY_DRONE_GROUPS_COUNT = "DroneGroups/Count";
const char *KEY_DRONE_GROUP_PREFIX = "DroneGroup";

const char *DEFAULT_MAP_NAME = "QGroundControl";
constexpr double DEFAULT_MAP_CENTER_LATITUDE = 38.045474;
constexpr double DEFAULT_MAP_CENTER_LONGITUDE = 114.502461;
constexpr double DEFAULT_MAP_INITIAL_ZOOM = 10.0;
constexpr double DEFAULT_MAP_VEHICLE_ZOOM = 16.0;
constexpr double DEFAULT_MAP_MINIMUM_ZOOM = 3.0;
constexpr double DEFAULT_MAP_MAXIMUM_ZOOM = 18.0;
constexpr double DEFAULT_MISSION_ALTITUDE = 30.0;
constexpr double DEFAULT_MISSION_MINIMUM_ALTITUDE = -1000.0;
constexpr double DEFAULT_MISSION_MAXIMUM_ALTITUDE = 10000.0;
} // namespace

QTestGCSConfig *QTestGCSConfig::s_instance = nullptr;

QTestGCSConfig::QTestGCSConfig(QObject *parent) : QGCSConfig(parent) {}

QTestGCSConfig::~QTestGCSConfig() = default;

QTestGCSConfig *QTestGCSConfig::instance()
{
    if (s_instance == nullptr) {
        s_instance = new QTestGCSConfig;
        QGCSConfig::setInstance(s_instance);
    }
    return s_instance;
}

QStringList QTestGCSConfig::refreshPortName() const
{
    QStringList portNames;
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        portNames.append(port.portName());
    }
    return portNames;
}

QStringList QTestGCSConfig::standardBaudRates() const
{
    QStringList baudRates;
    const auto standard = QSerialPortInfo::standardBaudRates();
    for (qint32 baudRate : standard) {
        baudRates.append(QString::number(baudRate));
    }
    return baudRates;
}

QString QTestGCSConfig::mapName() const
{
    if (!m_settings)
        return QString(DEFAULT_MAP_NAME);
    return m_settings->value(KEY_MAP_NAME, DEFAULT_MAP_NAME).toString();
}

void QTestGCSConfig::setMapName(const QString &mapName)
{
    if (m_settings)
        m_settings->setValue(KEY_MAP_NAME, mapName);
}

void QTestGCSConfig::release()
{
    QGCSConfig::release();
    s_instance = nullptr;
}

QString QTestGCSConfig::linkGroupKey(int index) const
{
    return QString("%1%2").arg(KEY_LINK_GROUP_PREFIX).arg(index);
}

int QTestGCSConfig::linkCount() const
{
    if (!m_settings)
        return 0;
    return m_settings->value(KEY_LINKS_COUNT, 0).toInt();
}

QVariantMap QTestGCSConfig::linkConfigAt(int index) const
{
    QVariantMap out;
    if (!m_settings || index < 0 || index >= linkCount())
        return out;
    const QString group = linkGroupKey(index);
    m_settings->beginGroup(group);
    out.insert(LinkConfigKeys::Type, m_settings->value(LinkConfigKeys::Type).toString());
    out.insert(LinkConfigKeys::Name, m_settings->value(LinkConfigKeys::Name).toString());
    out.insert(LinkConfigKeys::PortName, m_settings->value(LinkConfigKeys::PortName).toString());
    out.insert(LinkConfigKeys::BaudRate, m_settings->value(LinkConfigKeys::BaudRate).toInt());
    out.insert(LinkConfigKeys::HostName, m_settings->value(LinkConfigKeys::HostName).toString());
    out.insert(LinkConfigKeys::Port, m_settings->value(LinkConfigKeys::Port).toInt());
    m_settings->endGroup();
    return out;
}

void QTestGCSConfig::setLinkConfigAt(int index, const QVariantMap &config)
{
    if (!m_settings || index < 0 || index >= linkCount())
        return;
    const QString group = linkGroupKey(index);
    m_settings->beginGroup(group);
    if (config.contains(LinkConfigKeys::Type))
        m_settings->setValue(LinkConfigKeys::Type, config.value(LinkConfigKeys::Type).toString());
    if (config.contains(LinkConfigKeys::Name))
        m_settings->setValue(LinkConfigKeys::Name, config.value(LinkConfigKeys::Name).toString());
    if (config.contains(LinkConfigKeys::PortName))
        m_settings->setValue(LinkConfigKeys::PortName, config.value(LinkConfigKeys::PortName).toString());
    if (config.contains(LinkConfigKeys::BaudRate))
        m_settings->setValue(LinkConfigKeys::BaudRate, config.value(LinkConfigKeys::BaudRate).toInt());
    if (config.contains(LinkConfigKeys::HostName))
        m_settings->setValue(LinkConfigKeys::HostName, config.value(LinkConfigKeys::HostName).toString());
    if (config.contains(LinkConfigKeys::Port))
        m_settings->setValue(LinkConfigKeys::Port, config.value(LinkConfigKeys::Port).toInt());
    m_settings->endGroup();
    m_settings->sync();
}

void QTestGCSConfig::appendLinkConfig(const QVariantMap &config)
{
    if (!m_settings)
        return;
    const int count = linkCount();
    const QString group = linkGroupKey(count);
    m_settings->setValue(KEY_LINKS_COUNT, count + 1);
    m_settings->beginGroup(group);
    m_settings->setValue(LinkConfigKeys::Type, config.value(LinkConfigKeys::Type).toString());
    m_settings->setValue(LinkConfigKeys::Name, config.value(LinkConfigKeys::Name).toString());
    m_settings->setValue(LinkConfigKeys::PortName, config.value(LinkConfigKeys::PortName).toString());
    m_settings->setValue(LinkConfigKeys::BaudRate, config.value(LinkConfigKeys::BaudRate).toInt());
    m_settings->setValue(LinkConfigKeys::HostName, config.value(LinkConfigKeys::HostName).toString());
    m_settings->setValue(LinkConfigKeys::Port, config.value(LinkConfigKeys::Port).toInt());
    m_settings->endGroup();
    m_settings->sync();
}

void QTestGCSConfig::removeLinkConfigAt(int index)
{
    if (!m_settings || index < 0)
        return;
    const int count = linkCount();
    if (index >= count)
        return;
    for (int i = index; i < count - 1; ++i) {
        QVariantMap next = linkConfigAt(i + 1);
        setLinkConfigAt(i, next);
    }
    const QString lastGroup = linkGroupKey(count - 1);
    m_settings->beginGroup(lastGroup);
    for (const QString &key : m_settings->childKeys())
        m_settings->remove(key);
    m_settings->endGroup();
    m_settings->setValue(KEY_LINKS_COUNT, count - 1);
    m_settings->sync();
}

QVariantList QTestGCSConfig::linkConfigList() const
{
    QVariantList list;
    const int n = linkCount();
    for (int i = 0; i < n; ++i)
        list.append(linkConfigAt(i));
    return list;
}

void QTestGCSConfig::saveLinkConfigs()
{
    if (m_settings)
        m_settings->sync();
}

double QTestGCSConfig::mapCenterLatitude() const
{
    return m_settings
        ? m_settings->value(KEY_MAP_CENTER_LATITUDE,
                            DEFAULT_MAP_CENTER_LATITUDE).toDouble()
        : DEFAULT_MAP_CENTER_LATITUDE;
}

double QTestGCSConfig::mapCenterLongitude() const
{
    return m_settings
        ? m_settings->value(KEY_MAP_CENTER_LONGITUDE,
                            DEFAULT_MAP_CENTER_LONGITUDE).toDouble()
        : DEFAULT_MAP_CENTER_LONGITUDE;
}

double QTestGCSConfig::mapInitialZoom() const
{
    return m_settings
        ? m_settings->value(KEY_MAP_INITIAL_ZOOM,
                            DEFAULT_MAP_INITIAL_ZOOM).toDouble()
        : DEFAULT_MAP_INITIAL_ZOOM;
}

double QTestGCSConfig::mapVehicleZoom() const
{
    return m_settings
        ? m_settings->value(KEY_MAP_VEHICLE_ZOOM,
                            DEFAULT_MAP_VEHICLE_ZOOM).toDouble()
        : DEFAULT_MAP_VEHICLE_ZOOM;
}

double QTestGCSConfig::mapMinimumZoom() const
{
    return m_settings
        ? m_settings->value(KEY_MAP_MINIMUM_ZOOM,
                            DEFAULT_MAP_MINIMUM_ZOOM).toDouble()
        : DEFAULT_MAP_MINIMUM_ZOOM;
}

double QTestGCSConfig::mapMaximumZoom() const
{
    return m_settings
        ? m_settings->value(KEY_MAP_MAXIMUM_ZOOM,
                            DEFAULT_MAP_MAXIMUM_ZOOM).toDouble()
        : DEFAULT_MAP_MAXIMUM_ZOOM;
}

double QTestGCSConfig::missionDefaultAltitude() const
{
    return m_settings
        ? m_settings->value(KEY_MISSION_DEFAULT_ALTITUDE,
                            DEFAULT_MISSION_ALTITUDE).toDouble()
        : DEFAULT_MISSION_ALTITUDE;
}

double QTestGCSConfig::missionMinimumAltitude() const
{
    return m_settings
        ? m_settings->value(KEY_MISSION_MINIMUM_ALTITUDE,
                            DEFAULT_MISSION_MINIMUM_ALTITUDE).toDouble()
        : DEFAULT_MISSION_MINIMUM_ALTITUDE;
}

double QTestGCSConfig::missionMaximumAltitude() const
{
    return m_settings
        ? m_settings->value(KEY_MISSION_MAXIMUM_ALTITUDE,
                            DEFAULT_MISSION_MAXIMUM_ALTITUDE).toDouble()
        : DEFAULT_MISSION_MAXIMUM_ALTITUDE;
}

QString QTestGCSConfig::droneName(int systemId) const
{
    if (!m_settings || systemId < 0 || systemId > 255) {
        return {};
    }
    return m_settings
        ->value(QString("Drones/%1/Name").arg(systemId),
                tr("无人机 %1").arg(systemId))
        .toString();
}

void QTestGCSConfig::setDroneName(int systemId, const QString &name)
{
    if (!m_settings || systemId < 0 || systemId > 255) {
        return;
    }
    const QString normalized = name.trimmed();
    m_settings->setValue(
        QString("Drones/%1/Name").arg(systemId),
        normalized.isEmpty() ? tr("无人机 %1").arg(systemId) : normalized);
    m_settings->sync();
}

QString QTestGCSConfig::droneGroupKey(int index) const
{
    return QString("%1%2").arg(KEY_DRONE_GROUP_PREFIX).arg(index);
}

int QTestGCSConfig::findDroneGroup(const QString &name) const
{
    if (!m_settings) {
        return -1;
    }
    const QString normalized = name.trimmed();
    const int count = m_settings->value(KEY_DRONE_GROUPS_COUNT, 0).toInt();
    for (int index = 0; index < count; ++index) {
        m_settings->beginGroup(droneGroupKey(index));
        const QString currentName = m_settings->value("Name").toString();
        m_settings->endGroup();
        if (currentName == normalized) {
            return index;
        }
    }
    return -1;
}

QVariantList QTestGCSConfig::droneGroupList() const
{
    QVariantList groups;
    if (!m_settings) {
        return groups;
    }
    const int count = m_settings->value(KEY_DRONE_GROUPS_COUNT, 0).toInt();
    for (int index = 0; index < count; ++index) {
        m_settings->beginGroup(droneGroupKey(index));
        QVariantMap group;
        group.insert("name", m_settings->value("Name").toString());
        QVariantList members;
        const QStringList storedMembers =
            m_settings->value("Members").toStringList();
        for (const QString &member : storedMembers) {
            bool ok = false;
            const int systemId = member.toInt(&ok);
            if (ok && systemId >= 0 && systemId <= 255) {
                members.append(systemId);
            }
        }
        group.insert("members", members);
        m_settings->endGroup();
        groups.append(group);
    }
    return groups;
}

bool QTestGCSConfig::addDroneGroup(const QString &name)
{
    if (!m_settings) {
        return false;
    }
    const QString normalized = name.trimmed();
    if (normalized.isEmpty() || findDroneGroup(normalized) >= 0) {
        return false;
    }
    const int count = m_settings->value(KEY_DRONE_GROUPS_COUNT, 0).toInt();
    m_settings->beginGroup(droneGroupKey(count));
    m_settings->setValue("Name", normalized);
    m_settings->setValue("Members", QStringList());
    m_settings->endGroup();
    m_settings->setValue(KEY_DRONE_GROUPS_COUNT, count + 1);
    m_settings->sync();
    return true;
}

bool QTestGCSConfig::removeDroneGroup(const QString &name)
{
    const int index = findDroneGroup(name);
    if (!m_settings || index < 0) {
        return false;
    }
    const int count = m_settings->value(KEY_DRONE_GROUPS_COUNT, 0).toInt();
    for (int current = index; current < count - 1; ++current) {
        m_settings->beginGroup(droneGroupKey(current + 1));
        const QString nextName = m_settings->value("Name").toString();
        const QStringList nextMembers =
            m_settings->value("Members").toStringList();
        m_settings->endGroup();

        m_settings->beginGroup(droneGroupKey(current));
        m_settings->setValue("Name", nextName);
        m_settings->setValue("Members", nextMembers);
        m_settings->endGroup();
    }
    m_settings->remove(droneGroupKey(count - 1));
    m_settings->setValue(KEY_DRONE_GROUPS_COUNT, count - 1);
    m_settings->sync();
    return true;
}

bool QTestGCSConfig::setDroneGroupMembers(
    const QString &name, const QVariantList &systemIds)
{
    const int index = findDroneGroup(name);
    if (!m_settings || index < 0) {
        return false;
    }
    QStringList members;
    for (const QVariant &value : systemIds) {
        bool ok = false;
        const int systemId = value.toInt(&ok);
        if (ok && systemId >= 0 && systemId <= 255) {
            const QString id = QString::number(systemId);
            if (!members.contains(id)) {
                members.append(id);
            }
        }
    }
    m_settings->beginGroup(droneGroupKey(index));
    m_settings->setValue("Members", members);
    m_settings->endGroup();
    m_settings->sync();
    return true;
}

void QTestGCSConfig::initializeDefaults()
{
    QGCSConfig::initializeDefaults();
    if (!m_settings)
        return;
    const QString configuredMapName =
        m_settings->value(KEY_MAP_NAME).toString().trimmed();
    if (!m_settings->contains(KEY_MAP_NAME) || configuredMapName.isEmpty()) {
        m_settings->setValue(KEY_MAP_NAME, DEFAULT_MAP_NAME);
    }
    if (!m_settings->contains(KEY_MAP_CENTER_LATITUDE)) {
        m_settings->setValue(KEY_MAP_CENTER_LATITUDE,
                             DEFAULT_MAP_CENTER_LATITUDE);
    }
    if (!m_settings->contains(KEY_MAP_CENTER_LONGITUDE)) {
        m_settings->setValue(KEY_MAP_CENTER_LONGITUDE,
                             DEFAULT_MAP_CENTER_LONGITUDE);
    }
    if (!m_settings->contains(KEY_MAP_INITIAL_ZOOM)) {
        m_settings->setValue(KEY_MAP_INITIAL_ZOOM,
                             DEFAULT_MAP_INITIAL_ZOOM);
    }
    if (!m_settings->contains(KEY_MAP_VEHICLE_ZOOM)) {
        m_settings->setValue(KEY_MAP_VEHICLE_ZOOM,
                             DEFAULT_MAP_VEHICLE_ZOOM);
    }
    if (!m_settings->contains(KEY_MAP_MINIMUM_ZOOM)) {
        m_settings->setValue(KEY_MAP_MINIMUM_ZOOM,
                             DEFAULT_MAP_MINIMUM_ZOOM);
    }
    if (!m_settings->contains(KEY_MAP_MAXIMUM_ZOOM)) {
        m_settings->setValue(KEY_MAP_MAXIMUM_ZOOM,
                             DEFAULT_MAP_MAXIMUM_ZOOM);
    }
    if (!m_settings->contains(KEY_MISSION_DEFAULT_ALTITUDE)) {
        m_settings->setValue(KEY_MISSION_DEFAULT_ALTITUDE,
                             DEFAULT_MISSION_ALTITUDE);
    }
    if (!m_settings->contains(KEY_MISSION_MINIMUM_ALTITUDE)) {
        m_settings->setValue(KEY_MISSION_MINIMUM_ALTITUDE,
                             DEFAULT_MISSION_MINIMUM_ALTITUDE);
    }
    if (!m_settings->contains(KEY_MISSION_MAXIMUM_ALTITUDE)) {
        m_settings->setValue(KEY_MISSION_MAXIMUM_ALTITUDE,
                             DEFAULT_MISSION_MAXIMUM_ALTITUDE);
    }
    if (!m_settings->contains(KEY_LINKS_COUNT)) {
        m_settings->setValue(KEY_LINKS_COUNT, 0);
    }
    if (!m_settings->contains(KEY_DRONE_GROUPS_COUNT)) {
        m_settings->setValue(KEY_DRONE_GROUPS_COUNT, 0);
    }
    m_settings->sync();
}
