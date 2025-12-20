#ifndef QPLATPRIVATE_H
#define QPLATPRIVATE_H

#include <QString>
#include <memory>
#include <mavsdk/system.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mavlink_direct/mavlink_direct.h>
#include <mavsdk/plugins/info/info.h>

// 前向声明
class QPlat;

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
    ~QPlatPrivate(){}

    /**
     * @brief 获取固件版本
     * @return 固件版本
     */
    QString getFirmwareVersion() const { return m_firmwareVersion; }

    /**
     * @brief 获取软件版本
     * @return 软件版本
     */
    QString getSoftwareVersion() const { return m_softwareVersion; }

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
    QPlat*  q_ptr;
    QString m_firmwareVersion;              ///< 固件版本
    QString m_softwareVersion;              ///< 软件版本
    
    // MAVSDK相关
    std::shared_ptr<mavsdk::System> m_pSystem; ///< 系统对象
    std::unique_ptr<mavsdk::Info> m_pInfo;     ///< 信息插件

    std::unique_ptr<mavsdk::MavlinkDirect> m_pMavlinkDirect;

    mavsdk::System::IsConnectedHandle m_hConntecd;
    mavsdk::System::ComponentDiscoveredHandle m_hCommonpentDiscovered;
};

#endif // QPLATPRIVATE_H
