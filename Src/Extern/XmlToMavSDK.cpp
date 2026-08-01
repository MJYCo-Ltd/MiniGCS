#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>

#include "Extern/XmlToMavSDK.h"
#include "QGCSConfig.h"

XmlToMavSDK::XmlToMavSDK(const QString& xmlPath)
{
    if (!xmlPath.isEmpty()) {
        loadXml(xmlPath);
    }
}

bool XmlToMavSDK::isDefaultArdupilotDialectFile(const QString& xmlPath)
{
    return QFileInfo(xmlPath).fileName().compare(
               QStringLiteral("ardupilotmega.xml"), Qt::CaseInsensitive) == 0;
}

const XmlToMavSDK::ExternCmd* XmlToMavSDK::findCmd(const QString& name) const
{
    auto it = m_mapExternCMDs.find(name);
    if (it != m_mapExternCMDs.end()) {
        return &it.value();
    }
    return nullptr;
}

QStringList XmlToMavSDK::listCmdNames() const
{
    return m_mapExternCMDs.keys();
}

std::optional<mavsdk::MavlinkDirect::Result> XmlToMavSDK::applyCustomXmlOnce(
    mavsdk::MavlinkDirect& mavlinkDirect)
{
    if (!m_needsMessageSetInject) {
        m_customXmlApplied.store(true);
        return std::nullopt;
    }
    if (m_customXmlApplied.load()) {
        return std::nullopt;
    }
    if (m_xmlContent.empty()) {
        m_customXmlApplied.store(true);
        return mavsdk::MavlinkDirect::Result::Unknown;
    }

    bool expected = false;
    if (!m_customXmlApplyStarted.compare_exchange_strong(expected, true)) {
        return std::nullopt;
    }

    /// 写入 MavsdkImpl 共享 MessageSet，对所有 System 立即生效
    const auto result = mavlinkDirect.load_custom_xml(m_xmlContent);
    if (result == mavsdk::MavlinkDirect::Result::Success) {
        m_customXmlApplied.store(true);
    } else {
        /// 失败时允许后续系统发现或重连再次尝试。
        m_customXmlApplyStarted.store(false);
    }
    return result;
}

mavsdk::MavlinkDirect::Result XmlToMavSDK::sendCmd(
    mavsdk::MavlinkDirect& mavlinkDirect,
    const mavsdk::System& system,
    const QString& name,
    uint32_t uComponentID,
    const QVector<float>& params) const
{
    if (!m_bCmdTableLoaded) {
        return mavsdk::MavlinkDirect::Result::Unknown;
    }

    const ExternCmd* cmd = findCmd(name);
    if (!cmd) {
        qWarning() << "Command not found:" << name;
        return mavsdk::MavlinkDirect::Result::Unknown;
    }

    QVector<float> realParams = params;
    while (realParams.size() < 7) {
        realParams.append(0);
    }

    const QString fieldsJson = QStringLiteral(
        R"({"command":%1,"confirmation":0,"param1":%2,"param2":%3,"param3":%4,"param4":%5,"param5":%6,"param6":%7,"param7":%8})")
        .arg(cmd->value)
        .arg(realParams[0], 0, 'g', 8)
        .arg(realParams[1], 0, 'g', 8)
        .arg(realParams[2], 0, 'g', 8)
        .arg(realParams[3], 0, 'g', 8)
        .arg(realParams[4], 0, 'g', 8)
        .arg(realParams[5], 0, 'g', 8)
        .arg(realParams[6], 0, 'g', 8);

    mavsdk::MavlinkDirect::MavlinkMessage message;
    message.message_name = "COMMAND_LONG";
    message.component_id = QGCSConfig::instance()->stationComponentId();
    message.system_id = QGCSConfig::instance()->stationId();
    message.target_system_id = system.get_system_id();
    message.target_component_id = uComponentID;
    message.fields_json = fieldsJson.toStdString();

    return mavlinkDirect.send_message(message);
}

bool XmlToMavSDK::loadXml(const QString& xmlPath)
{
    m_mapExternCMDs.clear();
    m_xmlContent.clear();
    m_bCmdTableLoaded = false;
    m_needsMessageSetInject = false;
    m_customXmlApplied.store(false);
    m_customXmlApplyStarted.store(false);

    QFile file(xmlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open xml:" << xmlPath;
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();
    if (data.isEmpty()) {
        qWarning() << "Empty mav message extension xml:" << xmlPath;
        return false;
    }

    QXmlStreamReader xml(data);
    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == QLatin1String("enum") &&
            xml.attributes().value(QLatin1String("name")) ==
                QLatin1String("MAV_CMD")) {
            while (!(xml.isEndElement() && xml.name() == QLatin1String("enum"))) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == QLatin1String("entry")) {
                    ExternCmd cmd;
                    cmd.name = xml.attributes().value(QLatin1String("name")).toString();
                    cmd.value =
                        xml.attributes().value(QLatin1String("value")).toString().toUShort();
                    while (!(xml.isEndElement() &&
                             xml.name() == QLatin1String("entry"))) {
                        xml.readNext();
                        if (xml.isStartElement()) {
                            if (xml.name() == QLatin1String("description")) {
                                cmd.description = xml.readElementText();
                            } else if (xml.name() == QLatin1String("param")) {
                                CommandParam param;
                                param.index =
                                    xml.attributes().value(QLatin1String("index")).toInt();
                                param.label =
                                    xml.attributes().hasAttribute(QLatin1String("label"))
                                        ? xml.attributes()
                                              .value(QLatin1String("label"))
                                              .toString()
                                        : QStringLiteral("param%1").arg(param.index);
                                cmd.params.append(param);
                            }
                        }
                    }
                    if (!cmd.name.isEmpty()) {
                        m_mapExternCMDs.insert(cmd.name, cmd);
                    }
                }
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "XML parse error:" << xml.errorString() << "in" << xmlPath;
        m_mapExternCMDs.clear();
        return false;
    }

    m_xmlContent = data.toStdString();
    m_bCmdTableLoaded = true;
    /// 非默认方言文件才需要写入共享 MessageSet；ardupilotmega 已由 MAVSDK 内嵌
    m_needsMessageSetInject = !isDefaultArdupilotDialectFile(xmlPath);
    if (!m_needsMessageSetInject) {
        m_customXmlApplied.store(true);
    }
    return true;
}
