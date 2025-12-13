#include "QGCSConfig.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

// 配置项键名常量
namespace {
    const char* KEY_SERIAL_PORT_NAME = "Serial/PortName";
    const char* KEY_SERIAL_BAUD_RATE = "Serial/BaudRate";
    const char* KEY_MAP_NAME = "Map/Name";
    const char* KEY_GCS_SYSTEM_ID = "GCS/SystemId";
    const char* KEY_GCS_COMPONENT_ID = "GCS/ComponentId";

    // 默认值
    const char* DEFAULT_PORT_NAME = "COM4";
    const int DEFAULT_BAUD_RATE = 57600;
    const char* DEFAULT_MAP_NAME = "OpenStreetMap";
    const uint8_t DEFAULT_GCS_SYSTEM_ID = 246;
    const uint8_t DEFAULT_GCS_COMPONENT_ID = 191;
}

QGCSConfig::QGCSConfig(QObject *parent)
    : QObject(parent)
    , m_settings(nullptr)
{
    // 确定配置文件路径
    QString appName = QCoreApplication::applicationName();
    if (appName.isEmpty()) {
        appName = "MiniGCS";
    }

    // 使用应用程序目录下的配置文件
    QString appDir = QCoreApplication::applicationDirPath();
    m_configFilePath = QDir(appDir).filePath(appName + ".ini");

    // 创建QSettings实例
    m_settings = new QSettings(m_configFilePath, QSettings::IniFormat, this);
    // Qt6 默认使用 UTF-8 编码，无需设置 setIniCodec

    // 初始化默认值
    initializeDefaults();

    qDebug() << "QGCSConfig: 配置文件路径:" << m_configFilePath;
}

QGCSConfig::~QGCSConfig()
{
    if (m_settings) {
        save();
    }
}

QGCSConfig& QGCSConfig::instance()
{
    static QGCSConfig instance;
    return instance;
}

QString QGCSConfig::defaultPortName() const
{
    return m_settings->value(KEY_SERIAL_PORT_NAME, DEFAULT_PORT_NAME).toString();
}

void QGCSConfig::setDefaultPortName(const QString &portName)
{
    m_settings->setValue(KEY_SERIAL_PORT_NAME, portName);
}

int QGCSConfig::defaultBaudRate() const
{
    return m_settings->value(KEY_SERIAL_BAUD_RATE, DEFAULT_BAUD_RATE).toInt();
}

void QGCSConfig::setDefaultBaudRate(int baudRate)
{
    m_settings->setValue(KEY_SERIAL_BAUD_RATE, baudRate);
}

QString QGCSConfig::mapName() const
{
    return m_settings->value(KEY_MAP_NAME, DEFAULT_MAP_NAME).toString();
}

void QGCSConfig::setMapName(const QString &mapName)
{
    m_settings->setValue(KEY_MAP_NAME, mapName);
}

uint8_t QGCSConfig::gcsSystemId() const
{
    return static_cast<uint8_t>(m_settings->value(KEY_GCS_SYSTEM_ID, static_cast<int>(DEFAULT_GCS_SYSTEM_ID)).toInt());
}

uint8_t QGCSConfig::gcsComponentId() const
{
    return static_cast<uint8_t>(m_settings->value(KEY_GCS_COMPONENT_ID, static_cast<int>(DEFAULT_GCS_COMPONENT_ID)).toInt());
}

void QGCSConfig::save()
{
    if (m_settings) {
        m_settings->sync();
        qDebug() << "QGCSConfig: 配置已保存到" << m_configFilePath;
    }
}

void QGCSConfig::reload()
{
    if (m_settings) {
        m_settings->sync();
        qDebug() << "QGCSConfig: 配置已重新加载";
    }
}

QString QGCSConfig::configFilePath() const
{
    return m_configFilePath;
}

void QGCSConfig::initializeDefaults()
{
    // 如果配置项不存在，则设置默认值
    if (!m_settings->contains(KEY_SERIAL_PORT_NAME)) {
        m_settings->setValue(KEY_SERIAL_PORT_NAME, DEFAULT_PORT_NAME);
    }
    if (!m_settings->contains(KEY_SERIAL_BAUD_RATE)) {
        m_settings->setValue(KEY_SERIAL_BAUD_RATE, DEFAULT_BAUD_RATE);
    }
    if (!m_settings->contains(KEY_MAP_NAME)) {
        m_settings->setValue(KEY_MAP_NAME, DEFAULT_MAP_NAME);
    }
    if (!m_settings->contains(KEY_GCS_SYSTEM_ID)) {
        m_settings->setValue(KEY_GCS_SYSTEM_ID, static_cast<int>(DEFAULT_GCS_SYSTEM_ID));
    }
    if (!m_settings->contains(KEY_GCS_COMPONENT_ID)) {
        m_settings->setValue(KEY_GCS_COMPONENT_ID, static_cast<int>(DEFAULT_GCS_COMPONENT_ID));
    }

    // 立即保存默认值
    m_settings->sync();
}

