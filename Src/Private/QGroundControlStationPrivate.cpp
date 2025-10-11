#include "Private/QGroundControlStationPrivate.h"
#include "QGroundControlStation.h"
#include <QDebug>
#include <QObject>
#include <QMetaObject>
#include <QMetaMethod>
#include <QDateTime>
#include <sstream>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/info/info.h>

QGroundControlStationPrivate::QGroundControlStationPrivate()
    : m_isInitialized(false)
    , m_groundStationSystemId(246)      // 默认地面站系统ID
    , m_groundStationComponentId(191)   // 默认地面站组件ID
{
    // 初始化连接配置
    m_pConnectionConfig = std::make_unique<ConnectionConfig>();
}

QGroundControlStationPrivate::~QGroundControlStationPrivate()
{
    disconnect();
}

void QGroundControlStationPrivate::initializeMavsdk()
{
    if (m_isInitialized) {
        return;
    }
    
    // 创建MAVSDK实例，配置为地面站模式
    mavsdk::Mavsdk::Configuration config(
        mavsdk::ComponentType::GroundStation);
    config.set_system_id(m_groundStationSystemId);       // 使用可配置的 system ID
    config.set_component_id(m_groundStationComponentId); // 使用可配置的 component ID
    m_mavsdk = std::make_shared<mavsdk::Mavsdk>(config);
    
    m_isInitialized = true;
    qDebug() << "QGroundControlStationPrivate: MAVSDK initialized with system ID:" 
             << m_groundStationSystemId << "component ID:" << m_groundStationComponentId;
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
        qWarning() << errorMsg;
        
        // 移除有问题的连接
        m_mavsdk->remove_connection(error.connection_handle);
        
        // 发射错误信号
        QMetaObject::invokeMethod(parent, "connectionError", Qt::QueuedConnection,
                                  Q_ARG(QString, errorMsg));
    });
}

bool QGroundControlStationPrivate::connectToDataLink(int connectionType, const QString &address, int portOrBaudRate)
{
    if (!m_mavsdk) {
        qWarning() << "QGroundControlStationPrivate: MAVSDK not initialized";
        return false;
    }
    
    QString connectionString = generateConnectionString(connectionType, address, portOrBaudRate);
    qDebug() << "QGroundControlStationPrivate: Attempting to connect to" << connectionString;
    
    auto connectionResult = m_mavsdk->add_any_connection_with_handle(connectionString.toStdString());
    
    if (connectionResult.first == mavsdk::ConnectionResult::Success) {
        qDebug() << "QGroundControlStationPrivate: Connection successful";
        return true;
    } else {
        std::ostringstream oss;
        oss << connectionResult.first;
        QString errorMsg = QString("Connection failed: %1").arg(QString::fromStdString(oss.str()));
        qWarning() << errorMsg;
        return false;
    }
}

void QGroundControlStationPrivate::disconnect()
{
    if (!m_mavsdk) {
        return;
    }
    
    // 清理连接状态回调句柄
    for (auto& handle : m_connectionHandles) {
        // 取消订阅连接状态回调
    }
    m_connectionHandles.clear();
    
    // 清理系统连接信息
    m_systemConnectionInfo.clear();
    m_systemConnectionTypes.clear();
    m_reconnectAttempts.clear();
    
    m_mavsdk.reset();
    m_isInitialized = false;
    
    qDebug() << "QGroundControlStationPrivate: Disconnected from all vehicles";
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

bool QGroundControlStationPrivate::isConnected() const
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

void QGroundControlStationPrivate::setupSystemConnectionCallbacks(QObject* parent)
{
    if (!m_mavsdk || !parent) {
        return;
    }
    
    // 为每个系统设置连接状态回调
    auto systems = m_mavsdk->systems();
    for (auto system : systems) {
        uint8_t systemId = system->get_system_id();
        
        // 如果还没有为这个系统设置回调，则设置
        if (!m_connectionHandles.contains(systemId)) {
            auto handle = system->subscribe_is_connected([this, parent, systemId](bool isConnected) {
                if (isConnected) {
                    qDebug() << "QGroundControlStationPrivate: System" << systemId << "connected";
                    // 发射系统连接信号
                    QMetaObject::invokeMethod(parent, "vehicleConnected", Qt::QueuedConnection,
                                              Q_ARG(uint8_t, systemId));
                } else {
                    qDebug() << "QGroundControlStationPrivate: System" << systemId << "disconnected";
                    // 发射系统断开信号
                    QMetaObject::invokeMethod(parent, "vehicleDisconnected", Qt::QueuedConnection,
                                              Q_ARG(uint8_t, systemId), 
                                              Q_ARG(QString, "System disconnected"));
                    
                    // 清理连接信息
                    m_systemConnectionInfo.remove(systemId);
                    m_systemConnectionTypes.remove(systemId);
                    m_reconnectAttempts.remove(systemId);
                }
            });
            
            m_connectionHandles[systemId] = handle;
        }
    }
}

QString QGroundControlStationPrivate::generateConnectionString(int connectionType, const QString &address, int portOrBaudRate) const
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

void QGroundControlStationPrivate::setConnectionConfig(const ConnectionConfig &config)
{
    *m_pConnectionConfig = config;
    qDebug() << "QGroundControlStationPrivate: Connection config updated"
             << " auto reconnect:" << config.autoReconnect;
}

ConnectionConfig QGroundControlStationPrivate::getConnectionConfig() const
{
    return *m_pConnectionConfig;
}


void QGroundControlStationPrivate::setAutoReconnect(bool autoReconnect)
{
    m_pConnectionConfig->autoReconnect = autoReconnect;
    qDebug() << "QGroundControlStationPrivate: Auto reconnect set to" << autoReconnect;
}

bool QGroundControlStationPrivate::reconnectSystem(uint8_t systemId, QObject* parent)
{
    if (!m_pConnectionConfig->autoReconnect) {
        qDebug() << "QGroundControlStationPrivate: Auto reconnect disabled for system" << systemId;
        return false;
    }
    
    QString connectionString = getSystemConnectionInfo(systemId);
    if (connectionString.isEmpty()) {
        qWarning() << "QGroundControlStationPrivate: No connection info found for system" << systemId;
        return false;
    }
    
    int attemptCount = m_reconnectAttempts.value(systemId, 0) + 1;
    m_reconnectAttempts[systemId] = attemptCount;
    
    // 检查最大重连次数
    if (m_pConnectionConfig->maxReconnectAttempts > 0 && 
        attemptCount > m_pConnectionConfig->maxReconnectAttempts) {
        qWarning() << "QGroundControlStationPrivate: Max reconnect attempts reached for system" << systemId;
        QMetaObject::invokeMethod(parent, "reconnectFailed", Qt::QueuedConnection,
                                  Q_ARG(uint8_t, systemId), 
                                  Q_ARG(QString, "Max reconnect attempts reached"));
        return false;
    }
    
    qDebug() << "QGroundControlStationPrivate: Attempting to reconnect system" << systemId 
             << "attempt" << attemptCount;
    
    // 发射重连尝试信号
    QMetaObject::invokeMethod(parent, "reconnectAttempted", Qt::QueuedConnection,
                              Q_ARG(uint8_t, systemId), Q_ARG(int, attemptCount));
    
    // 尝试重连
    auto connectionResult = m_mavsdk->add_any_connection_with_handle(connectionString.toStdString());
    
    if (connectionResult.first == mavsdk::ConnectionResult::Success) {
        qDebug() << "QGroundControlStationPrivate: System" << systemId << "reconnected successfully";
        m_reconnectAttempts.remove(systemId); // 清除重连计数
        QMetaObject::invokeMethod(parent, "reconnected", Qt::QueuedConnection,
                                  Q_ARG(uint8_t, systemId));
    } else {
        qWarning() << "QGroundControlStationPrivate: Failed to reconnect system" << systemId;
        QMetaObject::invokeMethod(parent, "reconnectFailed", Qt::QueuedConnection,
                                  Q_ARG(uint8_t, systemId), 
                                  Q_ARG(QString, "Connection failed"));
    }
    
    return true;
}


void QGroundControlStationPrivate::setSystemConnectionInfo(uint8_t systemId, int connectionType, const QString &connectionString)
{
    m_systemConnectionInfo[systemId] = connectionString;
    m_systemConnectionTypes[systemId] = connectionType;
    qDebug() << "QGroundControlStationPrivate: Connection info set for system" << systemId 
             << "type:" << connectionType << "string:" << connectionString;
}

QString QGroundControlStationPrivate::getSystemConnectionInfo(uint8_t systemId) const
{
    return m_systemConnectionInfo.value(systemId);
}

uint8_t QGroundControlStationPrivate::getGroundStationSystemId() const
{
    return m_groundStationSystemId;
}

uint8_t QGroundControlStationPrivate::getGroundStationComponentId() const
{
    return m_groundStationComponentId;
}
