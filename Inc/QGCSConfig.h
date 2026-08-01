#ifndef QGCSCONFIG_H
#define QGCSCONFIG_H

#include <QObject>
#include <QString>
#include <cstdint>
#include "MiniGCSExport.h"

class QSettings;
struct QGCSConfigPrivateAccess;

/**
 * @brief QGCSConfig - 配置单例
 *
 * 管理应用程序 INI 配置：地面站身份、日志、类型文本目录等业务可选项。
 */
class MINIGCS_EXPORT QGCSConfig:public QObject
{
    Q_OBJECT
public:
    enum LogSeverity {
        LogSeverityEmergency = 0,
        LogSeverityAlert,
        LogSeverityCritical,
        LogSeverityError,
        LogSeverityWarning,
        LogSeverityNotice,
        LogSeverityInfo,
        LogSeverityDebug
    };
    Q_ENUM(LogSeverity)

    static QGCSConfig *instance();
    static void setInstance(QGCSConfig *p);

    static void qtLogHandler(QtMsgType type, const QMessageLogContext &ctx,
                             const QString &msg);

    void init();
    virtual void release();

    QString logLevel() const;

    /** 地面站身份 ID（配置文件设置） */
    uint8_t stationId() const;
    /** 地面站组件 ID（配置文件设置） */
    uint8_t stationComponentId() const;

    /** @deprecated 使用 stationId() */
    uint8_t gcsSystemId() const { return stationId(); }
    /** @deprecated 使用 stationComponentId() */
    uint8_t gcsComponentId() const { return stationComponentId(); }

    /** 类型/状态显示文本目录文件路径 */
    QString typeTextFile() const;

    /** 从类型文本目录读取文案 */
    QString typeText(const QString &section, const QString &key) const;

    bool timeSyncEnabled() const;

    double motionStartHorizontalSpeedMS() const;
    double motionStartVerticalSpeedMS() const;
    double motionStopHorizontalSpeedMS() const;
    double motionStopVerticalSpeedMS() const;
    int motionStartSampleCount() const;
    int motionStopSampleCount() const;

    void setTimeSyncEnabled(bool enabled);
    void save();
    Q_INVOKABLE void reload();
    QString configFilePath() const;

signals:
    /**
     * @brief warning 及以上级别的格式化业务日志
     * @param level 日志级别（实现侧数值，仅用于过滤展示）
     * @param message 已格式化的日志文本
     */
    void warningLogMessage(int level, const QString &message);

    /**
     * @brief 固件侧 warning 及以上级别日志
     */
    void firmwareWarningMessage(
        quint32 vehicleId, const QString &message);

protected:
    QGCSConfig(QObject* parent=nullptr);
    virtual ~QGCSConfig();
    virtual void initializeDefaults();

    QSettings* m_settings{};
    QString m_configFilePath;

private:
    friend struct QGCSConfigPrivateAccess;
    Q_DISABLE_COPY(QGCSConfig)

    void init_logging();

    static QGCSConfig* m_pSInsatance;
};

#endif // QGCSCONFIG_H
