#ifndef QGCSCONFIG_H
#define QGCSCONFIG_H

#include <QObject>
#include <QString>
#include "MiniGCSExport.h"

class QSettings;


/// <summary>
/// 前置声明 spdlog 命名空间和 sink 类
/// </summary>
namespace spdlog {
    namespace sinks {
        class sink;
    }
    using sink_ptr = std::shared_ptr<sinks::sink>;
};

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
   * @brief 获取MAV消息扩展文件名
   * @return MAV消息扩展文件名（如 "ardupilotmega.xml"）
   */
    QString mavMessageExtension() const;

    /**
   * @brief 获取是否开启时间同步
   * @return 是否开启时间同步（默认 true）
   */
    bool timeSyncEnabled() const;

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

    std::vector<spdlog::sink_ptr> sinks;
    static QGCSConfig* m_pSInsatance;
};

#endif // QGCSCONFIG_H
