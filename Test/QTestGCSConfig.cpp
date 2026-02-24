#include <QSettings>
#include "QTestGCSConfig.h"
#include <QtSerialPort/QSerialPortInfo>

namespace {
const char *KEY_MAP_NAME = "Map/Name";
const char *KEY_LINKS_COUNT = "Links/Count";
const char *KEY_LINK_GROUP_PREFIX = "Link";

const char *DEFAULT_MAP_NAME = "OpenStreetMap";
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

void QTestGCSConfig::initializeDefaults()
{
    QGCSConfig::initializeDefaults();
    if (!m_settings)
        return;
    if (!m_settings->contains(KEY_MAP_NAME)) {
        m_settings->setValue(KEY_MAP_NAME, DEFAULT_MAP_NAME);
    }
    if (!m_settings->contains(KEY_LINKS_COUNT)) {
        m_settings->setValue(KEY_LINKS_COUNT, 0);
    }
    m_settings->sync();
}
