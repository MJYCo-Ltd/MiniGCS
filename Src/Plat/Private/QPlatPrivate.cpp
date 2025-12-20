#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <sstream>
#include <thread>

#include "QGCSLog.h"
#include "QGCSConfig.h"
#include "Plat/Private/QPlatPrivate.h"
#include "Plat/QPlat.h"

QPlatPrivate::QPlatPrivate(QPlat *pPlat)
    : q_ptr(pPlat), m_firmwareVersion("Unknown"), m_softwareVersion("Unknown") {
}

QString QPlatPrivate::toString() const {
    std::ostringstream oss;

    oss << "QPlatInfo:\r\n"
        << "systemId=" << m_pSystem->get_system_id()
        << "\r\nfirmwareVersion=" << m_firmwareVersion.toStdString()
        << "\r\nsoftwareVersion=" << m_softwareVersion.toStdString()
        << "\r\nhasCamera=" << (m_pSystem->has_camera() ? "true" : "false")
        << "\r\nhas" << m_pSystem->has_gimbal() << "\r\ncomponentIds=[";

    for (auto one : m_pSystem->component_ids()) {
        oss << one << ',';
    }
    oss << "]";

    return QString::fromStdString(oss.str());
}

template<>struct fmt::formatter<mavsdk::MavlinkDirect::Result>:ostream_formatter{};
void QPlatPrivate::setSystem(std::shared_ptr<mavsdk::System> system) {

    /// 如果原来的system 不为空，取消订阅
    if (nullptr != m_pSystem) {
        m_pSystem->unsubscribe_is_connected(m_hConntecd);
        m_pSystem->unsubscribe_component_discovered(m_hCommonpentDiscovered);
    }
    m_pSystem = system;

    /// 根据配置文件配置是否开启时间同步
    if(QGCSConfig::instance()->timeSyncEnabled()){
        m_pSystem->enable_timesync();
    }
    // 创建插件实例
    m_pInfo = std::make_unique<mavsdk::Info>(*system);
    m_pMavlinkDirect = std::make_unique<mavsdk::MavlinkDirect>(*system);
    
    // 通过 Info 插件获取版本信息
    updateVersionInfo();

    QFile file(QGCSConfig::instance()->mavMessageExtension());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::string fileInfo = file.readAll().toStdString();
        // 分离线程，不阻塞主线程
        std::thread([this, fileInfo]() {
            auto result = m_pMavlinkDirect->load_custom_xml(fileInfo);
            if (mavsdk::MavlinkDirect::Result::Success != result) {
                spdlog::error(PLAT_FMT_STR, m_pSystem->get_system_id(),
                              "load_custom_xml", result);
            }
        }).detach();
    } else {
        spdlog::error(SYS_FMT_STR, "打开文件失败", file.fileName().toUtf8().data());
    }
}

std::shared_ptr<mavsdk::System> QPlatPrivate::getSystem() const {
    return m_pSystem;
}

void QPlatPrivate::setupMessageHandling() {
    if (!m_pSystem) {
        return;
    }

    // 订阅系统连接状态变化
    m_hConntecd = m_pSystem->subscribe_is_connected([this](bool isConnected) {
        // 发射连接状态变化信号
        QMetaObject::invokeMethod(q_ptr, "connectionStatusChanged",
                                  Qt::QueuedConnection, Q_ARG(bool, isConnected));
        
        // 连接时更新版本信息
        if (isConnected) {
            updateVersionInfo();
        }
    });

    // 订阅组件发现
    m_hCommonpentDiscovered = m_pSystem->subscribe_component_discovered(
        [this](mavsdk::ComponentType componentType) {
            // 这里可以处理新组件发现
            qDebug() << "QVehiclePrivate: Component discovered:"
                     << static_cast<int>(componentType);
        });
}

template<>struct fmt::formatter<mavsdk::Info::Result>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Info::Version>:ostream_formatter{};
template<>struct fmt::formatter<mavsdk::Info::Product>:ostream_formatter{};

void QPlatPrivate::updateVersionInfo() {
    if (!m_pInfo || !m_pSystem) {
        return;
    }
    
    // 异步获取版本信息，避免阻塞
    std::thread([this]() {
        bool bUpdate=false;
        // 获取版本信息
        auto version_result = m_pInfo->get_version();
        if (version_result.first == mavsdk::Info::Result::Success) {
            auto version = version_result.second;

            spdlog::info(PLAT_FMT_STR, m_pSystem->get_system_id(),
                         "version", version);
            
            auto typeToString = [version](mavsdk::Info::Version::FlightSoftwareVersionType t) {
                switch (t) {
                case mavsdk::Info::Version::FlightSoftwareVersionType::Alpha:   return "Alpha";
                case mavsdk::Info::Version::FlightSoftwareVersionType::Beta:    return "Beta";
                case mavsdk::Info::Version::FlightSoftwareVersionType::Release: return "Release";
                case mavsdk::Info::Version::FlightSoftwareVersionType::Dev:  return "Dev";
                case mavsdk::Info::Version::FlightSoftwareVersionType::Rc:  return "Rc";
                default: return "Unknown";
                }
            };

            m_softwareVersion = QString("Flight SW: v%1.%2.%3 (Vendor v%4.%5.%6, git %7, %8)"
                                        "OS SW: v%9.%10.%11 (git %12)")
                                    .arg(version.flight_sw_major)
                                    .arg(version.flight_sw_minor)
                                    .arg(version.flight_sw_patch)
                                    .arg(version.flight_sw_vendor_major)
                                    .arg(version.flight_sw_vendor_minor)
                                    .arg(version.flight_sw_vendor_patch)
                                    .arg(QString::fromStdString(version.flight_sw_git_hash))
                                    .arg(typeToString(version.flight_sw_version_type))
                                    .arg(version.os_sw_major)
                                    .arg(version.os_sw_minor)
                                    .arg(version.os_sw_patch)
                                    .arg(QString::fromStdString(version.os_sw_git_hash));
            bUpdate = true;
        } else {
            spdlog::warn(PLAT_FMT_STR, m_pSystem->get_system_id(),
                        "get_version", version_result.first);
            m_softwareVersion = "Unknown";
        }

        // 获取产品信息
        auto product_result = m_pInfo->get_product();
        if (product_result.first == mavsdk::Info::Result::Success) {
            auto product = product_result.second;
            
            spdlog::info(PLAT_FMT_STR, m_pSystem->get_system_id(),
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
                m_firmwareVersion = productParts.join(", ");
            } else {
                m_firmwareVersion = "Unknown";
            }
            bUpdate = true;
        } else {
            spdlog::warn(PLAT_FMT_STR, m_pSystem->get_system_id(),
                        "get_product", product_result.first);
            m_firmwareVersion = "Unknown";
        }

        if(bUpdate){
            QMetaObject::invokeMethod(q_ptr,"infoUpdated",Qt::QueuedConnection);
        }
    }).detach();
}
