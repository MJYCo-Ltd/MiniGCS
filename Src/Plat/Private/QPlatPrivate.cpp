#include <QDebug>
#include <QPointer>
#include <QThreadPool>
#include <sstream>
#include <utility>
#include <QJsonDocument>
#include <QJsonObject>

#include "Private/QGCSLog.h"
#include "Private/QMavsdkTextCatalog.h"
#include "QGCSConfig.h"
#include "Extern/XmlToMavSDK.h"
#include "Plat/Private/QPlatPrivate.h"
#include "Plat/QPlat.h"

QPlatPrivate::QPlatPrivate(QPlat *pPlat)
    : q_ptr(pPlat), m_infoState(std::make_shared<InfoState>()) {
}

QPlatPrivate::~QPlatPrivate()
{
    m_infoState->active = false;
    if (m_pMavlinkDirect && m_statusTextHandle.valid()) {
        m_pMavlinkDirect->unsubscribe_message(m_statusTextHandle);
        m_statusTextHandle = {};
    }
    if (m_pSystem) {
        if (m_hConntecd.valid()) {
            m_pSystem->unsubscribe_is_connected(m_hConntecd);
            m_hConntecd = {};
        }
        if (m_hCommonpentDiscovered.valid()) {
            m_pSystem->unsubscribe_component_discovered(m_hCommonpentDiscovered);
            m_hCommonpentDiscovered = {};
        }
    }
}

QString QPlatPrivate::toString() const {
    if (!m_pSystem) {
        return QStringLiteral("QPlatInfo: uninitialized");
    }

    std::ostringstream oss;
    const QString firmwareVersion = getFirmwareVersion();
    const QString softwareVersion = getSoftwareVersion();

    oss << "QPlatInfo:\r\n"
        << "systemId=" << m_pSystem->get_system_id()
        << "\r\nfirmwareVersion=" << firmwareVersion.toStdString()
        << "\r\nsoftwareVersion=" << softwareVersion.toStdString()
        << "\r\nhasCamera=" << (m_pSystem->has_camera() ? "true" : "false")
        << "\r\nhasGimbal=" << (m_pSystem->has_gimbal() ? "true" : "false")
        << "\r\ncomponentIds=[";

    for (auto one : m_pSystem->component_ids()) {
        oss << one << ',';
    }
    oss << "]";

    return QString::fromStdString(oss.str());
}

void QPlatPrivate::setSystem(std::shared_ptr<mavsdk::System> system) {
    m_infoState->active = false;
    m_infoState = std::make_shared<InfoState>();

    /// 如果原来的system 不为空，取消订阅
    if (nullptr != m_pSystem) {
        if (m_pMavlinkDirect && m_statusTextHandle.valid()) {
            m_pMavlinkDirect->unsubscribe_message(m_statusTextHandle);
            m_statusTextHandle = {};
        }
        if (m_hConntecd.valid()) {
            m_pSystem->unsubscribe_is_connected(m_hConntecd);
            m_hConntecd = {};
        }
        if (m_hCommonpentDiscovered.valid()) {
            m_pSystem->unsubscribe_component_discovered(m_hCommonpentDiscovered);
            m_hCommonpentDiscovered = {};
        }
    }
    m_pSystem = system;
    if (!m_pSystem) {
        m_pInfo.reset();
        m_pMavlinkDirect.reset();
        return;
    }

    /// 根据配置文件配置是否开启时间同步
    if(QGCSConfig::instance()->timeSyncEnabled()){
        m_pSystem->enable_timesync();
    }
    // 创建插件实例
    m_pInfo = std::make_shared<mavsdk::Info>(*system);
    m_pMavlinkDirect = std::make_shared<mavsdk::MavlinkDirect>(*system);

    const QPointer<QPlat> plat(q_ptr);
    const uint32_t systemId = m_pSystem->get_system_id();
    m_statusTextHandle = m_pMavlinkDirect->subscribe_message(
        "STATUSTEXT", [plat, systemId](
                          mavsdk::MavlinkDirect::MavlinkMessage msg) {
            if (!plat) {
                return;
            }
            std::string fieldsJson = std::move(msg.fields_json);
            QMetaObject::invokeMethod(
                plat,
                [plat, systemId, fieldsJson = std::move(fieldsJson)]() {
                    if (!plat) {
                        return;
                    }
                    QJsonParseError error;
                    const QJsonDocument document = QJsonDocument::fromJson(
                        QByteArray::fromStdString(fieldsJson), &error);
                    if (error.error != QJsonParseError::NoError ||
                        !document.isObject()) {
                        return;
                    }
                    const QJsonObject object = document.object();
                    const int severity = object.value("severity").toInt();
                    const QString text = object.value("text").toString();
                    QGCSConfig::instance()->dealMavsdkStatusText(
                        systemId, severity, text);
                    /// MAV_SEVERITY_EMERGENCY..WARNING
                    if (severity <= 4) {
                        emit plat->errorInfo(text);
                    }
                },
                Qt::QueuedConnection);
        });

    // 通过 Info 插件获取版本信息
    // 扩展 XML 由 QGroundControlStationPrivate 整站注入一次，此处不再每机加载
    updateVersionInfo();
}

std::shared_ptr<mavsdk::System> QPlatPrivate::getSystem() const {
    return m_pSystem;
}

void QPlatPrivate::setupMessageHandling() {
    if (!m_pSystem) {
        return;
    }

    // 订阅系统连接状态变化
    const QPointer<QPlat> plat(q_ptr);
    m_hConntecd = m_pSystem->subscribe_is_connected([plat](bool isConnected) {
        // 发射连接状态变化信号
        if (plat) {
            QMetaObject::invokeMethod(plat, "connectionStatusChanged",
                                      Qt::QueuedConnection,
                                      Q_ARG(bool, isConnected));
        }
    });

    // 订阅组件发现
    m_hCommonpentDiscovered = m_pSystem->subscribe_component_discovered(
        [plat](mavsdk::ComponentType componentType) {
            qDebug() << "QVehiclePrivate: Component discovered:"
                     << static_cast<int>(componentType);
            if (plat) {
                QMetaObject::invokeMethod(
                    plat, [plat]() {
                        if (plat) {
                            emit plat->componentsChanged();
                        }
                    }, Qt::QueuedConnection);
            }
        });
}

template<>struct fmt::formatter<mavsdk::Info::Result>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Info::Version>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Info::Product>:ostream_formatter{};

void QPlatPrivate::updateVersionInfo() {
    if (!m_pInfo || !m_pSystem) {
        return;
    }
    
    const auto state = m_infoState;
    bool expected = false;
    if (!state->updateRunning.compare_exchange_strong(expected, true)) {
        return;
    }
    const auto info = m_pInfo;
    const auto currentSystem = m_pSystem;
    const QPointer<QPlat> plat(q_ptr);

    QThreadPool::globalInstance()->start([state, info, currentSystem, plat]() {
        bool bUpdate=false;
        QString softwareVersion = "Unknown";
        QString firmwareVersion = "Unknown";

        // 获取版本信息
        auto version_result = info->get_version();
        if (version_result.first == mavsdk::Info::Result::Success) {
            auto version = version_result.second;

            spdlog::info(PLAT_FMT_STR, currentSystem->get_system_id(),
                         "version", version);
            
            softwareVersion =
                QString("Flight SW: v%1.%2.%3 (Vendor v%4.%5.%6, git %7, %8)"
                        "; OS SW: v%9.%10.%11 (git %12)")
                    .arg(version.flight_sw_major)
                    .arg(version.flight_sw_minor)
                    .arg(version.flight_sw_patch)
                    .arg(version.flight_sw_vendor_major)
                    .arg(version.flight_sw_vendor_minor)
                    .arg(version.flight_sw_vendor_patch)
                    .arg(QString::fromStdString(version.flight_sw_git_hash))
                    .arg(QMavsdkTextCatalog::text(
                        QStringLiteral("flightSoftwareVersionType"),
                        static_cast<int>(version.flight_sw_version_type)))
                    .arg(version.os_sw_major)
                    .arg(version.os_sw_minor)
                    .arg(version.os_sw_patch)
                    .arg(QString::fromStdString(version.os_sw_git_hash));
            bUpdate = true;
        } else {
            spdlog::warn(PLAT_FMT_STR, currentSystem->get_system_id(),
                        "get_version", version_result.first);
        }

        // 获取产品信息
        auto product_result = info->get_product();
        if (product_result.first == mavsdk::Info::Result::Success) {
            auto product = product_result.second;
            
            spdlog::info(PLAT_FMT_STR, currentSystem->get_system_id(),
                         "product", product);
            
            // 构建产品信息字符串
            QStringList productParts;
            if (!product.vendor_name.empty()) {
                productParts << QString("Vendor: %1").arg(QString::fromStdString(product.vendor_name));
            }
            if (product.vendor_id != 0) {
                productParts << QString("VendorID: %1").arg(product.vendor_id);
            }
            if (!product.product_name.empty()) {
                productParts << QString("Product: %1").arg(QString::fromStdString(product.product_name));
            }
            if (product.product_id != 0) {
                productParts << QString("ProductID: %1").arg(product.product_id);
            }
            
            if (!productParts.isEmpty()) {
                firmwareVersion = productParts.join(", ");
            }
            bUpdate = true;
        } else {
            spdlog::warn(PLAT_FMT_STR, currentSystem->get_system_id(),
                        "get_product", product_result.first);
        }

        if (state->active) {
            std::scoped_lock infoLock(state->mutex);
            state->softwareVersion = std::move(softwareVersion);
            state->firmwareVersion = std::move(firmwareVersion);
        }
        state->updateRunning = false;

        if (bUpdate && state->active && plat) {
            QMetaObject::invokeMethod(plat, "infoUpdated", Qt::QueuedConnection);
        }
    });
}
