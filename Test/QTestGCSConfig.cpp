#include <QSettings>
#include "QTestGCSConfig.h"
#include <QtSerialPort/QSerialPortInfo>

namespace {
const char *KEY_SERIAL_PORT_NAME = "Serial/PortName";
const char *KEY_SERIAL_BAUD_RATE = "Serial/BaudRate";
const char *KEY_MAP_NAME = "Map/Name";

const char *DEFAULT_PORT_NAME = "COM14";
const int DEFAULT_BAUD_RATE = 57600;
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

QString QTestGCSConfig::defaultPortName() const
{
    if (!m_settings)
        return QString(DEFAULT_PORT_NAME);
    return m_settings->value(KEY_SERIAL_PORT_NAME, DEFAULT_PORT_NAME).toString();
}

void QTestGCSConfig::setDefaultPortName(const QString &portName)
{
    if (m_settings)
        m_settings->setValue(KEY_SERIAL_PORT_NAME, portName);
}

int QTestGCSConfig::defaultBaudRate() const
{
    if (!m_settings)
        return DEFAULT_BAUD_RATE;
    return m_settings->value(KEY_SERIAL_BAUD_RATE, DEFAULT_BAUD_RATE).toInt();
}

void QTestGCSConfig::setDefaultBaudRate(int baudRate)
{
    if (m_settings)
        m_settings->setValue(KEY_SERIAL_BAUD_RATE, baudRate);
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

void QTestGCSConfig::initializeDefaults()
{
    QGCSConfig::initializeDefaults();
    if (!m_settings)
        return;
    if (!m_settings->contains(KEY_SERIAL_PORT_NAME)) {
        m_settings->setValue(KEY_SERIAL_PORT_NAME, DEFAULT_PORT_NAME);
    }
    if (!m_settings->contains(KEY_SERIAL_BAUD_RATE)) {
        m_settings->setValue(KEY_SERIAL_BAUD_RATE, DEFAULT_BAUD_RATE);
    }
    if (!m_settings->contains(KEY_MAP_NAME)) {
        m_settings->setValue(KEY_MAP_NAME, DEFAULT_MAP_NAME);
    }
    m_settings->sync();
}
