#include "Private/QDataLinkPrivate.h"
#include "QDataLink.h"
#include "QVehicle.h"
#include <QDebug>
#include <QMetaObject>
#include <QMetaMethod>
#include <sstream>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/info/info.h>

QDataLinkPrivate::QDataLinkPrivate()
    : m_isInitialized(false)
    , m_groundStationSystemId(246)      // 默认地面站系统ID
    , m_groundStationComponentId(191)   // 默认地面站组件ID
{
}

QDataLinkPrivate::~QDataLinkPrivate()
{
    disconnect();
}

void QDataLinkPrivate::initializeMavsdk()
{
    if (m_isInitialized) {
        return;
    }
    
    // 创建MAVSDK实例，配置为地面站模式
    mavsdk::Mavsdk::Configuration config(
        mavsdk::ComponentType::GroundStation);
    config.set_system_id(m_groundStationSystemId);
    config.set_component_id(m_groundStationComponentId);
    m_mavsdk = std::make_shared<mavsdk::Mavsdk>(config);
    
    m_isInitialized = true;
    qDebug() << "QDataLinkPrivate: MAVSDK initialized with system ID:" 
             << m_groundStationSystemId << "component ID:" << m_groundStationComponentId;
}

bool QDataLinkPrivate::connectToDataLink(int connectionType, const QString &address, int portOrBaudRate)
{
    if (!m_mavsdk) {
        qWarning() << "QDataLinkPrivate: MAVSDK not initialized";
        return false;
    }
    
    QString connectionString = generateConnectionString(connectionType, address, portOrBaudRate);
    qDebug() << "QDataLinkPrivate: Attempting to connect to" << connectionString;
    
    // 检查是否已经连接
    auto itorFind = m_connectionHandles.find(connectionString);
    if (itorFind != m_connectionHandles.end()) {
        qDebug() << "QDataLinkPrivate: Already connected to" << connectionString;
        return true;
    }
    
    auto connectionResult = m_mavsdk->add_any_connection_with_handle(connectionString.toStdString());
    
    if (connectionResult.first == mavsdk::ConnectionResult::Success) {
        m_connectionHandles.insert(connectionString, connectionResult.second);
        qDebug() << "QDataLinkPrivate: Connection successful to" << connectionString;
        return true;
    } else {
        std::ostringstream oss;
        oss << connectionResult.first;
        QString errorMsg = QString("Connection failed: %1").arg(QString::fromStdString(oss.str()));
        qWarning() << "QDataLinkPrivate:" << errorMsg;
        return false;
    }
}

void QDataLinkPrivate::disconnect()
{
    if (!m_mavsdk) {
        return;
    }
    
    // 清理系统连接状态回调句柄
    for (auto& handle : m_systemConnectionHandles) {
        // 取消订阅连接状态回调
    }
    m_systemConnectionHandles.clear();
    
    // 取消订阅新系统发现
    if (m_mavsdk) {
        m_mavsdk->unsubscribe_on_new_system(m_newSystemHandle);
    }
    
    // 清理连接句柄
    m_connectionHandles.clear();
    
    // 清理飞控对象
    for (auto vehicle : m_vehicles) {
        if (vehicle) {
            vehicle->deleteLater();
        }
    }
    m_vehicles.clear();
    
    m_mavsdk.reset();
    m_isInitialized = false;
    
    qDebug() << "QDataLinkPrivate: Disconnected from all connections";
}

QVector<std::shared_ptr<mavsdk::System>> QDataLinkPrivate::getConnectedSystems() const
{
    if (!m_mavsdk) {
        return QVector<std::shared_ptr<mavsdk::System>>();
    }
    
    auto systems = m_mavsdk->systems();
    QVector<std::shared_ptr<mavsdk::System>> result;
    
    for (auto system : systems) {
        if (system->is_connected()) {
            result.append(system);
        }
    }
    
    return result;
}

QVector<uint8_t> QDataLinkPrivate::getConnectedSystemIds() const
{
    if (!m_mavsdk) {
        return QVector<uint8_t>();
    }
    
    auto systems = m_mavsdk->systems();
    QVector<uint8_t> result;
    
    for (auto system : systems) {
        if (system->is_connected()) {
            result.append(system->get_system_id());
        }
    }
    
    return result;
}

mavsdk::System* QDataLinkPrivate::getSystem(uint8_t systemId) const
{
    if (!m_mavsdk) {
        return nullptr;
    }
    
    auto systems = m_mavsdk->systems();
    for (auto system : systems) {
        if (system->get_system_id() == systemId && system->is_connected()) {
            return system.get();
        }
    }
    
    return nullptr;
}

bool QDataLinkPrivate::isConnected() const
{
    if (!m_mavsdk) {
        return false;
    }
    
    auto systems = m_mavsdk->systems();
    for (auto system : systems) {
        if (system->is_connected()) {
            return true;
        }
    }
    
    return false;
}

int QDataLinkPrivate::getSystemCount() const
{
    if (!m_mavsdk) {
        return 0;
    }
    
    int count = 0;
    auto systems = m_mavsdk->systems();
    for (auto system : systems) {
        if (system->is_connected()) {
            count++;
        }
    }
    
    return count;
}

uint8_t QDataLinkPrivate::getGroundStationSystemId() const
{
    return m_groundStationSystemId;
}

uint8_t QDataLinkPrivate::getGroundStationComponentId() const
{
    return m_groundStationComponentId;
}

QVector<QVehicle*> QDataLinkPrivate::getAllVehicles() const
{
    QVector<QVehicle*> result;
    for (auto vehicle : m_vehicles) {
        result.append(vehicle);
    }
    return result;
}

QVehicle* QDataLinkPrivate::getVehicle(uint8_t systemId) const
{
    return m_vehicles.value(systemId, nullptr);
}

QVector<uint8_t> QDataLinkPrivate::getVehicleIDs() const
{
    QVector<uint8_t> result;
    for (auto it = m_vehicles.begin(); it != m_vehicles.end(); ++it) {
        result.append(it.key());
    }
    return result;
}

QVehicle* QDataLinkPrivate::createOrUpdateVehicle(void* system, QObject* parent)
{
    if (!system) {
        return nullptr;
    }
    
    // 将void*转换为mavsdk::System*
    auto mavsdkSystem = static_cast<mavsdk::System*>(system);
    uint8_t systemId = mavsdkSystem->get_system_id();
    
    // 检查是否已存在飞控对象
    QVehicle* vehicle = m_vehicles.value(systemId, nullptr);
    
    if (!vehicle) {
        // 创建新的飞控对象
        vehicle = new QVehicle(systemId, parent);
        m_vehicles[systemId] = vehicle;
        qDebug() << "QDataLinkPrivate: Created new vehicle for system" << systemId;
    } else {
        qDebug() << "QDataLinkPrivate: Updated existing vehicle for system" << systemId;
    }
    
    // 更新飞控信息
    vehicle->updateFromSystem(system);
    
    // 发射飞控创建信号
    QMetaObject::invokeMethod(parent, "vehicleCreated", Qt::QueuedConnection,
                              Q_ARG(QVehicle*, vehicle));
    
    return vehicle;
}

void QDataLinkPrivate::setupConnectionErrorHandling(QObject* parent)
{
    if (!m_mavsdk || !parent) {
        return;
    }
    
    // 订阅连接错误
    m_mavsdk->subscribe_connection_errors([this, parent](mavsdk::Mavsdk::ConnectionError error) {
        std::ostringstream oss;
        oss << error.error_description;
        QString errorMsg = QString("Connection error: %1").arg(QString::fromStdString(oss.str()));
        qWarning() << "QDataLinkPrivate:" << errorMsg;
        
        // 移除有问题的连接
        m_mavsdk->remove_connection(error.connection_handle);
        
        // 发射错误信号
        QMetaObject::invokeMethod(parent, "connectionError", Qt::QueuedConnection,
                                  Q_ARG(QString, errorMsg));
    });
}

void QDataLinkPrivate::setupNewSystemDiscoveryCallback(QObject* parent)
{
    if (!m_mavsdk || !parent) {
        return;
    }
    
    // 订阅新系统发现
    m_newSystemHandle = m_mavsdk->subscribe_on_new_system([this, parent]() {
        qDebug() << "QDataLinkPrivate: New system discovered";
        
        // 获取所有系统
        auto systems = m_mavsdk->systems();
        
        // 检查是否有新系统
        for (auto system : systems) {
            uint8_t systemId = system->get_system_id();
            
            // 如果还没有为这个系统设置回调，则设置
            if (!m_systemConnectionHandles.contains(systemId)) {
                qDebug() << "QDataLinkPrivate: Setting up callbacks for new system" << systemId;
                
                auto handle = system->subscribe_is_connected([this, parent, systemId, system](bool isConnected) {
                    qDebug() << "QDataLinkPrivate: System" << systemId << (isConnected ? "connected" : "disconnected");
                    
                    // 发射系统连接状态变化信号
                    QMetaObject::invokeMethod(parent, "systemConnectionStatusChanged", Qt::QueuedConnection,
                                              Q_ARG(uint8_t, systemId), Q_ARG(bool, isConnected));
                    
                    if (isConnected) {
                        // 创建或更新飞控对象
                        createOrUpdateVehicle(system.get(), parent);
                    }
                });
                
                m_systemConnectionHandles[systemId] = handle;
            }
        }
    });
}

void QDataLinkPrivate::setupSystemConnectionCallbacks(QObject* parent)
{
    if (!m_mavsdk || !parent) {
        return;
    }
    
    // 为现有系统设置连接状态回调
    auto systems = m_mavsdk->systems();
    for (auto system : systems) {
        uint8_t systemId = system->get_system_id();
        
        // 如果还没有为这个系统设置回调，则设置
        if (!m_systemConnectionHandles.contains(systemId)) {
            auto handle = system->subscribe_is_connected([this, parent, systemId, system](bool isConnected) {
                qDebug() << "QDataLinkPrivate: System" << systemId << (isConnected ? "connected" : "disconnected");
                
                // 发射系统连接状态变化信号
                QMetaObject::invokeMethod(parent, "systemConnectionStatusChanged", Qt::QueuedConnection,
                                          Q_ARG(uint8_t, systemId), Q_ARG(bool, isConnected));
                
                if (isConnected) {
                    // 创建或更新飞控对象
                    createOrUpdateVehicle(system.get(), parent);
                }
            });
            
            m_systemConnectionHandles[systemId] = handle;
        }
    }
}

QString QDataLinkPrivate::generateConnectionString(int connectionType, const QString &address, int portOrBaudRate) const
{
    QString result;
    
    switch (connectionType) {
        case 0: // Serial
            result = QString("serial://%1:%2").arg(address).arg(portOrBaudRate);
            break;
            
        case 1: // TCP
            result = QString("tcp://%1:%2").arg(address).arg(portOrBaudRate);
            break;
            
        case 2: // UDP
            if (address.isEmpty() || address == "0.0.0.0") {
                // 如果地址为空或者是默认地址，使用端口号作为监听端口
                result = QString("udp://0.0.0.0:%1").arg(portOrBaudRate);
            } else {
                result = QString("udp://%1:%2").arg(address).arg(portOrBaudRate);
            }
            break;
    }
    
    return result;
}
