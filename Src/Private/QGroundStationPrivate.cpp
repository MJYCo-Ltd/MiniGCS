#include "Private/QGroundStationPrivate.h"
#include "QGroundStation.h"
#include <QDebug>
#include <QObject>
#include <QMetaObject>
#include <QMetaMethod>
#include <QDateTime>
#include <sstream>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/info/info.h>

QGroundStationPrivate::QGroundStationPrivate()
    : m_statusTimer(nullptr)
    , m_isInitialized(false)
{
    // 初始化连接配置
    m_pConnectionConfig = std::make_unique<ConnectionConfig>();
}

QGroundStationPrivate::~QGroundStationPrivate()
{
    disconnect();
}

void QGroundStationPrivate::initializeMavsdk()
{
    if (m_isInitialized) {
        return;
    }
    
    // 创建MAVSDK实例，配置为地面站模式
    m_mavsdk = std::make_shared<mavsdk::Mavsdk>(
        mavsdk::Mavsdk::Configuration(mavsdk::ComponentType::GroundStation)
    );
    
    m_isInitialized = true;
    qDebug() << "QGroundStationPrivate: MAVSDK initialized";
}

void QGroundStationPrivate::setupConnectionErrorHandling(QObject* parent)
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

bool QGroundStationPrivate::connectToVehicle(const QString &connectionString)
{
    if (!m_mavsdk) {
        qWarning() << "QGroundStationPrivate: MAVSDK not initialized";
        return false;
    }
    
    qDebug() << "QGroundStationPrivate: Attempting to connect to" << connectionString;
    
    auto connectionResult = m_mavsdk->add_any_connection_with_handle(connectionString.toStdString());
    
    if (connectionResult.first == mavsdk::ConnectionResult::Success) {
        qDebug() << "QGroundStationPrivate: Connection successful";
        return true;
    } else {
        std::ostringstream oss;
        oss << connectionResult.first;
        QString errorMsg = QString("Connection failed: %1").arg(QString::fromStdString(oss.str()));
        qWarning() << errorMsg;
        return false;
    }
}

void QGroundStationPrivate::disconnect()
{
    if (!m_mavsdk) {
        return;
    }
    
    // 清理心跳相关数据
    m_lastHeartbeatTimes.clear();
    m_heartbeatStatus.clear();
    m_systemConnectionInfo.clear();
    m_systemConnectionTypes.clear();
    m_reconnectAttempts.clear();
    
    m_connectedSystems.clear();
    m_mavsdk.reset();
    m_isInitialized = false;
    
    qDebug() << "QGroundStationPrivate: Disconnected from all vehicles";
}

QVector<std::shared_ptr<mavsdk::System>> QGroundStationPrivate::getConnectedSystems() const
{
    if (!m_mavsdk) {
        return QVector<std::shared_ptr<mavsdk::System>>();
    }
    
    auto systems = m_mavsdk->systems();
    QVector<std::shared_ptr<mavsdk::System>> result;
    
    for (auto system : systems) {
        result.append(system);
    }
    
    return result;
}

mavsdk::System* QGroundStationPrivate::getSystem(uint8_t systemId) const
{
    if (!m_mavsdk) {
        return nullptr;
    }
    
    auto systems = m_mavsdk->systems();
    for (auto system : systems) {
        if (system->get_system_id() == systemId) {
            return system.get();
        }
    }
    
    return nullptr;
}

bool QGroundStationPrivate::isConnected() const
{
    if (!m_mavsdk) {
        return false;
    }
    
    return !m_mavsdk->systems().empty();
}

int QGroundStationPrivate::getSystemCount() const
{
    if (!m_mavsdk) {
        return 0;
    }
    
    return static_cast<int>(m_mavsdk->systems().size());
}

void QGroundStationPrivate::setStatusTimer(QTimer* timer)
{
    m_statusTimer = timer;
}

QTimer* QGroundStationPrivate::getStatusTimer() const
{
    return m_statusTimer;
}

QVector<std::shared_ptr<mavsdk::System>>& QGroundStationPrivate::getConnectedSystemsList()
{
    return m_connectedSystems;
}

void QGroundStationPrivate::checkSystemStatus(QObject* parent)
{
    if (!m_mavsdk || !parent) {
        return;
    }
    
    auto currentSystems = m_mavsdk->systems();
    
    // 检查新连接的系统
    for (auto system : currentSystems) {
        uint8_t systemId = system->get_system_id();
        bool found = false;
        for (auto existingSystem : m_connectedSystems) {
            if (existingSystem->get_system_id() == systemId) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            m_connectedSystems.append(system);
            
            // 初始化心跳状态
            updateHeartbeatTime(systemId);
            m_heartbeatStatus[systemId] = true;
            
            qDebug() << "QGroundStationPrivate: New system connected, ID:" << systemId;
        }
    }
    
    // 检查断开连接的系统
    QVector<std::shared_ptr<mavsdk::System>> stillConnected;
    for (auto existingSystem : m_connectedSystems) {
        uint8_t systemId = existingSystem->get_system_id();
        bool found = false;
        for (auto currentSystem : currentSystems) {
            if (currentSystem->get_system_id() == systemId) {
                found = true;
                stillConnected.append(existingSystem);
                break;
            }
        }
        
        if (!found) {
            // 发射通信链路断开信号
            QMetaObject::invokeMethod(parent, "communicationLost", Qt::QueuedConnection,
                                      Q_ARG(uint8_t, systemId), 
                                      Q_ARG(QString, "System disconnected"));
            qDebug() << "QGroundStationPrivate: System disconnected, ID:" << systemId;
            
            // 清理心跳相关数据
            m_lastHeartbeatTimes.remove(systemId);
            m_heartbeatStatus.remove(systemId);
        }
    }
    
    m_connectedSystems = stillConnected;
    
    // 检查心跳状态
    checkHeartbeatStatus(parent);
}

QString QGroundStationPrivate::generateConnectionString(int connectionType, const QString &connectionString) const
{
    QString result;
    
    switch (connectionType) {
        case 0: // Serial
            if (connectionString.startsWith("serial://")) {
                result = connectionString;
            } else {
                // 假设输入格式为 "COM11:9600" 或 "COM11"
                QStringList parts = connectionString.split(':');
                if (parts.size() >= 1) {
                    QString port = parts[0];
                    QString baudRate = parts.size() > 1 ? parts[1] : "57600";
                    result = QString("serial://%1:%2").arg(port, baudRate);
                } else {
                    result = QString("serial://%1:57600").arg(connectionString);
                }
            }
            break;
            
        case 1: // TCP
            if (connectionString.startsWith("tcp://")) {
                result = connectionString;
            } else {
                // 假设输入格式为 "192.168.1.100:14550" 或 "localhost:14550"
                QStringList parts = connectionString.split(':');
                if (parts.size() >= 2) {
                    QString host = parts[0];
                    QString port = parts[1];
                    result = QString("tcp://%1:%2").arg(host, port);
                } else {
                    result = QString("tcp://%1:14550").arg(connectionString);
                }
            }
            break;
            
        case 2: // UDP
            if (connectionString.startsWith("udp://")) {
                result = connectionString;
            } else {
                // 假设输入格式为 "0.0.0.0:14550" 或 "14550"
                QStringList parts = connectionString.split(':');
                if (parts.size() >= 2) {
                    QString host = parts[0];
                    QString port = parts[1];
                    result = QString("udp://%1:%2").arg(host, port);
                } else {
                    result = QString("udp://0.0.0.0:%1").arg(connectionString);
                }
            }
            break;
    }
    
    return result;
}

void QGroundStationPrivate::setConnectionConfig(const ConnectionConfig &config)
{
    *m_pConnectionConfig = config;
    qDebug() << "QGroundStationPrivate: Connection config updated - heartbeat timeout:" 
             << config.heartbeatTimeoutMs << "ms, auto reconnect:" << config.autoReconnect;
}

ConnectionConfig QGroundStationPrivate::getConnectionConfig() const
{
    return *m_pConnectionConfig;
}

void QGroundStationPrivate::setHeartbeatTimeout(int timeoutMs)
{
    m_pConnectionConfig->heartbeatTimeoutMs = timeoutMs;
    qDebug() << "QGroundStationPrivate: Heartbeat timeout set to" << timeoutMs << "ms";
}

void QGroundStationPrivate::setAutoReconnect(bool autoReconnect)
{
    m_pConnectionConfig->autoReconnect = autoReconnect;
    qDebug() << "QGroundStationPrivate: Auto reconnect set to" << autoReconnect;
}

bool QGroundStationPrivate::reconnectSystem(uint8_t systemId, QObject* parent)
{
    if (!m_pConnectionConfig->autoReconnect) {
        qDebug() << "QGroundStationPrivate: Auto reconnect disabled for system" << systemId;
        return false;
    }
    
    QString connectionString = getSystemConnectionInfo(systemId);
    if (connectionString.isEmpty()) {
        qWarning() << "QGroundStationPrivate: No connection info found for system" << systemId;
        return false;
    }
    
    int attemptCount = m_reconnectAttempts.value(systemId, 0) + 1;
    m_reconnectAttempts[systemId] = attemptCount;
    
    // 检查最大重连次数
    if (m_pConnectionConfig->maxReconnectAttempts > 0 && 
        attemptCount > m_pConnectionConfig->maxReconnectAttempts) {
        qWarning() << "QGroundStationPrivate: Max reconnect attempts reached for system" << systemId;
        QMetaObject::invokeMethod(parent, "reconnectFailed", Qt::QueuedConnection,
                                  Q_ARG(uint8_t, systemId), 
                                  Q_ARG(QString, "Max reconnect attempts reached"));
        return false;
    }
    
    qDebug() << "QGroundStationPrivate: Attempting to reconnect system" << systemId 
             << "attempt" << attemptCount;
    
    // 发射重连尝试信号
    QMetaObject::invokeMethod(parent, "reconnectAttempted", Qt::QueuedConnection,
                              Q_ARG(uint8_t, systemId), Q_ARG(int, attemptCount));
    
    // 尝试重连
    bool success = connectToVehicle(connectionString);
    
    if (success) {
        qDebug() << "QGroundStationPrivate: System" << systemId << "reconnected successfully";
        m_reconnectAttempts.remove(systemId); // 清除重连计数
        QMetaObject::invokeMethod(parent, "reconnected", Qt::QueuedConnection,
                                  Q_ARG(uint8_t, systemId));
    } else {
        qWarning() << "QGroundStationPrivate: Failed to reconnect system" << systemId;
        QMetaObject::invokeMethod(parent, "reconnectFailed", Qt::QueuedConnection,
                                  Q_ARG(uint8_t, systemId), 
                                  Q_ARG(QString, "Connection failed"));
    }
    
    return success;
}

void QGroundStationPrivate::checkHeartbeatStatus(QObject* parent)
{
    if (!m_mavsdk || !parent) {
        return;
    }
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    auto systems = m_mavsdk->systems();
    for (auto system : systems) {
        uint8_t systemId = system->get_system_id();
        
        // 检查心跳超时
        qint64 lastHeartbeat = m_lastHeartbeatTimes.value(systemId, 0);
        bool wasActive = m_heartbeatStatus.value(systemId, false);
        
        if (lastHeartbeat > 0) {
            qint64 timeSinceLastHeartbeat = currentTime - lastHeartbeat;
            bool isActive = timeSinceLastHeartbeat < m_pConnectionConfig->heartbeatTimeoutMs;
            
            // 心跳状态变化
            if (wasActive != isActive) {
                m_heartbeatStatus[systemId] = isActive;
                QMetaObject::invokeMethod(parent, "heartbeatStatusChanged", Qt::QueuedConnection,
                                          Q_ARG(uint8_t, systemId), Q_ARG(bool, isActive));
                
                if (!isActive) {
                    qWarning() << "QGroundStationPrivate: Heartbeat timeout for system" << systemId
                               << "last heartbeat:" << timeSinceLastHeartbeat << "ms ago";
                    
                    // 发射飞控掉线信号
                    QMetaObject::invokeMethod(parent, "vehicleDisconnected", Qt::QueuedConnection,
                                              Q_ARG(uint8_t, systemId), 
                                              Q_ARG(QString, "Heartbeat timeout"));
                    
                    // 尝试自动重连
                    if (m_pConnectionConfig->autoReconnect) {
                        QTimer::singleShot(m_pConnectionConfig->reconnectIntervalMs, [this, systemId, parent]() {
                            reconnectSystem(systemId, parent);
                        });
                    }
                }
            }
        }
    }
}

void QGroundStationPrivate::updateHeartbeatTime(uint8_t systemId)
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    m_lastHeartbeatTimes[systemId] = currentTime;
    
    // 如果之前心跳不活跃，现在更新为活跃
    if (!m_heartbeatStatus.value(systemId, false)) {
        m_heartbeatStatus[systemId] = true;
        qDebug() << "QGroundStationPrivate: Heartbeat restored for system" << systemId;
    }
}

qint64 QGroundStationPrivate::getLastHeartbeatTime(uint8_t systemId) const
{
    return m_lastHeartbeatTimes.value(systemId, 0);
}

bool QGroundStationPrivate::isHeartbeatActive(uint8_t systemId) const
{
    return m_heartbeatStatus.value(systemId, false);
}

void QGroundStationPrivate::setSystemConnectionInfo(uint8_t systemId, int connectionType, const QString &connectionString)
{
    m_systemConnectionInfo[systemId] = connectionString;
    m_systemConnectionTypes[systemId] = connectionType;
    qDebug() << "QGroundStationPrivate: Connection info set for system" << systemId 
             << "type:" << connectionType << "string:" << connectionString;
}

QString QGroundStationPrivate::getSystemConnectionInfo(uint8_t systemId) const
{
    return m_systemConnectionInfo.value(systemId, QString());
}
