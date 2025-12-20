#ifndef QGCSCONFIG_H
#define QGCSCONFIG_H

#include <QString>

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
 * 提供对串口、波特率、地图等配置项的读写访问
 * GCS系统ID和组件ID只能通过配置文件设置，不支持运行时修改
 */
class QGCSConfig
{
public:
    /**
     * @brief 获取配置单例实例
     * @return 配置单例实例的引用
     */
    static QGCSConfig* instance();

    /**
     * @brief 处理QtLog
     * @param type
     * @param ctx
     * @param msg
     */
    static void qtLogHandler(QtMsgType type,
        const QMessageLogContext& ctx,
        const QString& msg);

    void init();
    void release();

    /**
     * @brief 处理mavsdk消息
     * @param event
     */
    void dealMavsdkMessage(uint32_t systemID, const std::string& fields_json);

    /**
     * @brief 获取默认串口名称
     * @return 串口名称（如 "COM1", "/dev/ttyUSB0"）
     */
    QString defaultPortName() const;

    /**
     * @brief 设置默认串口名称
     * @param portName 串口名称
     */
    void setDefaultPortName(const QString& portName);

    /**
     * @brief 获取默认波特率
     * @return 波特率（如 57600, 115200）
     */
    int defaultBaudRate() const;

    /**
     * @brief 设置默认波特率
     * @param baudRate 波特率
     */
    void setDefaultBaudRate(int baudRate);

    /**
     * @brief 获取地图名称
     * @return 地图名称
     */
    QString mapName() const;

    /**
     * @brief 设置地图名称
     * @param mapName 地图名称
     */
    void setMapName(const QString& mapName);

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
    void reload();

    /**
     * @brief 获取配置文件路径
     * @return 配置文件路径
     */
    QString configFilePath() const;

private:
    QGCSConfig();
    ~QGCSConfig();
    Q_DISABLE_COPY(QGCSConfig)

    /**
     * @brief 初始化默认值
     */
    void initializeDefaults();

    /**
     * @brief 初始化日志系统
     */
    void init_logging();

    QSettings* m_settings;        ///< QSettings实例，用于读写INI文件
    QString m_configFilePath;     ///< 配置文件路径

    std::vector<spdlog::sink_ptr> sinks;
    static QGCSConfig* m_pSInsatance;
};

#endif // QGCSCONFIG_H
