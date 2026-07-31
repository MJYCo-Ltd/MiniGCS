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
 * @brief APM/ArduPilot 扩展命令适配
 *
 * MAVSDK 一个 Mavsdk 实例只有一份共享 MessageSet（解析表），按 system_id 分发。
 * 因此：
 * - 默认 ardupilotmega 方言已在 MAVSDK 启动时内嵌，本类主要解析 MAV_CMD 表供按名发送
 * - 仅当配置指向「额外」自定义 XML 时，才向共享 MessageSet 注入一次
 * - 每机 MavlinkDirect 只负责订阅/发送，不负责再加载方言
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
     * @brief 是否需要把该 XML 注入共享 MessageSet
     *
     * 默认 ardupilotmega.xml 已被 MAVSDK 内嵌，只需作命令表；其它自定义文件才注入。
     */
    bool needsMessageSetInject() const { return m_needsMessageSetInject; }

    /**
     * @brief 将已缓存的扩展 XML 注入 MAVSDK 共享 MessageSet（整站只执行一次）
     * @return 本次实际执行了 load 时返回结果；已完成/无需注入/其他线程正在执行时返回 nullopt
     */
    std::optional<mavsdk::MavlinkDirect::Result> applyCustomXmlOnce(
        mavsdk::MavlinkDirect& mavlinkDirect);

    bool isCustomXmlApplied() const { return m_customXmlApplied.load(); }

    const ExternCmd* findCmd(const QString& name) const;
    QStringList listCmdNames() const;

    mavsdk::MavlinkDirect::Result sendCmd(
        mavsdk::MavlinkDirect& mavlinkDirect,
        const mavsdk::System& system,
        const QString& name,
        uint32_t uComponentID,
        const QVector<float>& params) const;

private:
    static bool isDefaultArdupilotDialectFile(const QString& xmlPath);

    QMap<QString, ExternCmd> m_mapExternCMDs;
    std::string m_xmlContent;
    bool m_bCmdTableLoaded{false};
    bool m_needsMessageSetInject{false};
    std::atomic<bool> m_customXmlApplied{false};
    std::atomic<bool> m_customXmlApplyStarted{false};
};
