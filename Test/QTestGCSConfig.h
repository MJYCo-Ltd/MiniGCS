#ifndef QTESTGCSCONFIG_H
#define QTESTGCSCONFIG_H

#include "QGCSConfig.h"
#include <QVariantMap>
#include <QVariantList>

/**
 * @brief 链路类型枚举（与配置中 Type 字符串对应）
 */
namespace LinkType {
const char Serial[]     = "Serial";
const char TcpServer[]  = "TcpServer";
const char TcpClient[]  = "TcpClient";
const char UdpServer[]  = "UdpServer";
const char UdpClient[]  = "UdpClient";
}

/**
 * @brief 单条链路配置的 QVariantMap 键名
 * type: Serial|TcpServer|TcpClient|UdpServer|UdpClient
 * name: 可选显示名称
 * portName, baudRate: Serial 专用
 * hostName, port: TcpClient/UdpClient 专用
 * port: TcpServer/UdpServer 专用（仅端口）
 */
namespace LinkConfigKeys {
const char Type[]      = "type";
const char Name[]      = "name";
const char PortName[]  = "portName";
const char BaudRate[]  = "baudRate";
const char HostName[]  = "hostName";
const char Port[]      = "port";
}

/**
 * @brief QTestGCSConfig - Test 工程配置类，继承 QGCSConfig
 *
 * 在基类基础上增加串口、地图、多链路等 Test 工程专用配置项
 */
class QTestGCSConfig : public QGCSConfig
{
    Q_OBJECT
public:
    static QTestGCSConfig *instance();

    Q_INVOKABLE QStringList refreshPortName() const;
    Q_INVOKABLE QStringList standardBaudRates() const;

    Q_INVOKABLE QString mapName() const;
    Q_INVOKABLE void setMapName(const QString &mapName);

    // ---------- 多链路配置 ----------
    /** 链路数量 */
    Q_INVOKABLE int linkCount() const;
    /** 获取第 index 条链路配置（QVariantMap），index 从 0 开始 */
    Q_INVOKABLE QVariantMap linkConfigAt(int index) const;
    /** 设置第 index 条链路配置 */
    Q_INVOKABLE void setLinkConfigAt(int index, const QVariantMap &config);
    /** 追加一条链路配置 */
    Q_INVOKABLE void appendLinkConfig(const QVariantMap &config);
    /** 删除第 index 条链路配置 */
    Q_INVOKABLE void removeLinkConfigAt(int index);
    /** 获取全部链路配置（便于 QML 一次绑定） */
    Q_INVOKABLE QVariantList linkConfigList() const;
    /** 保存链路配置到文件（修改后需调用以持久化） */
    Q_INVOKABLE void saveLinkConfigs();

    void release() override;

protected:
    void initializeDefaults() override;

private:
    QTestGCSConfig(QObject *parent = nullptr);
    ~QTestGCSConfig() override;
    Q_DISABLE_COPY(QTestGCSConfig)

    QString linkGroupKey(int index) const;

    static QTestGCSConfig *s_instance;
};

#endif // QTESTGCSCONFIG_H
