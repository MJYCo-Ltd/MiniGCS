#include <QMetaObject>
#include <QMetaMethod>
#include <QByteArray>

#include "Private/QGroundControlStationPrivate.h"
#include "Plat/Private/QAutopilotPrivate.h"
#include "Plat/QPlat.h"
#include "QGroundControlStation.h"
#include "Link/QDataLink.h"

#include "QGCSConfig.h"
#include "QGCSLog.h"

QGroundControlStationPrivate::QGroundControlStationPrivate()
    : m_isInitialized(false)
{
}

QGroundControlStationPrivate::~QGroundControlStationPrivate()
{
    m_mavsdk->unsubscribe_on_new_system(m_newSystemHandle);
    unsubscribeRawBytesToBeSent();
}

template<>struct fmt::formatter<mavsdk::ConnectionResult>:ostream_formatter{};

void QGroundControlStationPrivate::initializeMavsdk()
{
    if (m_isInitialized) {
        return;
    }
    
    /// 创建MAVSDK实例，配置为地面站模式
    mavsdk::Mavsdk::Configuration config(
        mavsdk::ComponentType::GroundStation);
    config.set_system_id(QGCSConfig::instance()->gcsSystemId());
    config.set_component_id(QGCSConfig::instance()->gcsComponentId());
    m_mavsdk = std::make_shared<mavsdk::Mavsdk>(config);

    m_mavsdk->subscribe_incoming_messages_json(
        [](mavsdk::Mavsdk::MavlinkMessage msg) {
            QGCSConfig::instance()->dealMavsdkMessage(msg.system_id,
                                                      msg.fields_json);
            return (true);
        });

    /// 连接由 QLinkManager 通过 addTcpServer/addSerial 等添加
    m_isInitialized = true;
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

void QGroundControlStationPrivate::setupConnectionErrorHandling(QObject* parent)
{
    if (!m_mavsdk || !parent) {
        return;
    }
    
    // 订阅连接错误
    m_mavsdk->subscribe_connection_errors([this, parent](mavsdk::Mavsdk::ConnectionError error) {
        spdlog::critical(MAV_FMT_STR, "Connection error",
                         error.error_description);

        // 从映射中移除
        for (auto it = m_connectionHandles.begin(); it != m_connectionHandles.end(); ++it) {
            if (it->second == error.connection_handle) {
                m_connectionHandles.erase(it);
                break;
            }
        }
        m_mavsdk->remove_connection(error.connection_handle);

        QMetaObject::invokeMethod(parent, "mavConnectionError", Qt::QueuedConnection,
                                  Q_ARG(QString, QString::fromStdString(error.error_description)));
    });
}

void QGroundControlStationPrivate::setupNewSystemDiscoveryCallback(
    QObject *parent) {
    if (!m_mavsdk || !parent) {
        return;
    }

    // 订阅新系统发现
    m_newSystemHandle = m_mavsdk->subscribe_on_new_system([this, parent]() {
        QMetaObject::invokeMethod(parent, [this, parent]() {
            // 获取所有系统
            auto systems = m_mavsdk->systems();

            // 检查是否有新系统
            for (auto system : systems) {
                /// system 断开也会触发subscribe_on_new_system
                if (system->is_connected()) {
                    uint8_t systemId = system->get_system_id();
                    bool bHaveAutopilot = system->has_autopilot();
                    QGroundControlStation *pQGCS =
                        qobject_cast<QGroundControlStation *>(parent);
                    if (nullptr != pQGCS) {
                        QPlat *pPlat = pQGCS->getOrCreatePlat(systemId, bHaveAutopilot);
                        /// 如果平台的Private 指针没有设置 或者 Private的
                        /// system与现在的不一致
                        if (nullptr == pPlat->d_ptr.get() ||
                            pPlat->d_ptr.get()->getSystem() != system) {
                            if (bHaveAutopilot) {
                                QPlatPrivate *localQPlatPrivate = new QAutopilotPrivate(pPlat);
                                localQPlatPrivate->setSystem(system);
                                pPlat->SetPrivate(localQPlatPrivate);
                            } else {
                                QPlatPrivate *localQPlatPrivate = new QPlatPrivate(pPlat);
                                localQPlatPrivate->setSystem(system);
                                pPlat->SetPrivate(localQPlatPrivate);
                            }
                            /// 发送信号给qt
                            QMetaObject::invokeMethod(parent, "newPlatFind",
                                                      Q_ARG(QPlat *, pPlat));
                        }
                    }
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
    
    m_mavsdk->pass_received_raw_bytes(data.constData(), data.size());
}

void QGroundControlStationPrivate::setupRawBytesToBeSentCallback(
    std::function<void(const QByteArray &)> callback, QObject *parent) {
    if (!m_mavsdk || !parent) {
        return;
    }

    /// 取消之前的订阅（如果存在）
    unsubscribeRawBytesToBeSent();

    // 订阅需要发送的原始字节
    // MAVSDK 回调可能在非主线程中执行，需要通过 QMetaObject::invokeMethod
    // 确保在主线程中执行
    m_rawBytesHandle = m_mavsdk->subscribe_raw_bytes_to_be_sent(
        [callback, parent](const char *bytes, size_t length) {
            if (bytes && length > 0) {
                QByteArray data(bytes, static_cast<int>(length));
                // 通过 QMetaObject::invokeMethod 确保在主线程中执行 callback
                QMetaObject::invokeMethod(parent, [callback, data]() {
                    if (callback) {
                        callback(data);
                    }
                });
            }
        });
}

void QGroundControlStationPrivate::unsubscribeRawBytesToBeSent() {
    if (m_mavsdk && m_rawBytesHandle.valid()) {
        m_mavsdk->unsubscribe_raw_bytes_to_be_sent(m_rawBytesHandle);
    }
}

int QGroundControlStationPrivate::getMaxChannel()
{
    return(MAVLINK_COMM_NUM_BUFFERS);
}

bool QGroundControlStationPrivate::addConnection(const QString &connectionUrl)
{
    if (!m_mavsdk || connectionUrl.isEmpty()) {
        return false;
    }
    std::string url = connectionUrl.toStdString();
    auto result = m_mavsdk->add_any_connection_with_handle(url);
    if (result.first == mavsdk::ConnectionResult::Success) {
        m_connectionHandles[url] = result.second;
        return true;
    }
    spdlog::warn(MAV_FMT_STR, "addConnection failed", result.first);
    return false;
}

bool QGroundControlStationPrivate::addRawConnection(QDataLink *rawDataLink)
{
    if (!m_mavsdk || !rawDataLink || m_rawDataLink) return false;
    auto result = m_mavsdk->add_any_connection_with_handle("raw://");
    if (result.first != mavsdk::ConnectionResult::Success) {
        spdlog::warn(MAV_FMT_STR, "addRawConnection failed", result.first);
        return false;
    }
    m_connectionHandles["raw://"] = result.second;
    m_rawDataLink = rawDataLink;

    unsubscribeRawBytesToBeSent();
    m_rawBytesHandle = m_mavsdk->subscribe_raw_bytes_to_be_sent(
        [this](const char *bytes, size_t length) {
            if (bytes && length > 0 && m_rawDataLink) {
                QByteArray data(bytes, static_cast<int>(length));
                QDataLink *link = m_rawDataLink;
                QMetaObject::invokeMethod(link, "emitRawDataReceived",
                    Qt::QueuedConnection, Q_ARG(QByteArray, data));
            }
        });
    return true;
}

void QGroundControlStationPrivate::removeConnection(const QString &connectionUrl)
{
    if (!m_mavsdk || connectionUrl.isEmpty()) return;
    std::string url = connectionUrl.toStdString();
    if (url == "raw://") {
        m_rawDataLink = nullptr;
        unsubscribeRawBytesToBeSent();
    }
    auto it = m_connectionHandles.find(url);
    if (it != m_connectionHandles.end()) {
        if (it->second.valid()) {
            m_mavsdk->remove_connection(it->second);
        }
        m_connectionHandles.erase(it);
    }
}
