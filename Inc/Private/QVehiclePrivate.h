#ifndef QVEHICLEPRIVATE_H
#define QVEHICLEPRIVATE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <memory>
#include <mavsdk/system.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mavlink_direct/mavlink_direct.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>
#include <mavsdk/plugins/info/info.h>

// 前向声明
class QVehicle;

/**
 * @brief QVehicle的私有实现类
 * 
 * 该类封装了QVehicle的所有MAVSDK相关实现细节，
 * 使用PIMPL模式隐藏实现细节
 */
class QVehiclePrivate
{
public:
    QVehiclePrivate();
    ~QVehiclePrivate();

    /**
     * @brief 设置飞控唯一ID
     * @param unID 飞控唯一ID
     */
    void setUnID(uint8_t unID);

    /**
     * @brief 获取飞控唯一ID
     * @return 飞控唯一ID
     */
    uint8_t getUnID() const;

    /**
     * @brief 设置组件ID
     * @param componentId 组件ID
     */
    void setComponentId(uint8_t componentId);

    /**
     * @brief 获取组件ID
     * @return 组件ID
     */
    uint8_t getComponentId() const;

    /**
     * @brief 设置自动驾驶仪类型
     * @param autopilotType 自动驾驶仪类型
     */
    void setAutopilotType(const QString &autopilotType);

    /**
     * @brief 获取自动驾驶仪类型
     * @return 自动驾驶仪类型
     */
    QString getAutopilotType() const;

    /**
     * @brief 设置载具类型
     * @param vehicleType 载具类型
     */
    void setVehicleType(const QString &vehicleType);

    /**
     * @brief 获取载具类型
     * @return 载具类型
     */
    QString getVehicleType() const;

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
     * @brief 设置连接状态
     * @param connected 连接状态
     */
    void setConnected(bool connected);

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief 设置是否有相机
     * @param hasCamera 是否有相机
     */
    void setHasCamera(bool hasCamera);

    /**
     * @brief 检查是否有相机
     * @return 是否有相机
     */
    bool hasCamera() const;

    /**
     * @brief 设置组件ID列表
     * @param componentIds 组件ID列表
     */
    void setComponentIds(const QVector<uint8_t> &componentIds);

    /**
     * @brief 获取组件ID列表
     * @return 组件ID列表
     */
    QVector<uint8_t> getComponentIds() const;

    /**
     * @brief 设置最后连接时间
     * @param time 最后连接时间
     */
    void setLastConnectedTime(const QDateTime &time);

    /**
     * @brief 获取最后连接时间
     * @return 最后连接时间
     */
    QDateTime getLastConnectedTime() const;

    /**
     * @brief 设置最后断开时间
     * @param time 最后断开时间
     */
    void setLastDisconnectedTime(const QDateTime &time);

    /**
     * @brief 获取最后断开时间
     * @return 最后断开时间
     */
    QDateTime getLastDisconnectedTime() const;

    /**
     * @brief 更新飞控信息
     * @param system mavsdk系统指针
     */
    void updateFromSystem(void* system);

    /**
     * @brief 转换为字符串表示
     * @return 字符串表示
     */
    QString toString() const;

    /**
     * @brief 设置系统对象
     * @param system 系统对象
     */
    void setSystem(mavsdk::System *system);

    /**
     * @brief 获取系统对象
     * @return 系统对象
     */
    std::shared_ptr<mavsdk::System> getSystem() const;

    /**
     * @brief 设置消息处理回调
     * @param parent QVehicle实例指针，用于信号发射
     */
    void setupMessageHandling(QObject* parent);

    /**
     * @brief 设置遥测数据订阅
     * @param parent QVehicle实例指针，用于信号发射
     */
    void setupTelemetrySubscriptions(QObject* parent);

    /**
     * @brief 设置系统状态订阅
     * @param parent QVehicle实例指针，用于信号发射
     */
    void setupSystemStatusSubscriptions(QObject* parent);

    void sendCommand();
private:
    uint8_t m_unID;                         ///< 飞控唯一ID
    uint8_t m_componentId;                  ///< 组件ID
    QString m_autopilotType;                ///< 自动驾驶仪类型
    QString m_vehicleType;                  ///< 载具类型
    QString m_firmwareVersion;              ///< 固件版本
    QString m_hardwareVersion;              ///< 硬件版本
    QString m_softwareVersion;              ///< 软件版本
    bool m_isConnected;                     ///< 是否已连接
    bool m_hasCamera;                       ///< 是否有相机
    QVector<uint8_t> m_componentIds;        ///< 组件ID列表
    QDateTime m_lastConnectedTime;          ///< 最后连接时间
    QDateTime m_lastDisconnectedTime;       ///< 最后断开时间
    
    // MAVSDK相关
    std::shared_ptr<mavsdk::System> m_system; ///< 系统对象
    std::unique_ptr<mavsdk::Telemetry> m_telemetry; ///< 遥测插件
    std::unique_ptr<mavsdk::Info> m_info;     ///< 信息插件
    std::unique_ptr<mavsdk::MavlinkDirect> mavlinkDirect;
};

#endif // QVEHICLEPRIVATE_H
