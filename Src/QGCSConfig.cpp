#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QSettings>
#include <QFileInfo>
#include <QPointer>
#include <cstring>
#include <mutex>
#include <vector>
#include "QGCSConfig.h"
#include "Private/QMavsdkTextCatalog.h"

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/base_sink.h>
#include "Private/QGCSLog.h"

QGCSConfig *QGCSConfig::m_pSInsatance = nullptr;
// 配置项键名常量
namespace {
const char *KEY_GCS_SYSTEM_ID = "GCS/SystemId";
const char *KEY_GCS_COMPONENT_ID = "GCS/ComponentId";
const char *KEY_LOG_LEVEL = "Logging/Level";
const char *KEY_MAV_MESSAGE_EXTENSION = "MavMessage/Extension";
const char *KEY_MAVSDK_TYPE_TEXT_FILE = "Mavsdk/TypeTextFile";
const char *KEY_MAV_COMMAND_ACK_TIMEOUT_MS = "Mavsdk/CommandAckTimeoutMs";
const char *KEY_TIME_SYNC_ENABLED = "TimeSync/Enabled";
const char *KEY_MOTION_START_HORIZONTAL = "Motion/StartHorizontalSpeedMS";
const char *KEY_MOTION_START_VERTICAL = "Motion/StartVerticalSpeedMS";
const char *KEY_MOTION_STOP_HORIZONTAL = "Motion/StopHorizontalSpeedMS";
const char *KEY_MOTION_STOP_VERTICAL = "Motion/StopVerticalSpeedMS";
const char *KEY_MOTION_START_SAMPLES = "Motion/StartSampleCount";
const char *KEY_MOTION_STOP_SAMPLES = "Motion/StopSampleCount";

// 默认值
const uint8_t DEFAULT_GCS_SYSTEM_ID = 246;
const uint8_t DEFAULT_GCS_COMPONENT_ID = 191;
const char *DEFAULT_LOG_LEVEL = "debug";
const char *DEFAULT_MAV_MESSAGE_EXTENSION = "ardupilotmega.xml";
const char *DEFAULT_MAVSDK_TYPE_TEXT_FILE = "mavsdk_zh_CN.json";
constexpr int DEFAULT_MAV_COMMAND_ACK_TIMEOUT_MS = 5000;
const bool DEFAULT_TIME_SYNC_ENABLED = true;
constexpr double DEFAULT_MOTION_START_HORIZONTAL = 0.7;
constexpr double DEFAULT_MOTION_START_VERTICAL = 0.5;
constexpr double DEFAULT_MOTION_STOP_HORIZONTAL = 0.25;
constexpr double DEFAULT_MOTION_STOP_VERTICAL = 0.2;
constexpr int DEFAULT_MOTION_START_SAMPLES = 2;
constexpr int DEFAULT_MOTION_STOP_SAMPLES = 5;
enum MavSeverity {
    MavSeverityEmergency = 0,
    MavSeverityAlert,
    MavSeverityCritical,
    MavSeverityError,
    MavSeverityWarning,
    MavSeverityNotice,
    MavSeverityInfo,
    MavSeverityDebug
};
thread_local bool g_emittingFirmwareLog = false;
QtMessageHandler g_previousQtMessageHandler = nullptr;
bool g_qtLogHandlerInstalled = false;

class FirmwareLogScope final
{
public:
    FirmwareLogScope()
        : m_previous(g_emittingFirmwareLog)
    {
        g_emittingFirmwareLog = true;
    }

    ~FirmwareLogScope()
    {
        g_emittingFirmwareLog = m_previous;
    }

private:
    bool m_previous;
};

class QtWarningSink final : public spdlog::sinks::base_sink<std::mutex>
{
public:
    explicit QtWarningSink(QGCSConfig *config)
        : m_config(config)
    {}

protected:
    void sink_it_(const spdlog::details::log_msg &message) override
    {
        if (message.level < spdlog::level::warn || !m_config ||
            g_emittingFirmwareLog) {
            return;
        }

        spdlog::memory_buf_t buffer;
        formatter_->format(message, buffer);
        const QString text =
            QString::fromUtf8(buffer.data(), static_cast<qsizetype>(buffer.size()))
                .trimmed();
        const int level = static_cast<int>(message.level);
        const QPointer<QGCSConfig> config = m_config;
        QMetaObject::invokeMethod(
            m_config,
            [config, level, text]() {
                if (config) {
                    emit config->warningLogMessage(level, text);
                }
            },
            Qt::QueuedConnection);
    }

    void flush_() override {}

private:
    QPointer<QGCSConfig> m_config;
};
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

QGCSConfig::QGCSConfig(QObject *parent) : QObject(parent) {}

QGCSConfig::~QGCSConfig() {
    spdlog::warn(SYS_FMT_STR,"系统正在清理资源","即将退出……");
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

void QGCSConfig::setInstance(QGCSConfig *p) {
    if (m_pSInsatance != nullptr && m_pSInsatance != p) {
        delete m_pSInsatance;
    }
    m_pSInsatance = p;
}

void QGCSConfig::init_logging() {
    const QString logDirectory = QStringLiteral("data/log");
    if (!QDir().mkpath(logDirectory)) {
        qWarning() << "无法创建日志目录:" << logDirectory;
        return;
    }

    try {
        auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            "data/log/minigcs.log", 0, 0, false, 7);
        auto warning_sink = std::make_shared<QtWarningSink>(this);

        QString configuredLevel = DEFAULT_LOG_LEVEL;
        if (m_settings) {
            configuredLevel =
                m_settings->value(KEY_LOG_LEVEL, DEFAULT_LOG_LEVEL).toString();
        }
        spdlog::level::level_enum lvl = levelFromString(configuredLevel);

        file_sink->set_level(lvl);
        warning_sink->set_level(spdlog::level::warn);

        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(file_sink);
        sinks.push_back(warning_sink);

        auto logger =
            std::make_shared<spdlog::logger>("core", sinks.begin(), sinks.end());
        spdlog::set_default_logger(logger);

        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
        logger->set_level(
            lvl < spdlog::level::warn ? lvl : spdlog::level::warn);

        spdlog::warn(SYS_FMT_STR, "系统启动 日志级别",
                     configuredLevel.toStdString());
    } catch (const spdlog::spdlog_ex &error) {
        qWarning() << "日志系统初始化失败:" << error.what();
    }
}

void QGCSConfig::qtLogHandler(QtMsgType type, const QMessageLogContext &ctx,
                              const QString &msg) {
    std::string logMsg(msg.toUtf8().constData());
    const char *pFileName = nullptr;
    if (nullptr != ctx.file) {
        pFileName = strrchr(ctx.file, '/');
        if (nullptr == pFileName) {
            pFileName = strrchr(ctx.file, '\\');
        }
    }

    // 拼接上下文信息
    std::string ctxInfo = fmt::format(
        "[{}:{} {}] {}", nullptr != pFileName ? pFileName+1 : "", ctx.line,
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
    if (m_settings) {
        return;
    }

    // 确定配置文件路径
    QString appName = QCoreApplication::applicationName();
    if (appName.isEmpty()) {
        appName = "MiniGCS";
    }
    // 使用应用程序目录下的配置文件
    QString appDir = QString("%1%2Config").arg(QCoreApplication::applicationDirPath()).arg(QDir::separator());
    if (!QDir().mkpath(appDir)) {
        qWarning() << "无法创建配置目录:" << appDir;
        return;
    }
    m_configFilePath = QDir(appDir).filePath(appName + ".ini");

    // 创建QSettings实例
    m_settings = new QSettings(m_configFilePath, QSettings::IniFormat);
    // Qt6 默认使用 UTF-8 编码，无需设置 setIniCodec

    // 初始化默认值
    initializeDefaults();
    if (m_settings->status() != QSettings::NoError) {
        qWarning() << "配置文件读写异常:" << m_configFilePath
                   << "status=" << m_settings->status();
    }

    // 默认值与用户配置加载完成后再初始化日志。
    init_logging();
    if (!g_qtLogHandlerInstalled) {
        g_previousQtMessageHandler =
            qInstallMessageHandler(&QGCSConfig::qtLogHandler);
        g_qtLogHandlerInstalled = true;
    }
}

void QGCSConfig::release() {
    if (g_qtLogHandlerInstalled) {
        qInstallMessageHandler(g_previousQtMessageHandler);
        g_previousQtMessageHandler = nullptr;
        g_qtLogHandlerInstalled = false;
    }
    delete m_pSInsatance;
    m_pSInsatance = nullptr;
}

void QGCSConfig::dealMavsdkStatusText(uint32_t systemID, int severity,
                                      const QString &text)
{
    if (severity >= MavSeverityEmergency &&
        severity <= MavSeverityWarning) {
        const QString formatted =
            QStringLiteral("[%1] [MAVLink] [system_id=%2] %3")
                .arg(QDateTime::currentDateTime().toString(
                         QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz")))
                .arg(systemID)
                .arg(text);
        emit firmwareWarningMessage(
            static_cast<quint32>(systemID), severity, formatted);
    }

    const FirmwareLogScope firmwareLogScope;
    switch (severity) {
    case MavSeverityEmergency:
    case MavSeverityAlert:
    case MavSeverityCritical:
        spdlog::critical(PLAT_FMT_STR, systemID, "text",
                         text.toUtf8().data());
        break;
    case MavSeverityError:
        spdlog::error(PLAT_FMT_STR, systemID, "text", text.toUtf8().data());
        break;
    case MavSeverityWarning:
        spdlog::warn(PLAT_FMT_STR, systemID, "text", text.toUtf8().data());
        break;
    case MavSeverityNotice:
    case MavSeverityInfo:
        spdlog::info(PLAT_FMT_STR, systemID, "text", text.toUtf8().data());
        break;
    case MavSeverityDebug:
        spdlog::debug(PLAT_FMT_STR, systemID, "text", text.toUtf8().data());
        break;
    }
}

QString QGCSConfig::logLevel() const {
    if (!m_settings)
        return QString(DEFAULT_LOG_LEVEL);
    return m_settings->value(KEY_LOG_LEVEL, DEFAULT_LOG_LEVEL).toString();
}

uint8_t QGCSConfig::gcsSystemId() const {
    if (!m_settings) {
        return DEFAULT_GCS_SYSTEM_ID;
    }
    return static_cast<uint8_t>(
        m_settings
            ->value(KEY_GCS_SYSTEM_ID, static_cast<int>(DEFAULT_GCS_SYSTEM_ID))
            .toInt());
}

uint8_t QGCSConfig::gcsComponentId() const {
    if (!m_settings) {
        return DEFAULT_GCS_COMPONENT_ID;
    }
    return static_cast<uint8_t>(
        m_settings
            ->value(KEY_GCS_COMPONENT_ID,
                    static_cast<int>(DEFAULT_GCS_COMPONENT_ID))
            .toInt());
}

QString QGCSConfig::mavMessageExtension() const {
    QString configured = QString::fromLatin1(DEFAULT_MAV_MESSAGE_EXTENSION);
    if (m_settings) {
        configured =
            m_settings->value(KEY_MAV_MESSAGE_EXTENSION,
                              DEFAULT_MAV_MESSAGE_EXTENSION).toString().trimmed();
    }
    if (configured.isEmpty()) {
        configured = QString::fromLatin1(DEFAULT_MAV_MESSAGE_EXTENSION);
    }

    const QFileInfo configuredFile(configured);
    if (configuredFile.isAbsolute()) {
        return configuredFile.absoluteFilePath();
    }

    QString configDirectory;
    if (!m_configFilePath.isEmpty()) {
        configDirectory = QFileInfo(m_configFilePath).absolutePath();
    } else {
        configDirectory =
            QDir(QCoreApplication::applicationDirPath()).filePath("Config");
    }
    const QString primaryPath = QDir(configDirectory).filePath(configured);
    if (QFileInfo::exists(primaryPath) ||
        configured != QString::fromLatin1(DEFAULT_MAV_MESSAGE_EXTENSION)) {
        return primaryPath;
    }

    const QString buildTreePath =
        QDir(QCoreApplication::applicationDirPath())
            .filePath(QStringLiteral("../Config/%1").arg(configured));
    if (QFileInfo::exists(buildTreePath)) {
        return QFileInfo(buildTreePath).absoluteFilePath();
    }

    const QString workingDirectoryPath =
        QDir::current().filePath(QStringLiteral("Config/%1").arg(configured));
    if (QFileInfo::exists(workingDirectoryPath)) {
        return QFileInfo(workingDirectoryPath).absoluteFilePath();
    }
    return primaryPath;
}

QString QGCSConfig::mavsdkTypeTextFile() const {
    QString configured = QString::fromLatin1(DEFAULT_MAVSDK_TYPE_TEXT_FILE);
    if (m_settings) {
        configured =
            m_settings->value(KEY_MAVSDK_TYPE_TEXT_FILE,
                              DEFAULT_MAVSDK_TYPE_TEXT_FILE).toString().trimmed();
    }
    if (configured.isEmpty()) {
        configured = QString::fromLatin1(DEFAULT_MAVSDK_TYPE_TEXT_FILE);
    }

    const QFileInfo configuredFile(configured);
    if (configuredFile.isAbsolute()) {
        return configuredFile.absoluteFilePath();
    }

    QString configDirectory;
    if (!m_configFilePath.isEmpty()) {
        configDirectory = QFileInfo(m_configFilePath).absolutePath();
    } else {
        configDirectory =
            QDir(QCoreApplication::applicationDirPath()).filePath("Config");
    }
    const QString primaryPath = QDir(configDirectory).filePath(configured);
    if (QFileInfo::exists(primaryPath) ||
        configured != QString::fromLatin1(DEFAULT_MAVSDK_TYPE_TEXT_FILE)) {
        return primaryPath;
    }

    const QString buildTreePath =
        QDir(QCoreApplication::applicationDirPath())
            .filePath(QStringLiteral("../Config/%1").arg(configured));
    if (QFileInfo::exists(buildTreePath)) {
        return QFileInfo(buildTreePath).absoluteFilePath();
    }

    const QString workingDirectoryPath =
        QDir::current().filePath(QStringLiteral("Config/%1").arg(configured));
    if (QFileInfo::exists(workingDirectoryPath)) {
        return QFileInfo(workingDirectoryPath).absoluteFilePath();
    }
    return primaryPath;
}

QString QGCSConfig::mavsdkText(
    const QString &section, const QString &key) const
{
    return QMavsdkTextCatalog::text(section, key);
}

int QGCSConfig::mavCommandAckTimeoutMs() const
{
    const int configured = m_settings
        ? m_settings->value(KEY_MAV_COMMAND_ACK_TIMEOUT_MS,
                            DEFAULT_MAV_COMMAND_ACK_TIMEOUT_MS).toInt()
        : DEFAULT_MAV_COMMAND_ACK_TIMEOUT_MS;
    return qBound(1000, configured, 60000);
}

bool QGCSConfig::timeSyncEnabled() const {
    if (!m_settings) {
        return DEFAULT_TIME_SYNC_ENABLED;
    }
    return m_settings->value(KEY_TIME_SYNC_ENABLED, DEFAULT_TIME_SYNC_ENABLED).toBool();
}

double QGCSConfig::motionStartHorizontalSpeedMS() const
{
    return m_settings
        ? m_settings->value(KEY_MOTION_START_HORIZONTAL,
                            DEFAULT_MOTION_START_HORIZONTAL).toDouble()
        : DEFAULT_MOTION_START_HORIZONTAL;
}

double QGCSConfig::motionStartVerticalSpeedMS() const
{
    return m_settings
        ? m_settings->value(KEY_MOTION_START_VERTICAL,
                            DEFAULT_MOTION_START_VERTICAL).toDouble()
        : DEFAULT_MOTION_START_VERTICAL;
}

double QGCSConfig::motionStopHorizontalSpeedMS() const
{
    return m_settings
        ? m_settings->value(KEY_MOTION_STOP_HORIZONTAL,
                            DEFAULT_MOTION_STOP_HORIZONTAL).toDouble()
        : DEFAULT_MOTION_STOP_HORIZONTAL;
}

double QGCSConfig::motionStopVerticalSpeedMS() const
{
    return m_settings
        ? m_settings->value(KEY_MOTION_STOP_VERTICAL,
                            DEFAULT_MOTION_STOP_VERTICAL).toDouble()
        : DEFAULT_MOTION_STOP_VERTICAL;
}

int QGCSConfig::motionStartSampleCount() const
{
    return m_settings
        ? m_settings->value(KEY_MOTION_START_SAMPLES,
                            DEFAULT_MOTION_START_SAMPLES).toInt()
        : DEFAULT_MOTION_START_SAMPLES;
}

int QGCSConfig::motionStopSampleCount() const
{
    return m_settings
        ? m_settings->value(KEY_MOTION_STOP_SAMPLES,
                            DEFAULT_MOTION_STOP_SAMPLES).toInt()
        : DEFAULT_MOTION_STOP_SAMPLES;
}

void QGCSConfig::setTimeSyncEnabled(bool enabled) {
    if (m_settings) {
        m_settings->setValue(KEY_TIME_SYNC_ENABLED, enabled);
    }
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
    if (!m_settings) {
        return;
    }

    // 如果配置项不存在，则设置默认值
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
    if (!m_settings->contains(KEY_MAV_MESSAGE_EXTENSION)) {
        m_settings->setValue(KEY_MAV_MESSAGE_EXTENSION, DEFAULT_MAV_MESSAGE_EXTENSION);
    }
    if (!m_settings->contains(KEY_MAVSDK_TYPE_TEXT_FILE)) {
        m_settings->setValue(KEY_MAVSDK_TYPE_TEXT_FILE,
                             DEFAULT_MAVSDK_TYPE_TEXT_FILE);
    }
    if (!m_settings->contains(KEY_MAV_COMMAND_ACK_TIMEOUT_MS)) {
        m_settings->setValue(KEY_MAV_COMMAND_ACK_TIMEOUT_MS,
                             DEFAULT_MAV_COMMAND_ACK_TIMEOUT_MS);
    }
    if (!m_settings->contains(KEY_TIME_SYNC_ENABLED)) {
        m_settings->setValue(KEY_TIME_SYNC_ENABLED, DEFAULT_TIME_SYNC_ENABLED);
    }
    if (!m_settings->contains(KEY_MOTION_START_HORIZONTAL)) {
        m_settings->setValue(KEY_MOTION_START_HORIZONTAL,
                             DEFAULT_MOTION_START_HORIZONTAL);
    }
    if (!m_settings->contains(KEY_MOTION_START_VERTICAL)) {
        m_settings->setValue(KEY_MOTION_START_VERTICAL,
                             DEFAULT_MOTION_START_VERTICAL);
    }
    if (!m_settings->contains(KEY_MOTION_STOP_HORIZONTAL)) {
        m_settings->setValue(KEY_MOTION_STOP_HORIZONTAL,
                             DEFAULT_MOTION_STOP_HORIZONTAL);
    }
    if (!m_settings->contains(KEY_MOTION_STOP_VERTICAL)) {
        m_settings->setValue(KEY_MOTION_STOP_VERTICAL,
                             DEFAULT_MOTION_STOP_VERTICAL);
    }
    if (!m_settings->contains(KEY_MOTION_START_SAMPLES)) {
        m_settings->setValue(KEY_MOTION_START_SAMPLES,
                             DEFAULT_MOTION_START_SAMPLES);
    }
    if (!m_settings->contains(KEY_MOTION_STOP_SAMPLES)) {
        m_settings->setValue(KEY_MOTION_STOP_SAMPLES,
                             DEFAULT_MOTION_STOP_SAMPLES);
    }

    // 立即保存默认值
    m_settings->sync();
}
