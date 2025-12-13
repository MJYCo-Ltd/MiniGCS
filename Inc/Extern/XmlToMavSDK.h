#ifndef XMLTOMAVSDK_H
#define XMLTOMAVSDK_H

#pragma once

#include <QString>
#include <QFile>
#include <QXmlStreamReader>
#include <QVector>
#include <QMap>
#include <QDebug>
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

    // 查找命令
    const ExternCmd* findCmd(const QString& name) const;

    // 列出所有命令
    QStringList listCmdNames() const;

    /**
     * @brief 设置要发送给的固件
     * @param system
     */
    void setSystem(std::shared_ptr<mavsdk::System> system);

    // 发送命令（参数数量不足时自动补0）
    mavsdk::MavlinkDirect::Result sendCmd(const QString& name,uint32_t uComponentID,const QVector<float>& params);
protected:
    void loadXml(const QString& xmlPath);
private:
    QMap<QString, ExternCmd> m_mapExternCMDs;
    std::shared_ptr<mavsdk::MavlinkDirect> m_pMavlinkDirect;
    std::shared_ptr<mavsdk::System> m_pSystem;
    bool m_bLoadXml{false};
};

#endif // XMLTOMAVSDK_H
