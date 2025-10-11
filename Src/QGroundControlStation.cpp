#include "QGroundControlStation.h"
#include "Private/QGroundControlStationPrivate.h"
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <chrono>
#include <sstream>
#include <mavsdk/mavsdk.h>
#include <mavsdk/system.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/info/info.h>

QGroundControlStation::QGroundControlStation(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<QGroundControlStationPrivate>())
{
    d_ptr->initializeMavsdk();
    d_ptr->setupConnectionErrorHandling(this);
    d_ptr->setupSystemConnectionCallbacks(this);
}

QGroundControlStation::~QGroundControlStation()
{
    disconnect();
}

bool QGroundControlStation::connectToDataLink(ConnectionType connectionType, const QString &address, int portOrBaudRate)
{
    bool success = d_ptr->connectToDataLink(static_cast<int>(connectionType), address, portOrBaudRate);
    
    if (success) {
        // 等待系统连接后设置连接信息和回调
        QTimer::singleShot(1000, [this, connectionType, address, portOrBaudRate]() {
            auto vehicleIds = getVehicleIDs();
            QString connectionString = d_ptr->generateConnectionString(static_cast<int>(connectionType), address, portOrBaudRate);
            for (uint8_t systemId : vehicleIds) {
                d_ptr->setSystemConnectionInfo(systemId, static_cast<int>(connectionType), connectionString);
            }
            // 设置连接状态回调
            d_ptr->setupSystemConnectionCallbacks(this);
        });
    }
    
    return success;
}

void QGroundControlStation::disconnect()
{
    d_ptr->disconnect();
}

QVector<uint8_t> QGroundControlStation::getVehicleIDs() const
{
    QVector<uint8_t> result;
    auto systems = d_ptr->getConnectedSystems();
    
    for (auto system : systems) {
        result.append(system->get_system_id());
    }
    
    return result;
}

VehicleInfo QGroundControlStation::getVehicleInfo(uint8_t systemId) const
{
    auto system = d_ptr->getSystem(systemId);
    if (system) {
        return extractVehicleInfo(system);
    }
    
    return VehicleInfo();
}

QVector<VehicleInfo> QGroundControlStation::getAllVehicleInfo() const
{
    QVector<VehicleInfo> result;
    auto systems = d_ptr->getConnectedSystems();
    
    for (auto system : systems) {
        result.append(extractVehicleInfo(system.get()));
    }
    
    return result;
}

bool QGroundControlStation::isConnected() const
{
    return d_ptr->isConnected();
}

int QGroundControlStation::getSystemCount() const
{
    return d_ptr->getSystemCount();
}

void* QGroundControlStation::getSystemHandle(uint8_t systemId) const
{
    return d_ptr->getSystem(systemId);
}

void QGroundControlStation::setConnectionConfig(const ConnectionConfig &config)
{
    d_ptr->setConnectionConfig(config);
}

ConnectionConfig QGroundControlStation::getConnectionConfig() const
{
    return d_ptr->getConnectionConfig();
}

void QGroundControlStation::setAutoReconnect(bool autoReconnect)
{
    d_ptr->setAutoReconnect(autoReconnect);
}

bool QGroundControlStation::reconnectSystem(uint8_t systemId)
{
    return d_ptr->reconnectSystem(systemId, this);
}


VehicleInfo QGroundControlStation::extractVehicleInfo(void* system) const
{
    VehicleInfo info;
    
    if (!system) {
        return info;
    }
    
    // 将void*转换为mavsdk::System*
    auto mavsdkSystem = static_cast<mavsdk::System*>(system);
    
    info.unID = mavsdkSystem->get_system_id();
    info.componentIds = mavsdkSystem->component_ids();
    info.isConnected = mavsdkSystem->is_connected();
    info.hasCamera = mavsdkSystem->has_camera();
    
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

    info.firmwareVersion;
    info.hardwareVersion = "Unknown";
    info.softwareVersion = "Unknown";
    
    return info;
}

uint8_t QGroundControlStation::getGroundStationSystemId() const
{
    return d_ptr->getGroundStationSystemId();
}

uint8_t QGroundControlStation::getGroundStationComponentId() const
{
    return d_ptr->getGroundStationComponentId();
}
