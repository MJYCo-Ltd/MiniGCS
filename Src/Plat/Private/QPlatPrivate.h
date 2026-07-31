#ifndef QPLATPRIVATE_H
#define QPLATPRIVATE_H

#include <QString>
#include <atomic>
#include <memory>
#include <mutex>
#include <mavsdk/system.h>
#include <mavsdk/plugins/mavlink_direct/mavlink_direct.h>
#include <mavsdk/plugins/info/info.h>

// 前向声明
class QPlat;

class XmlToMavSDK;

/**
 * @brief QStandalone的私有实现类
 * 
 * 该类封装了QStandalone的所有MAVSDK相关实现细节，
 * 使用PIMPL模式隐藏实现细节
 */
class QPlatPrivate
{
public:
    QPlatPrivate(QPlat* pPlat);
    virtual ~QPlatPrivate();

    /**
     * @brief 获取固件版本
     * @return 固件版本
     */
    QString getFirmwareVersion() const
    {
        const auto state = m_infoState;
        std::scoped_lock lock(state->mutex);
        return state->firmwareVersion;
    }

    /**
     * @brief 获取软件版本
     * @return 软件版本
     */
    QString getSoftwareVersion() const
    {
        const auto state = m_infoState;
        std::scoped_lock lock(state->mutex);
        return state->softwareVersion;
    }

    /**
     * @brief 转换为字符串表示
     * @return 字符串表示
     */
    QString toString() const;

    /**
     * @brief 设置系统对象
     * @param system 系统对象
     */
    virtual void setSystem(std::shared_ptr<mavsdk::System> system);

    /**
     * @brief 获取系统对象
     * @return 系统对象
     */
    std::shared_ptr<mavsdk::System> getSystem() const;

    /**
     * @brief 设置整站共享的扩展命令表（MAV_CMD 目录 / 可选自定义 MessageSet 注入源）
     */
    void setMavMessageExtension(const std::shared_ptr<XmlToMavSDK> &extension)
    {
        m_xmlExtension = extension;
    }

    std::shared_ptr<XmlToMavSDK> mavMessageExtension() const
    {
        return m_xmlExtension;
    }

    /**
     * @brief 设置消息处理回调
     * @param parent QVehicle实例指针，用于信号发射
     */
    virtual void setupMessageHandling();

private:
    /**
     * @brief 更新版本信息（通过 Info 插件）
     */
    void updateVersionInfo();

protected:
    struct InfoState {
        mutable std::mutex mutex;
        QString firmwareVersion{"Unknown"};
        QString softwareVersion{"Unknown"};
        std::atomic_bool active{true};
        std::atomic_bool updateRunning{false};
    };

    QPlat*  q_ptr;
    std::shared_ptr<InfoState> m_infoState;
    
    // MAVSDK相关
    std::shared_ptr<mavsdk::System> m_pSystem; ///< 系统对象
    std::shared_ptr<mavsdk::Info> m_pInfo;     ///< 信息插件
    std::shared_ptr<mavsdk::MavlinkDirect> m_pMavlinkDirect;
    std::shared_ptr<XmlToMavSDK> m_xmlExtension; ///< 整站一份，按名发扩展命令

    mavsdk::System::IsConnectedHandle m_hConntecd;
    mavsdk::System::ComponentDiscoveredHandle m_hCommonpentDiscovered;
    mavsdk::MavlinkDirect::MessageHandle m_statusTextHandle;
};

#endif // QPLATPRIVATE_H
