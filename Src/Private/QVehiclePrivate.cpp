#include "Private/QVehiclePrivate.h"
#include "QVehicle.h"
#include <QDebug>
#include <QDateTime>
#include <sstream>
#include <iostream>

QVehiclePrivate::QVehiclePrivate()
    : m_unID(0)
    , m_componentId(0)
    , m_autopilotType("Unknown")
    , m_vehicleType("Unknown")
    , m_firmwareVersion("Unknown")
    , m_hardwareVersion("Unknown")
    , m_softwareVersion("Unknown")
    , m_isConnected(false)
    , m_hasCamera(false)
{
}

QVehiclePrivate::~QVehiclePrivate()
{
}

void QVehiclePrivate::setUnID(uint8_t unID)
{
    m_unID = unID;
}

uint8_t QVehiclePrivate::getUnID() const
{
    return m_unID;
}

void QVehiclePrivate::setComponentId(uint8_t componentId)
{
    m_componentId = componentId;
}

uint8_t QVehiclePrivate::getComponentId() const
{
    return m_componentId;
}

void QVehiclePrivate::setAutopilotType(const QString &autopilotType)
{
    m_autopilotType = autopilotType;
}

QString QVehiclePrivate::getAutopilotType() const
{
    return m_autopilotType;
}

void QVehiclePrivate::setVehicleType(const QString &vehicleType)
{
    m_vehicleType = vehicleType;
}

QString QVehiclePrivate::getVehicleType() const
{
    return m_vehicleType;
}

void QVehiclePrivate::setFirmwareVersion(const QString &firmwareVersion)
{
    m_firmwareVersion = firmwareVersion;
}

QString QVehiclePrivate::getFirmwareVersion() const
{
    return m_firmwareVersion;
}

void QVehiclePrivate::setHardwareVersion(const QString &hardwareVersion)
{
    m_hardwareVersion = hardwareVersion;
}

QString QVehiclePrivate::getHardwareVersion() const
{
    return m_hardwareVersion;
}

void QVehiclePrivate::setSoftwareVersion(const QString &softwareVersion)
{
    m_softwareVersion = softwareVersion;
}

QString QVehiclePrivate::getSoftwareVersion() const
{
    return m_softwareVersion;
}

void QVehiclePrivate::setConnected(bool connected)
{
    m_isConnected = connected;
    
    if (connected) {
        m_lastConnectedTime = QDateTime::currentDateTime();
    } else {
        m_lastDisconnectedTime = QDateTime::currentDateTime();
    }
}

bool QVehiclePrivate::isConnected() const
{
    return m_isConnected;
}

void QVehiclePrivate::setHasCamera(bool hasCamera)
{
    m_hasCamera = hasCamera;
}

bool QVehiclePrivate::hasCamera() const
{
    return m_hasCamera;
}

void QVehiclePrivate::setComponentIds(const QVector<uint8_t> &componentIds)
{
    m_componentIds = componentIds;
}

QVector<uint8_t> QVehiclePrivate::getComponentIds() const
{
    return m_componentIds;
}

void QVehiclePrivate::setLastConnectedTime(const QDateTime &time)
{
    m_lastConnectedTime = time;
}

QDateTime QVehiclePrivate::getLastConnectedTime() const
{
    return m_lastConnectedTime;
}

void QVehiclePrivate::setLastDisconnectedTime(const QDateTime &time)
{
    m_lastDisconnectedTime = time;
}

QDateTime QVehiclePrivate::getLastDisconnectedTime() const
{
    return m_lastDisconnectedTime;
}

void QVehiclePrivate::updateFromSystem(void* system)
{
    if (!system) {
        return;
    }
    
    // 将void*转换为mavsdk::System*
    auto mavsdkSystem = static_cast<mavsdk::System*>(system);
    
    // 更新基本信息
    setUnID(mavsdkSystem->get_system_id());
    setConnected(mavsdkSystem->is_connected());
    setHasCamera(mavsdkSystem->has_camera());
    
    // 更新组件ID列表
    auto componentIds = mavsdkSystem->component_ids();
    QVector<uint8_t> qComponentIds;
    for (auto id : componentIds) {
        qComponentIds.append(id);
    }
    setComponentIds(qComponentIds);
    
    // 获取自动驾驶仪类型
    auto autopilotType = mavsdkSystem->autopilot_type();
    std::ostringstream autopilotOss;
    autopilotOss << autopilotType;
    setAutopilotType(QString::fromStdString(autopilotOss.str()));
    
    // 获取载具类型
    auto vehicleType = mavsdkSystem->vehicle_type();
    std::ostringstream vehicleOss;
    vehicleOss << vehicleType;
    setVehicleType(QString::fromStdString(vehicleOss.str()));
    
    // 尝试获取固件版本信息
    mavsdk::Info infoPlugin(*mavsdkSystem);
    
    // 注意：这些调用可能需要异步处理
    // 这里先设置默认值，实际使用时可能需要回调处理
    std::ostringstream infoOss;
    infoOss << infoPlugin.get_flight_information().second;
    
    // 这里可以添加更多的信息获取逻辑
    // 目前保持默认值
}

QString QVehiclePrivate::toString() const
{
    std::ostringstream oss;
    
    oss << "QVehicle[unID=" << static_cast<int>(m_unID) 
        << ", componentId=" << static_cast<int>(m_componentId)
        << ", autopilotType=" << m_autopilotType.toStdString()
        << ", vehicleType=" << m_vehicleType.toStdString()
        << ", firmwareVersion=" << m_firmwareVersion.toStdString()
        << ", hardwareVersion=" << m_hardwareVersion.toStdString()
        << ", softwareVersion=" << m_softwareVersion.toStdString()
        << ", isConnected=" << (m_isConnected ? "true" : "false")
        << ", hasCamera=" << (m_hasCamera ? "true" : "false")
        << ", componentIds=[";
    
    for (int i = 0; i < m_componentIds.size(); ++i) {
        if (i > 0) oss << ",";
        oss << static_cast<int>(m_componentIds[i]);
    }
    oss << "]";
    
    if (m_lastConnectedTime.isValid()) {
        oss << ", lastConnected=" << m_lastConnectedTime.toString().toStdString();
    }
    if (m_lastDisconnectedTime.isValid()) {
        oss << ", lastDisconnected=" << m_lastDisconnectedTime.toString().toStdString();
    }
    
    oss << "]";
    
    return QString::fromStdString(oss.str());
}

void QVehiclePrivate::setSystem(std::shared_ptr<mavsdk::System> system)
{
    m_system = system;
    
    if (system) {
        // 创建插件实例
        m_telemetry = std::make_unique<mavsdk::Telemetry>(*system);
        m_info = std::make_unique<mavsdk::Info>(*system);
    } else {
        m_telemetry.reset();
        m_info.reset();
    }
}

std::shared_ptr<mavsdk::System> QVehiclePrivate::getSystem() const
{
    return m_system;
}

void QVehiclePrivate::setupMessageHandling(QObject* parent)
{
    if (!m_system || !parent) {
        return;
    }
    
    // 设置遥测数据订阅
    setupTelemetrySubscriptions(parent);
    
    // 设置系统状态订阅
    setupSystemStatusSubscriptions(parent);
    
    qDebug() << "QVehiclePrivate: Message handling setup for system" << m_unID;
}

void QVehiclePrivate::setupTelemetrySubscriptions(QObject* parent)
{
    if (!m_telemetry || !parent) {
        return;
    }
    
    // 订阅位置信息
    m_telemetry->subscribe_position([parent](mavsdk::Telemetry::Position position) {
        // 这里可以发射位置更新信号
        // QMetaObject::invokeMethod(parent, "positionUpdated", Qt::QueuedConnection, ...);
    });
    
    // 订阅姿态信息
    m_telemetry->subscribe_attitude_euler([parent](mavsdk::Telemetry::EulerAngle attitude) {
        // 这里可以发射姿态更新信号
        // QMetaObject::invokeMethod(parent, "attitudeUpdated", Qt::QueuedConnection, ...);
    });
    
    // 订阅电池状态
    m_telemetry->subscribe_battery([parent](mavsdk::Telemetry::Battery battery) {
        // 这里可以发射电池状态更新信号
        // QMetaObject::invokeMethod(parent, "batteryUpdated", Qt::QueuedConnection, ...);
    });
    
    // 订阅飞行状态
    m_telemetry->subscribe_flight_mode([parent](mavsdk::Telemetry::FlightMode flightMode) {
        // 这里可以发射飞行状态更新信号
        // QMetaObject::invokeMethod(parent, "flightModeUpdated", Qt::QueuedConnection, ...);
    });
    
    qDebug() << "QVehiclePrivate: Telemetry subscriptions setup for system" << m_unID;
}

void QVehiclePrivate::setupSystemStatusSubscriptions(QObject* parent)
{
    if (!m_system || !parent) {
        return;
    }
    
    // 订阅系统连接状态变化
    m_system->subscribe_is_connected([this, parent](bool isConnected) {
        setConnected(isConnected);
        
        // 发射连接状态变化信号
        QMetaObject::invokeMethod(parent, "connectionStatusChanged", Qt::QueuedConnection,
                                  Q_ARG(bool, isConnected));
        
        if (isConnected) {
            QMetaObject::invokeMethod(parent, "connected", Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(parent, "disconnected", Qt::QueuedConnection,
                                      Q_ARG(QString, "System disconnected"));
        }
    });
    
    // 订阅组件发现
    m_system->subscribe_component_discovered([this, parent](mavsdk::ComponentType componentType) {
        // 这里可以处理新组件发现
        qDebug() << "QVehiclePrivate: Component discovered:" << static_cast<int>(componentType);
    });
    
    qDebug() << "QVehiclePrivate: System status subscriptions setup for system" << m_unID;
}
