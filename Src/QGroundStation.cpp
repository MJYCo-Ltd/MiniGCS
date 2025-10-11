#include "QGroundStation.h"
#include "Private/QGroundStationPrivate.h"
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <chrono>
#include <sstream>
#include <mavsdk/mavsdk.h>
#include <mavsdk/system.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/info/info.h>

QGroundStation::QGroundStation(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<QGroundStationPrivate>())
{
    d_ptr->initializeMavsdk();
    d_ptr->setupConnectionErrorHandling(this);
    
    // 设置定时器，每2秒检查一次系统状态
    auto timer = new QTimer(this);
    d_ptr->setStatusTimer(timer);
    timer->setInterval(2000);
    connect(timer, &QTimer::timeout, this, &QGroundStation::checkSystemStatus);
    timer->start();
}

QGroundStation::~QGroundStation()
{
    disconnect();
}

bool QGroundStation::connectToVehicle(ConnectionType connectionType, const QString &connectionString)
{
    QString fullConnectionString = d_ptr->generateConnectionString(static_cast<int>(connectionType), connectionString);
    bool success = d_ptr->connectToVehicle(fullConnectionString);
    
    if (success) {
        // 等待系统连接后设置连接信息
        QTimer::singleShot(1000, [this, connectionType, fullConnectionString]() {
            auto systemIds = getConnectedSystemIds();
            for (uint8_t systemId : systemIds) {
                d_ptr->setSystemConnectionInfo(systemId, static_cast<int>(connectionType), fullConnectionString);
            }
        });
    }
    
    return success;
}

void QGroundStation::disconnect()
{
    d_ptr->disconnect();
}

QVector<uint8_t> QGroundStation::getConnectedSystemIds() const
{
    QVector<uint8_t> result;
    auto systems = d_ptr->getConnectedSystems();
    
    for (auto system : systems) {
        result.append(system->get_system_id());
    }
    
    return result;
}

VehicleInfo QGroundStation::getVehicleInfo(uint8_t systemId) const
{
    auto system = d_ptr->getSystem(systemId);
    if (system) {
        return extractVehicleInfo(system);
    }
    
    return VehicleInfo();
}

QVector<VehicleInfo> QGroundStation::getAllVehicleInfo() const
{
    QVector<VehicleInfo> result;
    auto systems = d_ptr->getConnectedSystems();
    
    for (auto system : systems) {
        result.append(extractVehicleInfo(system.get()));
    }
    
    return result;
}

bool QGroundStation::isConnected() const
{
    return d_ptr->isConnected();
}

int QGroundStation::getSystemCount() const
{
    return d_ptr->getSystemCount();
}

void* QGroundStation::getSystemHandle(uint8_t systemId) const
{
    return d_ptr->getSystem(systemId);
}

void QGroundStation::setConnectionConfig(const ConnectionConfig &config)
{
    d_ptr->setConnectionConfig(config);
}

ConnectionConfig QGroundStation::getConnectionConfig() const
{
    return d_ptr->getConnectionConfig();
}

void QGroundStation::setHeartbeatTimeout(int timeoutMs)
{
    d_ptr->setHeartbeatTimeout(timeoutMs);
}

void QGroundStation::setAutoReconnect(bool autoReconnect)
{
    d_ptr->setAutoReconnect(autoReconnect);
}

bool QGroundStation::reconnectSystem(uint8_t systemId)
{
    return d_ptr->reconnectSystem(systemId, this);
}

void QGroundStation::checkSystemStatus()
{
    d_ptr->checkSystemStatus(this);
    
    // 发射飞控信息更新信号
    auto systems = d_ptr->getConnectedSystems();
    for (auto system : systems) {
        VehicleInfo info = extractVehicleInfo(system.get());
        emit vehicleInfoUpdated(info);
        
        // 如果是新连接的系统，发射vehicleConnected信号
        uint8_t systemId = system->get_system_id();
        if (d_ptr->getLastHeartbeatTime(systemId) > 0) {
            // 检查是否是最近连接的系统（心跳时间在合理范围内）
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            qint64 lastHeartbeat = d_ptr->getLastHeartbeatTime(systemId);
            if (currentTime - lastHeartbeat < 10000) { // 10秒内认为是新连接
                emit vehicleConnected(systemId, info);
            }
        }
    }
}

VehicleInfo QGroundStation::extractVehicleInfo(void* system) const
{
    VehicleInfo info;
    
    if (!system) {
        return info;
    }
    
    // 将void*转换为mavsdk::System*
    auto mavsdkSystem = static_cast<mavsdk::System*>(system);
    
    info.systemId = mavsdkSystem->get_system_id();
    info.componentIds = mavsdkSystem->component_ids();
    info.isConnected = mavsdkSystem->is_connected();
    info.hasCamera = mavsdkSystem->has_camera();
    
    // 获取心跳信息
    info.lastHeartbeatTime = d_ptr->getLastHeartbeatTime(info.systemId);
    info.isHeartbeatActive = d_ptr->isHeartbeatActive(info.systemId);
    
    // 获取自动驾驶仪类型
    auto autopilotType = mavsdkSystem->autopilot_type();
    std::ostringstream autopilotOss;
    autopilotOss << autopilotType;
    info.autopilotType = QString::fromStdString(autopilotOss.str());
    
    // 获取载具类型
    auto vehicleType = mavsdkSystem->vehicle_type();
    std::ostringstream vehicleOss;
    vehicleOss << vehicleType;
    info.vehicleType = QString::fromStdString(vehicleOss.str());
    
    // 尝试获取固件版本信息
    mavsdk::Info infoPlugin(*mavsdkSystem);

    // 注意：这些调用可能需要异步处理
    // 这里先设置默认值，实际使用时可能需要回调处理
    std::ostringstream infoOss;
    infoOss << infoPlugin.get_flight_information().second;

    info.firmwareVersion = infoPlugin.get_product();
    info.hardwareVersion = "Unknown";
    info.softwareVersion = "Unknown";
    
    return info;
}

