#include "Private/QPlatPrivate.h"
#include "AsyncSendMavLink.h"
#include "QAutopilot.h"
#include <QDateTime>
#include <QDebug>
#include <iostream>
#include <sstream>
QPlatPrivate::QPlatPrivate()
    :m_firmwareVersion("Unknown"),
    m_hardwareVersion("Unknown"), m_softwareVersion("Unknown") {}

QPlatPrivate::~QPlatPrivate() {}


QString QPlatPrivate::getFirmwareVersion() const {
    return m_firmwareVersion;
}

void QPlatPrivate::setHardwareVersion(const QString &hardwareVersion) {
    m_hardwareVersion = hardwareVersion;
}

QString QPlatPrivate::getHardwareVersion() const {
    return m_hardwareVersion;
}

void QPlatPrivate::setSoftwareVersion(const QString &softwareVersion) {
    m_softwareVersion = softwareVersion;
}

QString QPlatPrivate::getSoftwareVersion() const {
    return m_softwareVersion;
}

bool QPlatPrivate::hasCamera() const { return m_system->has_camera(); }

QString QPlatPrivate::toString() const {
    std::ostringstream oss;

    oss << "MavlinkInfo:\n"
        << "systemId=" << m_system->get_system_id()
        << "\nfirmwareVersion=" << m_firmwareVersion.toStdString()
        << "\nhardwareVersion=" << m_hardwareVersion.toStdString()
        << "\nsoftwareVersion=" << m_softwareVersion.toStdString()
        << "\nhasCamera=" << (m_system->has_camera() ? "true" : "false")
        << "\nhas" << m_system->has_gimbal() << "\ncomponentIds=[";

    for (auto one:m_system->component_ids()) {
        oss<<one<<',';
    }
    oss << "]";


    return QString::fromStdString(oss.str());
}

void QPlatPrivate::setSystem(std::shared_ptr<mavsdk::System> system) {
    m_system = system;
    // 创建插件实例
    mavlinkPassthrough = std::make_unique<mavsdk::MavlinkPassthrough>(*system);
}

std::shared_ptr<mavsdk::System> QPlatPrivate::getSystem() const {
    return m_system;
}

void QPlatPrivate::setupMessageHandling(QObject *parent) {
    if (!m_system || !parent) {
        return;
    }
    // 订阅系统连接状态变化
    m_system->subscribe_is_connected([this, parent](bool isConnected) {
        // 发射连接状态变化信号
        QMetaObject::invokeMethod(parent, "connectionStatusChanged",
                                  Qt::QueuedConnection, Q_ARG(bool, isConnected));
    });

    // 订阅组件发现
    m_system->subscribe_component_discovered(
        [this, parent](mavsdk::ComponentType componentType) {
            // 这里可以处理新组件发现
            qDebug() << "QVehiclePrivate: Component discovered:"
                     << static_cast<int>(componentType);
        });
}
