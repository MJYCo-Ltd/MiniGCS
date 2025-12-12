#ifndef QVEHICLE_H
#define QVEHICLE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <memory>

// 前向声明
class QVehiclePrivate;

/**
 * @brief QVehicle类 - 飞控载具类
 * 
 * 该类封装了飞控载具的所有信息，包括连接状态、硬件信息等
 */
class QVehicle : public QObject
{
    Q_OBJECT

public:
    explicit QVehicle(QObject *parent = nullptr);
    explicit QVehicle(uint8_t unID, QObject *parent = nullptr);
    ~QVehicle();

    /**
     * @brief 获取飞控唯一ID
     * @return 飞控唯一ID
     */
    uint8_t getUnID() const;

    /**
     * @brief 设置飞控唯一ID
     * @param unID 飞控唯一ID
     */
    void setUnID(uint8_t unID);

    /**
     * @brief 获取组件ID
     * @return 组件ID
     */
    uint8_t getComponentId() const;

    /**
     * @brief 设置组件ID
     * @param componentId 组件ID
     */
    void setComponentId(uint8_t componentId);

    /**
     * @brief 获取自动驾驶仪类型
     * @return 自动驾驶仪类型
     */
    QString getAutopilotType() const;

    /**
     * @brief 设置自动驾驶仪类型
     * @param autopilotType 自动驾驶仪类型
     */
    void setAutopilotType(const QString &autopilotType);

    /**
     * @brief 获取载具类型
     * @return 载具类型
     */
    QString getVehicleType() const;

    /**
     * @brief 设置载具类型
     * @param vehicleType 载具类型
     */
    void setVehicleType(const QString &vehicleType);

    /**
     * @brief 获取固件版本
     * @return 固件版本
     */
    QString getFirmwareVersion() const;

    /**
     * @brief 设置固件版本
     * @param firmwareVersion 固件版本
     */
    void setFirmwareVersion(const QString &firmwareVersion);

    /**
     * @brief 获取硬件版本
     * @return 硬件版本
     */
    QString getHardwareVersion() const;

    /**
     * @brief 设置硬件版本
     * @param hardwareVersion 硬件版本
     */
    void setHardwareVersion(const QString &hardwareVersion);

    /**
     * @brief 获取软件版本
     * @return 软件版本
     */
    QString getSoftwareVersion() const;

    /**
     * @brief 设置软件版本
     * @param softwareVersion 软件版本
     */
    void setSoftwareVersion(const QString &softwareVersion);

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief 设置连接状态
     * @param connected 连接状态
     */
    void setConnected(bool connected);

    /**
     * @brief 检查是否有相机
     * @return 是否有相机
     */
    bool hasCamera() const;

    /**
     * @brief 设置是否有相机
     * @param hasCamera 是否有相机
     */
    void setHasCamera(bool hasCamera);

    /**
     * @brief 获取组件ID列表
     * @return 组件ID列表
     */
    QVector<uint8_t> getComponentIds() const;

    /**
     * @brief 设置组件ID列表
     * @param componentIds 组件ID列表
     */
    void setComponentIds(const QVector<uint8_t> &componentIds);

    /**
     * @brief 获取最后连接时间
     * @return 最后连接时间
     */
    QDateTime getLastConnectedTime() const;

    /**
     * @brief 设置最后连接时间
     * @param time 最后连接时间
     */
    void setLastConnectedTime(const QDateTime &time);

    /**
     * @brief 获取最后断开时间
     * @return 最后断开时间
     */
    QDateTime getLastDisconnectedTime() const;

    /**
     * @brief 设置最后断开时间
     * @param time 最后断开时间
     */
    void setLastDisconnectedTime(const QDateTime &time);

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

signals:

    /**
     * @brief 连接状态变化信号
     * @param connected 是否连接
     */
    void connectionStatusChanged(bool connected);

    /**
     * @brief 信息更新信号
     */
    void infoUpdated();
public slots:
    void SendCommand();
protected:
    void timerEvent(QTimerEvent *event) override;
private:
    std::unique_ptr<QVehiclePrivate> d_ptr;    ///< 私有实现指针
};

#endif // QVEHICLE_H
