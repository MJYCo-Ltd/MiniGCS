#include "Private/QPlatPrivate.h"

#include <QDateTime>
#include <QDebug>
#include <sstream>
#include "QGCSConfig.h"
QPlatPrivate::QPlatPrivate()
    : m_firmwareVersion("Unknown"), m_hardwareVersion("Unknown"),
    m_softwareVersion("Unknown") {}

QPlatPrivate::~QPlatPrivate() {}

QString QPlatPrivate::toString() const {
    std::ostringstream oss;

    oss << "MavlinkInfo:\n"
        << "systemId=" << m_pSystem->get_system_id()
        << "\nfirmwareVersion=" << m_firmwareVersion.toStdString()
        << "\nhardwareVersion=" << m_hardwareVersion.toStdString()
        << "\nsoftwareVersion=" << m_softwareVersion.toStdString()
        << "\nhasCamera=" << (m_pSystem->has_camera() ? "true" : "false")
        << "\nhas" << m_pSystem->has_gimbal() << "\ncomponentIds=[";

    for (auto one : m_pSystem->component_ids()) {
        oss << one << ',';
    }
    oss << "]";

    return QString::fromStdString(oss.str());
}
#include <QFile>
void QPlatPrivate::setSystem(std::shared_ptr<mavsdk::System> system) {

	/// 如果原来的system 不为空，取消订阅
    if (nullptr != m_pSystem) {
        m_pSystem->unsubscribe_is_connected(m_hConntecd);
        m_pSystem->unsubscribe_component_discovered(m_hCommonpentDiscovered);
    }
    m_pSystem = system;

    // 创建插件实例
    m_pMavlinkPassthrough = std::make_unique<mavsdk::MavlinkPassthrough>(*system);
    m_pMavlinkDirect = std::make_unique<mavsdk::MavlinkDirect>(*system);
	m_pEvents = std::make_unique<mavsdk::Events>(*system);
    QFile file("E:/Code/Git/MJY/MiniGCS/Depends/include/mavsdk/mavlink/message_definitions/v1.0/ardupilotmega.xml");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        auto result = m_pMavlinkDirect->load_custom_xml(file.readAll().toStdString());
        std::ostringstream oss;
        oss<<result;
        qDebug()<<oss.str();
    }

}

std::shared_ptr<mavsdk::System> QPlatPrivate::getSystem() const {
    return m_pSystem;
}

void QPlatPrivate::setupMessageHandling(QObject *parent) {
    if (!m_pSystem || !parent) {
        return;
    }

    // 订阅系统连接状态变化
    m_hConntecd = m_pSystem->subscribe_is_connected([this, parent](bool isConnected) {
        // 发射连接状态变化信号
        QMetaObject::invokeMethod(parent, "connectionStatusChanged",
                                  Qt::QueuedConnection, Q_ARG(bool, isConnected));
    });

    // 订阅组件发现
    m_hCommonpentDiscovered = m_pSystem->subscribe_component_discovered(
        [this, parent](mavsdk::ComponentType componentType) {
            // 这里可以处理新组件发现
            qDebug() << "QVehiclePrivate: Component discovered:"
                     << static_cast<int>(componentType);
        });

    m_pEvents->subscribe_events([](mavsdk::Events::Event event){
        qDebug()<<event.event_name;
        QGCSConfig::instance()->dealMavsdkLog(event);
    });
}
