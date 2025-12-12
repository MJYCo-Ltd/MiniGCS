#include "Private/QGroundControlStationPrivate.h"
#include "QVehicle.h"
#include <QDebug>
#include <QMetaObject>
#include <QMetaMethod>
#include <QByteArray>
#include <sstream>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/info/info.h>

QGroundControlStationPrivate::QGroundControlStationPrivate()
    : m_isInitialized(false)
    , m_groundStationSystemId(246)      // 默认地面站系统ID
    , m_groundStationComponentId(191)   // 默认地面站组件ID
{
}

QGroundControlStationPrivate::~QGroundControlStationPrivate()
{
    unsubscribeRawBytesToBeSent();
}

void QGroundControlStationPrivate::initializeMavsdk()
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

    auto connectionResult = m_mavsdk->add_any_connection_with_handle("raw://");

    if (connectionResult.first == mavsdk::ConnectionResult::Success) {
        m_connectionHandle = connectionResult.second;
        qDebug() << "QGroundControlStationPrivate: Connection successful";
    } else {
        std::ostringstream oss;
        oss << connectionResult.first;
        QString errorMsg = QString("Connection failed: %1").arg(QString::fromStdString(oss.str()));
        qWarning() << "QGroundControlStationPrivate:" << errorMsg;
    }
    
    m_isInitialized = true;
    qDebug() << "QGroundControlStationPrivate: MAVSDK initialized with system ID:" 
             << m_groundStationSystemId << "component ID:" << m_groundStationComponentId;
}

QVector<std::shared_ptr<mavsdk::System>> QGroundControlStationPrivate::getConnectedSystems() const
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

QVector<uint8_t> QGroundControlStationPrivate::getConnectedSystemIds() const
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

mavsdk::System* QGroundControlStationPrivate::getSystem(uint8_t systemId) const
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

int QGroundControlStationPrivate::getSystemCount() const
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

uint8_t QGroundControlStationPrivate::getGroundStationSystemId() const
{
    return m_groundStationSystemId;
}

uint8_t QGroundControlStationPrivate::getGroundStationComponentId() const
{
    return m_groundStationComponentId;
}

QVector<QVehicle*> QGroundControlStationPrivate::getAllVehicles() const
{
    QVector<QVehicle*> result;
    for (auto vehicle : m_vehicles) {
        result.append(vehicle);
    }
    return result;
}

QVehicle* QGroundControlStationPrivate::getVehicle(uint8_t systemId) const
{
    return m_vehicles.value(systemId, nullptr);
}

QVector<uint8_t> QGroundControlStationPrivate::getVehicleIDs() const
{
    QVector<uint8_t> result;
    for (auto it = m_vehicles.begin(); it != m_vehicles.end(); ++it) {
        result.append(it.key());
    }
    return result;
}

QVehicle* QGroundControlStationPrivate::createOrUpdateVehicle(void* system, QObject* parent)
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
        mavsdkSystem->subscribe_is_connected([vehicle](bool isConnected) {
            // MAVSDK 回调可能在非主线程中执行，需要通过队列连接确保在主线程中处理
            QMetaObject::invokeMethod(vehicle, "connectionStatusChanged", Qt::QueuedConnection,
                                      Q_ARG(bool, isConnected));
        });
        m_vehicles[systemId] = vehicle;
        qDebug() << "QGroundControlStationPrivate: Created new vehicle for system" << systemId;
    } else {
        qDebug() << "QGroundControlStationPrivate: Updated existing vehicle for system" << systemId;
    }

    vehicle->updateFromSystem(system);
    
    // 发射飞控创建信号
    QMetaObject::invokeMethod(parent, "newVehicleFind", Qt::QueuedConnection,
                              Q_ARG(QVehicle*, vehicle));
    
    return vehicle;
}

void QGroundControlStationPrivate::setupConnectionErrorHandling(QObject* parent)
{
    if (!m_mavsdk || !parent) {
        return;
    }
    
    // 订阅连接错误
    m_mavsdk->subscribe_connection_errors([this, parent](mavsdk::Mavsdk::ConnectionError error) {
        std::ostringstream oss;
        oss << error.error_description;
        QString errorMsg = QString("Connection error: %1").arg(QString::fromStdString(oss.str()));
        qWarning() << "QGroundControlStationPrivate:" << errorMsg;
        
        // 移除有问题的连接
        m_mavsdk->remove_connection(error.connection_handle);
        if (m_connectionHandle == error.connection_handle) {
            m_connectionHandle = mavsdk::Handle<>();
        }

        // 发射错误信号
        QMetaObject::invokeMethod(parent, "connectionError", Qt::QueuedConnection,
                                  Q_ARG(QString, errorMsg));
    });
}

void QGroundControlStationPrivate::setupNewSystemDiscoveryCallback(QObject* parent)
{
    if (!m_mavsdk || !parent) {
        return;
    }
    
    // 订阅新系统发现
    m_newSystemHandle = m_mavsdk->subscribe_on_new_system([this, parent]() {
        // MAVSDK 回调可能在非主线程中执行，需要通过队列连接确保在主线程中处理
        QMetaObject::invokeMethod(parent, [this, parent]() {
            // 获取所有系统
            auto systems = m_mavsdk->systems();

            // 检查是否有新系统
            for (auto system : systems) {
                uint8_t systemId = system->get_system_id();
                
                // 如果还没有为这个系统设置回调，则设置
                if (!m_vehicles.contains(systemId)) {
                    QMetaObject::invokeMethod(parent, [this, parent, system]() {
                        createOrUpdateVehicle(system.get(), parent);
                    });
                }
            }
        });
    });
}

void QGroundControlStationPrivate::processReceivedRawData(const QByteArray &data)
{
    if (!m_mavsdk) {
        return;
    }
    
    // 将接收到的数据传递给MAVSDK处理
    m_mavsdk->pass_received_raw_bytes(data.constData(), data.size());
}

void QGroundControlStationPrivate::setupRawBytesToBeSentCallback(std::function<void(const QByteArray&)> callback, QObject* parent)
{
    if (!m_mavsdk) {
        qWarning() << "QGroundControlStationPrivate: MAVSDK not initialized, cannot setup RawBytes callback";
        return;
    }

    if (!parent) {
        qWarning() << "QGroundControlStationPrivate: Parent object is null, cannot setup RawBytes callback safely";
        return;
    }

    // 取消之前的订阅（如果存在）
    unsubscribeRawBytesToBeSent();

    // 订阅需要发送的原始字节
    // MAVSDK 回调可能在非主线程中执行，需要通过 QMetaObject::invokeMethod 确保在主线程中执行
    m_rawBytesHandle = m_mavsdk->subscribe_raw_bytes_to_be_sent(
        [callback, parent](const char* bytes, size_t length) {
            if (bytes && length > 0) {
                QByteArray data(bytes, static_cast<int>(length));
                // 通过 QMetaObject::invokeMethod 确保在主线程中执行 callback
                QMetaObject::invokeMethod(parent, [callback, data]() {
                    if (callback) {
                        callback(data);
                    }
                }, Qt::QueuedConnection);
            }
        }
    );

    qDebug() << "QGroundControlStationPrivate: RawBytes callback setup successfully";
}

void QGroundControlStationPrivate::unsubscribeRawBytesToBeSent()
{
    if (m_mavsdk && m_rawBytesHandle.valid()) {
        m_mavsdk->unsubscribe_raw_bytes_to_be_sent(m_rawBytesHandle);
        qDebug() << "QGroundControlStationPrivate: RawBytes callback unsubscribed";
    }
}

