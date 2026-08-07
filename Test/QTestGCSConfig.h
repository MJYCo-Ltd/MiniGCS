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
 * @brief 单条链路配置的 QVariantMap / QML 键名（camelCase）
 * type: Serial|TcpServer|TcpClient|UdpServer|UdpClient
 * name: 可选显示名称
 * portName, baudRate: Serial 专用
 * hostName, port: TcpClient/UdpClient 为远端；TcpServer/UdpServer 中
 * hostName 为绑定地址（可空，空则底层使用 0.0.0.0）
 *
 * INI 文件内仍使用 PascalCase（Type/Name/...），由 QTestGCSConfig 做映射。
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
    /** 串口默认波特率（新建链路 / QML 回落） */
    Q_INVOKABLE int defaultBaudRate() const;

    Q_INVOKABLE QString mapName() const;
    Q_INVOKABLE double mapCenterLatitude() const;
    Q_INVOKABLE double mapCenterLongitude() const;
    Q_INVOKABLE double mapInitialZoom() const;
    Q_INVOKABLE double mapVehicleZoom() const;
    Q_INVOKABLE double mapMinimumZoom() const;
    Q_INVOKABLE double mapMaximumZoom() const;
    Q_INVOKABLE double missionDefaultAltitude() const;
    Q_INVOKABLE double missionMinimumAltitude() const;
    Q_INVOKABLE double missionMaximumAltitude() const;
    qint64 flightRecordMinimumSampleIntervalMs() const;
    double flightRecordMinimumSampleDistanceM() const;
    int flightRecordMaximumCount() const;
    Q_INVOKABLE bool setMapConfiguration(const QVariantMap &config);

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

    // ---------- 无人机与编组配置 ----------
    Q_INVOKABLE QString droneName(int systemId) const;
    Q_INVOKABLE void setDroneName(int systemId, const QString &name);
    Q_INVOKABLE QVariantList droneGroupList() const;
    Q_INVOKABLE bool addDroneGroup(const QString &name);
    Q_INVOKABLE bool removeDroneGroup(const QString &name);
    Q_INVOKABLE bool setDroneGroupMembers(
        const QString &name, const QVariantList &systemIds);

    void release() override;

signals:
    void mapConfigurationChanged();

protected:
    void initializeDefaults() override;

private:
    QTestGCSConfig(QObject *parent = nullptr);
    ~QTestGCSConfig() override;
    Q_DISABLE_COPY(QTestGCSConfig)

    QString linkGroupKey(int index) const;
    QString resolveLinkGroup(int index) const;
    QString droneGroupKey(int index) const;
    int findDroneGroup(const QString &name) const;

    static QTestGCSConfig *s_instance;
};

#endif // QTESTGCSCONFIG_H
