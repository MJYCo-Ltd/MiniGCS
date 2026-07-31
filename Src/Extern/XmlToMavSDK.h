#pragma once

#include <QString>
#include <QVector>
#include <QMap>
#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/mavlink_direct/mavlink_direct.h>

/**
 * @brief APM/ArduPilot 扩展 MAV_CMD 与自定义消息适配
 *
 * - 解析扩展 XML 中的 MAV_CMD 表，供按命令名发送
 * - 将扩展 XML 注入 MAVSDK（整站一次，由 GCS 调用 applyCustomXmlOnce）
 */
class XmlToMavSDK
{
public:
    struct CommandParam {
        QString label;
        int index;
    };

    struct ExternCmd {
        QString name;
        uint16_t value;
        QString description;
        QVector<CommandParam> params;
    };

    explicit XmlToMavSDK(const QString& xmlPath = QString());

    bool loadXml(const QString& xmlPath);
    bool isCmdTableLoaded() const { return m_bCmdTableLoaded; }
    bool hasXmlContent() const { return !m_xmlContent.empty(); }
    const std::string& xmlContent() const { return m_xmlContent; }

    /**
     * @brief 将已缓存的扩展 XML 注入 MAVSDK（只真正执行一次）
     * @return 本次实际执行了 load 时返回结果；已完成/其他线程正在执行时返回 nullopt
     */
    std::optional<mavsdk::MavlinkDirect::Result> applyCustomXmlOnce(
        mavsdk::MavlinkDirect& mavlinkDirect);

    bool isCustomXmlApplied() const { return m_customXmlApplied.load(); }

    const ExternCmd* findCmd(const QString& name) const;
    QStringList listCmdNames() const;

    void setSystem(std::shared_ptr<mavsdk::System> system);

    mavsdk::MavlinkDirect::Result sendCmd(
        const QString& name,
        uint32_t uComponentID,
        const QVector<float>& params);

    mavsdk::MavlinkDirect::Result sendCmd(
        mavsdk::MavlinkDirect& mavlinkDirect,
        const mavsdk::System& system,
        const QString& name,
        uint32_t uComponentID,
        const QVector<float>& params) const;

private:
    QMap<QString, ExternCmd> m_mapExternCMDs;
    std::string m_xmlContent;
    std::shared_ptr<mavsdk::MavlinkDirect> m_pMavlinkDirect;
    std::shared_ptr<mavsdk::System> m_pSystem;
    bool m_bCmdTableLoaded{false};
    std::atomic<bool> m_customXmlApplied{false};
    std::atomic<bool> m_customXmlApplyStarted{false};
};
