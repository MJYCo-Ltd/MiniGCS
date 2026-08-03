#include <QSettings>
#include "QTestGCSConfig.h"
#include <QtSerialPort/QSerialPortInfo>

namespace {
const char *KEY_LINK_DEFAULT_BAUD_RATE = "Link/DefaultBaudRate";
const char *KEY_LOGGING_MAXIMUM_VISIBLE_COUNT =
    "Logging/MaximumVisibleCount";
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
const char *KEY_FLIGHT_RECORD_MINIMUM_SAMPLE_INTERVAL_MS =
    "FlightRecord/MinimumSampleIntervalMs";
const char *KEY_FLIGHT_RECORD_MINIMUM_SAMPLE_DISTANCE_M =
    "FlightRecord/MinimumSampleDistanceM";
const char *KEY_FLIGHT_RECORD_MAXIMUM_COUNT =
    "FlightRecord/MaximumCount";
const char *KEY_LINKS_COUNT = "Links/Count";
const char *KEY_LINK_GROUP_PREFIX = "link";
const char *KEY_DRONE_GROUPS_COUNT = "DroneGroups/Count";
const char *KEY_DRONE_GROUP_PREFIX = "DroneGroup";

// INI 分组内键名（PascalCase，与 Config/*.ini 保持一致）
const char *INI_LINK_TYPE = "Type";
const char *INI_LINK_NAME = "Name";
const char *INI_LINK_PORT_NAME = "PortName";
const char *INI_LINK_BAUD_RATE = "BaudRate";
const char *INI_LINK_HOST_NAME = "HostName";
const char *INI_LINK_PORT = "Port";

QVariant linkMapValue(const QVariantMap &config,
                      const char *camelKey, const char *pascalKey)
{
    if (config.contains(QLatin1String(camelKey)))
        return config.value(QLatin1String(camelKey));
    return config.value(QLatin1String(pascalKey));
}

bool linkMapContains(const QVariantMap &config,
                     const char *camelKey, const char *pascalKey)
{
    return config.contains(QLatin1String(camelKey))
        || config.contains(QLatin1String(pascalKey));
}

constexpr int DEFAULT_LINK_BAUD_RATE = 115200;
constexpr int DEFAULT_MAXIMUM_VISIBLE_LOG_COUNT = 500;
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
constexpr qint64 DEFAULT_FLIGHT_RECORD_MINIMUM_SAMPLE_INTERVAL_MS = 1000;
constexpr double DEFAULT_FLIGHT_RECORD_MINIMUM_SAMPLE_DISTANCE_M = 2.0;
constexpr int DEFAULT_FLIGHT_RECORD_MAXIMUM_COUNT = 200;
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

int QTestGCSConfig::defaultBaudRate() const
{
    if (!m_settings) {
        return DEFAULT_LINK_BAUD_RATE;
    }
    return m_settings
        ->value(KEY_LINK_DEFAULT_BAUD_RATE, DEFAULT_LINK_BAUD_RATE)
        .toInt();
}

int QTestGCSConfig::maximumVisibleLogCount() const
{
    if (!m_settings) {
        return DEFAULT_MAXIMUM_VISIBLE_LOG_COUNT;
    }
    return qMax(
        1, m_settings
               ->value(KEY_LOGGING_MAXIMUM_VISIBLE_COUNT,
                       DEFAULT_MAXIMUM_VISIBLE_LOG_COUNT)
               .toInt());
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

QString QTestGCSConfig::resolveLinkGroup(int index) const
{
    const QString primary = linkGroupKey(index);
    if (!m_settings)
        return primary;

    m_settings->beginGroup(primary);
    const bool primaryOk = !m_settings->value(INI_LINK_TYPE).toString().isEmpty();
    m_settings->endGroup();
    if (primaryOk)
        return primary;

    // 兼容旧版大写前缀 Link0（Linux 上 QSettings 区分大小写）
    const QString legacy = QStringLiteral("Link%1").arg(index);
    m_settings->beginGroup(legacy);
    const bool legacyOk = !m_settings->value(INI_LINK_TYPE).toString().isEmpty();
    m_settings->endGroup();
    return legacyOk ? legacy : primary;
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
    const QString group = resolveLinkGroup(index);
    m_settings->beginGroup(group);
    out.insert(LinkConfigKeys::Type,
               m_settings->value(INI_LINK_TYPE).toString());
    out.insert(LinkConfigKeys::Name,
               m_settings->value(INI_LINK_NAME).toString());
    out.insert(LinkConfigKeys::PortName,
               m_settings->value(INI_LINK_PORT_NAME).toString());
    out.insert(LinkConfigKeys::BaudRate,
               m_settings->value(INI_LINK_BAUD_RATE).toInt());
    out.insert(LinkConfigKeys::HostName,
               m_settings->value(INI_LINK_HOST_NAME).toString());
    out.insert(LinkConfigKeys::Port,
               m_settings->value(INI_LINK_PORT).toInt());
    m_settings->endGroup();
    return out;
}

void QTestGCSConfig::setLinkConfigAt(int index, const QVariantMap &config)
{
    if (!m_settings || index < 0 || index >= linkCount())
        return;
    const QString group = linkGroupKey(index);
    m_settings->beginGroup(group);
    if (linkMapContains(config, LinkConfigKeys::Type, INI_LINK_TYPE))
        m_settings->setValue(INI_LINK_TYPE,
                             linkMapValue(config, LinkConfigKeys::Type,
                                          INI_LINK_TYPE).toString());
    if (linkMapContains(config, LinkConfigKeys::Name, INI_LINK_NAME))
        m_settings->setValue(INI_LINK_NAME,
                             linkMapValue(config, LinkConfigKeys::Name,
                                          INI_LINK_NAME).toString());
    if (linkMapContains(config, LinkConfigKeys::PortName, INI_LINK_PORT_NAME))
        m_settings->setValue(INI_LINK_PORT_NAME,
                             linkMapValue(config, LinkConfigKeys::PortName,
                                          INI_LINK_PORT_NAME).toString());
    if (linkMapContains(config, LinkConfigKeys::BaudRate, INI_LINK_BAUD_RATE))
        m_settings->setValue(INI_LINK_BAUD_RATE,
                             linkMapValue(config, LinkConfigKeys::BaudRate,
                                          INI_LINK_BAUD_RATE).toInt());
    if (linkMapContains(config, LinkConfigKeys::HostName, INI_LINK_HOST_NAME))
        m_settings->setValue(INI_LINK_HOST_NAME,
                             linkMapValue(config, LinkConfigKeys::HostName,
                                          INI_LINK_HOST_NAME).toString());
    if (linkMapContains(config, LinkConfigKeys::Port, INI_LINK_PORT))
        m_settings->setValue(INI_LINK_PORT,
                             linkMapValue(config, LinkConfigKeys::Port,
                                          INI_LINK_PORT).toInt());
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
    m_settings->setValue(INI_LINK_TYPE,
                         linkMapValue(config, LinkConfigKeys::Type,
                                      INI_LINK_TYPE).toString());
    m_settings->setValue(INI_LINK_NAME,
                         linkMapValue(config, LinkConfigKeys::Name,
                                      INI_LINK_NAME).toString());
    m_settings->setValue(INI_LINK_PORT_NAME,
                         linkMapValue(config, LinkConfigKeys::PortName,
                                      INI_LINK_PORT_NAME).toString());
    m_settings->setValue(INI_LINK_BAUD_RATE,
                         linkMapValue(config, LinkConfigKeys::BaudRate,
                                      INI_LINK_BAUD_RATE).toInt());
    m_settings->setValue(INI_LINK_HOST_NAME,
                         linkMapValue(config, LinkConfigKeys::HostName,
                                      INI_LINK_HOST_NAME).toString());
    m_settings->setValue(INI_LINK_PORT,
                         linkMapValue(config, LinkConfigKeys::Port,
                                      INI_LINK_PORT).toInt());
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
    const int last = count - 1;
    m_settings->remove(linkGroupKey(last));
    m_settings->remove(QStringLiteral("Link%1").arg(last));
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

qint64 QTestGCSConfig::flightRecordMinimumSampleIntervalMs() const
{
    return m_settings
        ? qMax<qint64>(100, m_settings->value(
              KEY_FLIGHT_RECORD_MINIMUM_SAMPLE_INTERVAL_MS,
              DEFAULT_FLIGHT_RECORD_MINIMUM_SAMPLE_INTERVAL_MS).toLongLong())
        : DEFAULT_FLIGHT_RECORD_MINIMUM_SAMPLE_INTERVAL_MS;
}

double QTestGCSConfig::flightRecordMinimumSampleDistanceM() const
{
    return m_settings
        ? qMax(0.0, m_settings->value(
              KEY_FLIGHT_RECORD_MINIMUM_SAMPLE_DISTANCE_M,
              DEFAULT_FLIGHT_RECORD_MINIMUM_SAMPLE_DISTANCE_M).toDouble())
        : DEFAULT_FLIGHT_RECORD_MINIMUM_SAMPLE_DISTANCE_M;
}

int QTestGCSConfig::flightRecordMaximumCount() const
{
    return m_settings
        ? qMax(1, m_settings->value(
              KEY_FLIGHT_RECORD_MAXIMUM_COUNT,
              DEFAULT_FLIGHT_RECORD_MAXIMUM_COUNT).toInt())
        : DEFAULT_FLIGHT_RECORD_MAXIMUM_COUNT;
}

bool QTestGCSConfig::setMapConfiguration(const QVariantMap &config)
{
    if (!m_settings) {
        return false;
    }

    const QString mapName = config.value(QStringLiteral("mapName"))
                                .toString().trimmed();
    bool latitudeOk = false;
    bool longitudeOk = false;
    bool initialZoomOk = false;
    bool vehicleZoomOk = false;
    bool minimumZoomOk = false;
    bool maximumZoomOk = false;
    bool defaultAltitudeOk = false;
    bool minimumAltitudeOk = false;
    bool maximumAltitudeOk = false;
    const double latitude = config.value(QStringLiteral("centerLatitude"))
                                .toDouble(&latitudeOk);
    const double longitude = config.value(QStringLiteral("centerLongitude"))
                                 .toDouble(&longitudeOk);
    const double initialZoom = config.value(QStringLiteral("initialZoom"))
                                   .toDouble(&initialZoomOk);
    const double vehicleZoom = config.value(QStringLiteral("vehicleZoom"))
                                   .toDouble(&vehicleZoomOk);
    const double minimumZoom = config.value(QStringLiteral("minimumZoom"))
                                   .toDouble(&minimumZoomOk);
    const double maximumZoom = config.value(QStringLiteral("maximumZoom"))
                                   .toDouble(&maximumZoomOk);
    const double defaultAltitude = config.value(
        QStringLiteral("defaultAltitude")).toDouble(&defaultAltitudeOk);
    const double minimumAltitude = config.value(
        QStringLiteral("minimumAltitude")).toDouble(&minimumAltitudeOk);
    const double maximumAltitude = config.value(
        QStringLiteral("maximumAltitude")).toDouble(&maximumAltitudeOk);

    const bool valid = !mapName.isEmpty() && latitudeOk && longitudeOk &&
        initialZoomOk && vehicleZoomOk && minimumZoomOk && maximumZoomOk &&
        defaultAltitudeOk && minimumAltitudeOk && maximumAltitudeOk &&
        latitude >= -90.0 && latitude <= 90.0 &&
        longitude >= -180.0 && longitude <= 180.0 &&
        minimumZoom >= 0.0 && maximumZoom <= 30.0 &&
        minimumZoom <= initialZoom && initialZoom <= maximumZoom &&
        minimumZoom <= vehicleZoom && vehicleZoom <= maximumZoom &&
        minimumAltitude <= defaultAltitude &&
        defaultAltitude <= maximumAltitude;
    if (!valid) {
        return false;
    }

    m_settings->setValue(KEY_MAP_NAME, mapName);
    m_settings->setValue(KEY_MAP_CENTER_LATITUDE, latitude);
    m_settings->setValue(KEY_MAP_CENTER_LONGITUDE, longitude);
    m_settings->setValue(KEY_MAP_INITIAL_ZOOM, initialZoom);
    m_settings->setValue(KEY_MAP_VEHICLE_ZOOM, vehicleZoom);
    m_settings->setValue(KEY_MAP_MINIMUM_ZOOM, minimumZoom);
    m_settings->setValue(KEY_MAP_MAXIMUM_ZOOM, maximumZoom);
    m_settings->setValue(KEY_MISSION_DEFAULT_ALTITUDE, defaultAltitude);
    m_settings->setValue(KEY_MISSION_MINIMUM_ALTITUDE, minimumAltitude);
    m_settings->setValue(KEY_MISSION_MAXIMUM_ALTITUDE, maximumAltitude);
    m_settings->sync();
    emit mapConfigurationChanged();
    return true;
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
    if (!m_settings->contains(KEY_LINK_DEFAULT_BAUD_RATE)) {
        m_settings->setValue(KEY_LINK_DEFAULT_BAUD_RATE,
                             DEFAULT_LINK_BAUD_RATE);
    }
    if (!m_settings->contains(KEY_LOGGING_MAXIMUM_VISIBLE_COUNT)) {
        m_settings->setValue(KEY_LOGGING_MAXIMUM_VISIBLE_COUNT,
                             DEFAULT_MAXIMUM_VISIBLE_LOG_COUNT);
    }
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
    if (!m_settings->contains(
            KEY_FLIGHT_RECORD_MINIMUM_SAMPLE_INTERVAL_MS)) {
        m_settings->setValue(
            KEY_FLIGHT_RECORD_MINIMUM_SAMPLE_INTERVAL_MS,
            DEFAULT_FLIGHT_RECORD_MINIMUM_SAMPLE_INTERVAL_MS);
    }
    if (!m_settings->contains(
            KEY_FLIGHT_RECORD_MINIMUM_SAMPLE_DISTANCE_M)) {
        m_settings->setValue(
            KEY_FLIGHT_RECORD_MINIMUM_SAMPLE_DISTANCE_M,
            DEFAULT_FLIGHT_RECORD_MINIMUM_SAMPLE_DISTANCE_M);
    }
    if (!m_settings->contains(KEY_FLIGHT_RECORD_MAXIMUM_COUNT)) {
        m_settings->setValue(KEY_FLIGHT_RECORD_MAXIMUM_COUNT,
                             DEFAULT_FLIGHT_RECORD_MAXIMUM_COUNT);
    }
    if (!m_settings->contains(KEY_LINKS_COUNT)) {
        m_settings->setValue(KEY_LINKS_COUNT, 0);
    }
    if (!m_settings->contains(KEY_DRONE_GROUPS_COUNT)) {
        m_settings->setValue(KEY_DRONE_GROUPS_COUNT, 0);
    }
    m_settings->sync();
}
