#ifndef QGCSCONFIG_H
#define QGCSCONFIG_H

#include <QObject>
#include <QString>
#include <cstdint>
#include <string>
#include "MiniGCSExport.h"

class QSettings;

/**
 * @brief QGCSConfig类 - 配置文件单例类
 *
 * 该类使用单例模式管理应用程序的配置文件（INI格式）
 * 提供 GCS 系统ID、组件ID、日志等通用配置项的读写访问
 */
class MINIGCS_EXPORT QGCSConfig:public QObject
{
    Q_OBJECT
public:
    /**
   * @brief 获取配置单例实例
   * @return 配置单例实例的引用
   */
    static QGCSConfig *instance();

    /**
   * @brief 设置配置单例实例（用于派生类注入，须在首次 instance() 前调用）
   * @param p 配置实例（如 QTestGCSConfig）
   */
    static void setInstance(QGCSConfig *p);

    /**
   * @brief 处理QtLog
   * @param type
   * @param ctx
   * @param msg
   */
    static void qtLogHandler(QtMsgType type, const QMessageLogContext &ctx,
                             const QString &msg);

    void init();
    virtual void release();

    /**
   * @brief 处理mavsdk消息
   * @param event
   */
    void dealMavsdkMessage(uint32_t systemID, const std::string &fields_json);

    /**
     * @brief 处理已解析的 MAVLink STATUSTEXT，应在 Qt 对象线程调用
     */
    void dealMavsdkStatusText(uint32_t systemID, int severity,
                              const QString &text);

    /**
   * @brief 获取日志等级字符串（例如 "debug","info","warn","error"）
   */
    QString logLevel() const;

    /**
   * @brief 获取GCS系统ID
   * @return 系统ID（只能通过配置文件设置）
   */
    uint8_t gcsSystemId() const;

    /**
   * @brief 获取GCS组件ID
   * @return 组件ID（只能通过配置文件设置）
   */
    uint8_t gcsComponentId() const;

    /**
   * @brief 获取 MAV 消息扩展 XML 路径（支持相对/绝对路径，解析规则同文本目录）
   * @return 扩展 XML 文件路径（如 Config 下的 ardupilotmega.xml）
   */
    QString mavMessageExtension() const;

    /**
   * @brief 获取 MAVSDK 类型文本映射文件路径
   */
    QString mavsdkTypeTextFile() const;

    /**
     * @brief 从 MAVSDK 文本目录读取字符串键对应的文本
     */
    QString mavsdkText(const QString &section, const QString &key) const;

    /**
   * @brief 获取是否开启时间同步
   * @return 是否开启时间同步（默认 true）
   */
    bool timeSyncEnabled() const;

    double motionStartHorizontalSpeedMS() const;
    double motionStartVerticalSpeedMS() const;
    double motionStopHorizontalSpeedMS() const;
    double motionStopVerticalSpeedMS() const;
    int motionStartSampleCount() const;
    int motionStopSampleCount() const;

    /**
   * @brief 设置是否开启时间同步
   * @param enabled 是否开启
   */
    void setTimeSyncEnabled(bool enabled);

    /**
   * @brief 保存配置到文件
   */
    void save();

    /**
   * @brief 重新加载配置
   */
    Q_INVOKABLE void reload();

    /**
   * @brief 获取配置文件路径
   * @return 配置文件路径
   */
    QString configFilePath() const;

signals:
    /**
     * @brief warning 及以上级别的格式化日志
     * @param level spdlog 级别数值
     * @param message 已格式化的日志文本
     */
    void warningLogMessage(int level, const QString &message);

    /**
     * @brief MAVLink STATUSTEXT 中 warning 及以上级别的固件日志
     */
    void firmwareWarningMessage(
        quint32 systemId, int severity, const QString &message);

protected:
    QGCSConfig(QObject* parent=nullptr);
    virtual ~QGCSConfig();

    /**
     * @brief 初始化默认值
     */
    virtual void initializeDefaults();

    QSettings* m_settings{};        ///< QSettings实例，用于读写INI文件
    QString m_configFilePath;     ///< 配置文件路径
private:
    Q_DISABLE_COPY(QGCSConfig)

    /**
     * @brief 初始化日志系统
     */
    void init_logging();

    static QGCSConfig* m_pSInsatance;
};

#endif // QGCSCONFIG_H
