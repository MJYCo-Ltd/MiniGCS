#pragma once

#include <QString>
#include <QFile>
#include <QXmlStreamReader>
#include <QVector>
#include <QMap>
#include <memory>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/mavlink_direct/mavlink_direct.h>

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

    explicit XmlToMavSDK(const QString& xmlPath);

    const ExternCmd* findCmd(const QString& name) const;
    QStringList listCmdNames() const;
    void setSystem(std::shared_ptr<mavsdk::System> system);
    mavsdk::MavlinkDirect::Result sendCmd(
        const QString& name,
        uint32_t uComponentID,
        const QVector<float>& params);
    void loadXml(const QString& xmlPath);

private:
    QMap<QString, ExternCmd> m_mapExternCMDs;
    std::shared_ptr<mavsdk::MavlinkDirect> m_pMavlinkDirect;
    std::shared_ptr<mavsdk::System> m_pSystem;
    bool m_bLoadXml{false};
};
