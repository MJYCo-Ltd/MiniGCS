#include <QMetaObject>
#include <QMetaMethod>
#include <QByteArray>
#include <QPointer>
#include <QThreadPool>

#include "Private/QGroundControlStationPrivate.h"
#include "Plat/Private/QAutopilotPrivate.h"
#include "Plat/QPlat.h"
#include "QGroundControlStation.h"
#include "Link/QDataLink.h"
#include "Link/QLinkManager.h"
#include "Extern/XmlToMavSDK.h"

#include "QGCSConfig.h"
#include "Private/QGCSLog.h"

QGroundControlStationPrivate::QGroundControlStationPrivate()
    : m_isInitialized(false)
{
}

QGroundControlStationPrivate::~QGroundControlStationPrivate()
{
    if (!m_mavsdk) {
        return;
    }

    m_rawDataLink = nullptr;
    unsubscribeRawBytesToBeSent();
    if (m_newSystemHandle.valid()) {
        m_mavsdk->unsubscribe_on_new_system(m_newSystemHandle);
    }
    if (m_connectionErrorHandle.valid()) {
        m_mavsdk->unsubscribe_connection_errors(m_connectionErrorHandle);
    }
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

    /// 解析 APM 扩展 XML（命令表 + 内容缓存）；注入 MAVSDK 等首个 System 出现时只做一次
    m_xmlExtension = std::make_shared<XmlToMavSDK>(
        QGCSConfig::instance()->mavMessageExtension());
    if (!m_xmlExtension->hasXmlContent()) {
        spdlog::warn(
            SYS_FMT_STR,
            "mav message extension",
            QGCSConfig::instance()->mavMessageExtension().toUtf8().constData());
    }

    /// 连接由 QLinkManager 通过 addTcpServer/addSerial 等添加
    m_isInitialized = true;
}

template<>struct fmt::formatter<mavsdk::MavlinkDirect::Result>:ostream_formatter{};

void QGroundControlStationPrivate::ensureCustomXmlLoaded(
    const std::shared_ptr<mavsdk::System> &system)
{
    if (!system || !m_xmlExtension) {
        return;
    }
    if (m_xmlExtension->isCustomXmlApplied()) {
        return;
    }
    if (!m_xmlExtension->hasXmlContent()) {
        return;
    }

    const auto extension = m_xmlExtension;
    const std::weak_ptr<mavsdk::System> weakSystem = system;
    QThreadPool::globalInstance()->start([extension, weakSystem]() {
        const auto currentSystem = weakSystem.lock();
        if (!currentSystem || !extension) {
            return;
        }

        mavsdk::MavlinkDirect mavlinkDirect(*currentSystem);
        const auto result = extension->applyCustomXmlOnce(mavlinkDirect);
        if (!result.has_value()) {
            return;
        }
        if (mavsdk::MavlinkDirect::Result::Success != *result) {
            spdlog::error(PLAT_FMT_STR, currentSystem->get_system_id(),
                          "load_custom_xml", *result);
        } else {
            spdlog::info(PLAT_FMT_STR, currentSystem->get_system_id(),
                         "load_custom_xml", "success");
        }
    });
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

    if (m_connectionErrorHandle.valid()) {
        m_mavsdk->unsubscribe_connection_errors(m_connectionErrorHandle);
    }
    
    // 订阅连接错误
    QPointer<QGroundControlStation> station =
        qobject_cast<QGroundControlStation *>(parent);
    m_connectionErrorHandle = m_mavsdk->subscribe_connection_errors(
        [station](mavsdk::Mavsdk::ConnectionError error) {
        if (!station) {
            return;
        }

        QMetaObject::invokeMethod(station, [station, error]() {
            if (!station || !station->d_ptr) {
                return;
            }
            station->d_ptr->handleConnectionError(error, station);
        }, Qt::QueuedConnection);
    });
}

void QGroundControlStationPrivate::handleConnectionError(
    const mavsdk::Mavsdk::ConnectionError &error,
    QGroundControlStation *station)
{
    if (!m_mavsdk || !station) {
        return;
    }

    const QString description =
        QString::fromStdString(error.error_description);
    QString connectionString;
    for (auto it = m_connectionHandles.begin();
         it != m_connectionHandles.end(); ++it) {
        if (it->second == error.connection_handle) {
            connectionString = QString::fromStdString(it->first);
            break;
        }
    }

    if (!connectionString.isEmpty()) {
        removeConnection(connectionString);
    }
    if (!connectionString.isEmpty() && station->linkManager()) {
        station->linkManager()->handleConnectionError(connectionString,
                                                      description);
    }
    emit station->mavConnectionError(description);
}

void QGroundControlStationPrivate::setupNewSystemDiscoveryCallback(
    QObject *parent) {
    if (!m_mavsdk || !parent) {
        return;
    }

    if (m_newSystemHandle.valid()) {
        m_mavsdk->unsubscribe_on_new_system(m_newSystemHandle);
    }

    QPointer<QGroundControlStation> station =
        qobject_cast<QGroundControlStation *>(parent);
    std::weak_ptr<mavsdk::Mavsdk> weakMavsdk = m_mavsdk;
    QGroundControlStationPrivate *self = this;

    // 订阅新系统发现
    m_newSystemHandle = m_mavsdk->subscribe_on_new_system([station, weakMavsdk, self]() {
        if (!station) {
            return;
        }
        QMetaObject::invokeMethod(station, [station, weakMavsdk, self]() {
            const auto mavsdk = weakMavsdk.lock();
            if (!station || !mavsdk || !self) {
                return;
            }
            // 获取所有系统
            auto systems = mavsdk->systems();

            /// 扩展方言只需注入一次（任意已连接 System 即可）
            for (const auto &system : systems) {
                if (system && system->is_connected()) {
                    self->ensureCustomXmlLoaded(system);
                    break;
                }
            }

            // 检查是否有新系统
            for (auto system : systems) {
                /// system 断开也会触发subscribe_on_new_system
                if (system->is_connected()) {
                    uint8_t systemId = system->get_system_id();
                    bool bHaveAutopilot = system->has_autopilot();
                    if (station) {
                        QPlat *pPlat =
                            station->getOrCreatePlat(systemId, bHaveAutopilot);
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
                            emit station->newPlatFind(pPlat);
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
    QPointer<QObject> context(parent);
    m_rawBytesHandle = m_mavsdk->subscribe_raw_bytes_to_be_sent(
        [callback, context](const char *bytes, size_t length) {
            if (bytes && length > 0) {
                QByteArray data(bytes, static_cast<int>(length));
                if (!context) {
                    return;
                }
                // 通过 QMetaObject::invokeMethod 确保在主线程中执行 callback
                QMetaObject::invokeMethod(context, [callback, context, data]() {
                    if (context && callback) {
                        callback(data);
                    }
                });
            }
        });
}

void QGroundControlStationPrivate::unsubscribeRawBytesToBeSent() {
    if (m_mavsdk && m_rawBytesHandle.valid()) {
        m_mavsdk->unsubscribe_raw_bytes_to_be_sent(m_rawBytesHandle);
        m_rawBytesHandle = {};
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
    const QPointer<QDataLink> link = m_rawDataLink;

    unsubscribeRawBytesToBeSent();
    m_rawBytesHandle = m_mavsdk->subscribe_raw_bytes_to_be_sent(
        [link](const char *bytes, size_t length) {
            if (bytes && length > 0 && link) {
                QByteArray data(bytes, static_cast<int>(length));
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
        const mavsdk::Mavsdk::ConnectionHandle handle = it->second;
        m_connectionHandles.erase(it);
        if (handle.valid()) {
            m_mavsdk->remove_connection(handle);
        }
    }
}
