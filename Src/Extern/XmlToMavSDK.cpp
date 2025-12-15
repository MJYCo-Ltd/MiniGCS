#include "Extern/XmlToMavSDK.h"
#include "QGCSConfig.h"

XmlToMavSDK::XmlToMavSDK(const QString& xmlPath)
{
    loadXml(xmlPath);
}

// 查找命令
const XmlToMavSDK::ExternCmd* XmlToMavSDK::findCmd(const QString& name) const
{
    auto it = m_mapExternCMDs.find(name);
    if (it != m_mapExternCMDs.end()) return &it.value();
    return nullptr;
}

// 列出所有命令
QStringList XmlToMavSDK::listCmdNames() const {
    return m_mapExternCMDs.keys();
}

void XmlToMavSDK::setSystem(std::shared_ptr<mavsdk::System> system)
{
    m_pSystem = system;
    m_pMavlinkDirect = std::make_shared<mavsdk::MavlinkDirect>(system);
}

// 发送命令（参数数量不足时自动补0）
mavsdk::MavlinkDirect::Result XmlToMavSDK::sendCmd(const QString& name, uint32_t uComponentID, const QVector<float>& params)
{
    if(!m_bLoadXml)
    {
        return mavsdk::MavlinkDirect::Result::Unknown;
    }

    const ExternCmd* cmd = findCmd(name);
    if (!cmd)
    {
        qWarning() << "Command not found:" << name;
        return mavsdk::MavlinkDirect::Result::Unknown;
    }

    QVector<float> realParams = params;
    while (realParams.size() < 7) realParams.append(0);

    // 构建 COMMAND_LONG 消息的 JSON 字段
    QString fieldsJson = QString(R"({
        "param1": %1,
        "param2": %2,
        "param3": %3,
        "param4": %4,
        "param5": %5,
        "param6": %6,
        "param7": %7
    })")
    .arg(realParams[0])
    .arg(realParams[1])
    .arg(realParams[2])
    .arg(realParams[3])
    .arg(realParams[4])
    .arg(realParams[5])
    .arg(realParams[6]);

    mavsdk::MavlinkDirect::MavlinkMessage message;
    message.message_name = cmd->name.toStdString();
    message.component_id = QGCSConfig::instance()->gcsComponentId();
    message.system_id = QGCSConfig::instance()->gcsSystemId();
    message.target_system_id = m_pSystem->get_system_id();
    message.target_component_id = uComponentID;
    message.fields_json = fieldsJson.toStdString();

    return m_pMavlinkDirect->send_message(message);
}

void XmlToMavSDK::loadXml(const QString& xmlPath) {
    QFile file(xmlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open xml:" << xmlPath;
        return;
    }

    if(mavsdk::MavlinkDirect::Result::Success == m_pMavlinkDirect->load_custom_xml(file.readAll().toStdString()))
    {
        m_bLoadXml = true;
    }

    QXmlStreamReader xml(&file);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "enum" &&
            xml.attributes().value("name") == "MAV_CMD") {
            // 进入 MAV_CMD 枚举
            while (!(xml.isEndElement() && xml.name() == "enum")) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == "entry") {
                    ExternCmd cmd;
                    cmd.name = xml.attributes().value("name").toString();
                    cmd.value = xml.attributes().value("value").toString().toUShort();
                    // 处理内部
                    while (!(xml.isEndElement() && xml.name() == "entry")) {
                        xml.readNext();
                        if (xml.isStartElement()) {
                            if (xml.name() == "description") {
                                cmd.description = xml.readElementText();
                            } else if (xml.name() == "param") {
                                CommandParam param;
                                param.index = xml.attributes().value("index").toInt();
                                param.label = xml.attributes().hasAttribute("label")
                                                  ? xml.attributes().value("label").toString()
                                                  : QString("param%1").arg(param.index);
                                cmd.params.append(param);
                            }
                        }
                    }
                    m_mapExternCMDs[cmd.name] = cmd;
                }
            }
        }
    }
    if (xml.hasError()) {
        qWarning() << "XML parse error:" << xml.errorString();
    }
    file.close();
}
