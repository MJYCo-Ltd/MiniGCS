#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include "QGCSConfig.h"

#include <spdlog/sinks/daily_file_sink.h>
#include "QGCSLog.h"

QGCSConfig *QGCSConfig::m_pSInsatance = nullptr;
// 配置项键名常量
namespace {
const char *KEY_SERIAL_PORT_NAME = "Serial/PortName";
const char *KEY_SERIAL_BAUD_RATE = "Serial/BaudRate";
const char *KEY_MAP_NAME = "Map/Name";
const char *KEY_GCS_SYSTEM_ID = "GCS/SystemId";
const char *KEY_GCS_COMPONENT_ID = "GCS/ComponentId";
const char *KEY_LOG_LEVEL = "Logging/Level";

// 默认值
const char *DEFAULT_PORT_NAME = "COM14";
const int DEFAULT_BAUD_RATE = 57600;
const char *DEFAULT_MAP_NAME = "OpenStreetMap";
const uint8_t DEFAULT_GCS_SYSTEM_ID = 246;
const uint8_t DEFAULT_GCS_COMPONENT_ID = 191;
const char *DEFAULT_LOG_LEVEL = "debug";
} // namespace

// 将 QString（名称）映射到 spdlog 的 level_enum
static spdlog::level::level_enum levelFromString(const QString &levelStr) {
    QString l = levelStr.trimmed().toLower();
    if (l == "trace")
        return spdlog::level::trace;
    if (l == "debug")
        return spdlog::level::debug;
    if (l == "info")
        return spdlog::level::info;
    if (l == "warn" || l == "warning")
        return spdlog::level::warn;
    if (l == "err" || l == "error")
        return spdlog::level::err;
    if (l == "critical" || l == "crit")
        return spdlog::level::critical;
    if (l == "off")
        return spdlog::level::off;
    // fallback
    return spdlog::level::debug;
}

QGCSConfig::QGCSConfig() : m_settings(nullptr) {}

QGCSConfig::~QGCSConfig() {
    spdlog::warn(SYS_FMT_STR,"系统正在清理资源","即将退出……");
    sinks.clear();
    if (m_settings) {
        save();
        delete m_settings;
        m_settings = nullptr;
    }
}

QGCSConfig *QGCSConfig::instance() {
    if (m_pSInsatance == nullptr) {
        m_pSInsatance = new QGCSConfig;
    }

    return (m_pSInsatance);
}

void QGCSConfig::init_logging() {
    // 先建立 sinks
    // auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
        "log/minigcs.log", 0,0,false,7);

    // 从配置读取日志级别
    QString configuredLevel = DEFAULT_LOG_LEVEL;
    if (m_settings) {
        configuredLevel =
            m_settings->value(KEY_LOG_LEVEL, DEFAULT_LOG_LEVEL).toString();
    }
    spdlog::level::level_enum lvl = levelFromString(configuredLevel);

    // 将级别应用到 sink（并保留原来的按用途设置可选，这里统一使用配置级别）
    file_sink->set_level(lvl);

    sinks.clear();
    sinks.push_back(file_sink);

    auto logger =
        std::make_shared<spdlog::logger>("core", sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
    spdlog::set_level(lvl); // 全局级别

    spdlog::warn(SYS_FMT_STR,
                 "系统启动 日志级别",configuredLevel.toStdString());
}

void QGCSConfig::qtLogHandler(QtMsgType type, const QMessageLogContext &ctx,
                              const QString &msg) {
    std::string logMsg(msg.toUtf8().constData());
    // 拼接上下文信息
    std::string ctxInfo = fmt::format(
        "[{}:{} {}] {}", ctx.file ? strrchr(ctx.file,'\\')+1 : "", ctx.line,
        ctx.function ? ctx.function : "", logMsg);

    switch (type) {
    case QtDebugMsg:
        spdlog::debug(ctxInfo);
        break;
    case QtInfoMsg:
        spdlog::info(ctxInfo);
        break;
    case QtWarningMsg:
        spdlog::warn(ctxInfo);
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        spdlog::error(ctxInfo);
        break;
    }
}

void QGCSConfig::init() {
    // 确定配置文件路径
    QString appName = QCoreApplication::applicationName();
    if (appName.isEmpty()) {
        appName = "MiniGCS";
    }

    // 使用应用程序目录下的配置文件
    QString appDir = QCoreApplication::applicationDirPath();
    m_configFilePath = QDir(appDir).filePath(appName + ".ini");
    init_logging();

    // 创建QSettings实例
    m_settings = new QSettings(m_configFilePath, QSettings::IniFormat);
    // Qt6 默认使用 UTF-8 编码，无需设置 setIniCodec

    // 初始化默认值
    initializeDefaults();
}

void QGCSConfig::release() {
    delete m_pSInsatance;
    m_pSInsatance = nullptr;
}

void QGCSConfig::dealMavsdkMessage(uint32_t systemID,
                                   const std::string &jsonMessage) {
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonMessage.c_str(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return;
    const QJsonObject obj = doc.object();
    if (obj.value("message_name").toString() == "STATUSTEXT") {
        switch (obj.value("severity").toInt()) {
        case 0:
            spdlog::critical(PLAT_FMT_STR, systemID, "text",
                             obj.value("text").toString().toUtf8().data());
            break;
        case 1:
            spdlog::critical(PLAT_FMT_STR, systemID, "text",
                             obj.value("text").toString().toUtf8().data());
            break;
        case 2:
            spdlog::critical(PLAT_FMT_STR, systemID, "text",
                             obj.value("text").toString().toUtf8().data());
            break;
        case 3:
            spdlog::error(PLAT_FMT_STR, systemID, "text",
                          obj.value("text").toString().toUtf8().data());
            break;
        case 4:
            spdlog::warn(PLAT_FMT_STR, systemID, "text",
                         obj.value("text").toString().toUtf8().data());
            break;
        case 5:
            spdlog::info(PLAT_FMT_STR, systemID, "text",
                         obj.value("text").toString().toUtf8().data());
            break;
        case 6:
            spdlog::info(PLAT_FMT_STR, systemID, "text",
                         obj.value("text").toString().toUtf8().data());
            break;
        case 7:
            spdlog::debug(PLAT_FMT_STR, systemID, "text",
                          obj.value("text").toString().toUtf8().data());
            break;
        }
    }
}

QString QGCSConfig::defaultPortName() const {
    return m_settings->value(KEY_SERIAL_PORT_NAME, DEFAULT_PORT_NAME).toString();
}

void QGCSConfig::setDefaultPortName(const QString &portName) {
    m_settings->setValue(KEY_SERIAL_PORT_NAME, portName);
}

int QGCSConfig::defaultBaudRate() const {
    return m_settings->value(KEY_SERIAL_BAUD_RATE, DEFAULT_BAUD_RATE).toInt();
}

void QGCSConfig::setDefaultBaudRate(int baudRate) {
    m_settings->setValue(KEY_SERIAL_BAUD_RATE, baudRate);
}

QString QGCSConfig::mapName() const {
    return m_settings->value(KEY_MAP_NAME, DEFAULT_MAP_NAME).toString();
}

void QGCSConfig::setMapName(const QString &mapName) {
    m_settings->setValue(KEY_MAP_NAME, mapName);
}

QString QGCSConfig::logLevel() const {
    if (!m_settings)
        return QString(DEFAULT_LOG_LEVEL);
    return m_settings->value(KEY_LOG_LEVEL, DEFAULT_LOG_LEVEL).toString();
}

void QGCSConfig::setLogLevel(const QString &level) {
    if (!m_settings)
        return;
    QString l = level.trimmed();
    if (l.isEmpty())
        return;
    m_settings->setValue(KEY_LOG_LEVEL, l);
    m_settings->sync();

    // 立即应用到 spdlog 全局级别
    spdlog::level::level_enum lvl = levelFromString(l);
    spdlog::set_level(lvl);
    for (auto &s : sinks) {
        if (s)
            s->set_level(lvl);
    }
    spdlog::info("Log level changed to {}", l.toStdString());
}

uint8_t QGCSConfig::gcsSystemId() const {
    return static_cast<uint8_t>(
        m_settings
            ->value(KEY_GCS_SYSTEM_ID, static_cast<int>(DEFAULT_GCS_SYSTEM_ID))
            .toInt());
}

uint8_t QGCSConfig::gcsComponentId() const {
    return static_cast<uint8_t>(
        m_settings
            ->value(KEY_GCS_COMPONENT_ID,
                    static_cast<int>(DEFAULT_GCS_COMPONENT_ID))
            .toInt());
}

void QGCSConfig::save() {
    if (m_settings) {
        m_settings->sync();
    }
}

void QGCSConfig::reload() {
    if (m_settings) {
        m_settings->sync();
    }
}

QString QGCSConfig::configFilePath() const { return m_configFilePath; }

void QGCSConfig::initializeDefaults() {
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
        m_settings->setValue(KEY_GCS_SYSTEM_ID,
                             static_cast<int>(DEFAULT_GCS_SYSTEM_ID));
    }
    if (!m_settings->contains(KEY_GCS_COMPONENT_ID)) {
        m_settings->setValue(KEY_GCS_COMPONENT_ID,
                             static_cast<int>(DEFAULT_GCS_COMPONENT_ID));
    }
    if (!m_settings->contains(KEY_LOG_LEVEL)) {
        m_settings->setValue(KEY_LOG_LEVEL, DEFAULT_LOG_LEVEL);
    }

    // 立即保存默认值
    m_settings->sync();
}
