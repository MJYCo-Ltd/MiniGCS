#ifndef QPLATPRIVATE_H
#define QPLATPRIVATE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <memory>
#include <mavsdk/system.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
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
    QPlatPrivate();
    ~QPlatPrivate();

    /**
     * @brief 设置固件版本
     * @param firmwareVersion 固件版本
     */
    void setFirmwareVersion(const QString &firmwareVersion);

    /**
     * @brief 获取固件版本
     * @return 固件版本
     */
    QString getFirmwareVersion() const;

    /**
     * @brief 设置硬件版本
     * @param hardwareVersion 硬件版本
     */
    void setHardwareVersion(const QString &hardwareVersion);

    /**
     * @brief 获取硬件版本
     * @return 硬件版本
     */
    QString getHardwareVersion() const;

    /**
     * @brief 设置软件版本
     * @param softwareVersion 软件版本
     */
    void setSoftwareVersion(const QString &softwareVersion);

    /**
     * @brief 获取软件版本
     * @return 软件版本
     */
    QString getSoftwareVersion() const;

    /**
     * @brief 检查是否有相机
     * @return 是否有相机
     */
    bool hasCamera() const;

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
    virtual void setupMessageHandling(QObject* parent);
protected:
    QString m_firmwareVersion;              ///< 固件版本
    QString m_hardwareVersion;              ///< 硬件版本
    QString m_softwareVersion;              ///< 软件版本
    
    // MAVSDK相关
    std::shared_ptr<mavsdk::System> m_system; ///< 系统对象
    std::unique_ptr<mavsdk::Info> m_info;     ///< 信息插件
    std::unique_ptr<mavsdk::MavlinkPassthrough> mavlinkPassthrough;
};

#endif // QPLATPRIVATE_H
