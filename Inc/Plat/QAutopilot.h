#ifndef _YTY_QAUTOPILOT_H
#define _YTY_QAUTOPILOT_H

#include "QPlat.h"

class QAutopilotPrivate;

/**
 * @brief QAutopilot类 - 具备自动驾驶功能的系统
 * 
 * 该类封装了自动驾驶功能的所有信息，包括连接状态、硬件信息等
 */
class QAutopilot : public QPlat
{
    Q_OBJECT

public:
    explicit QAutopilot(QObject *parent = nullptr);
    ~QAutopilot();

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

protected:
    /**
     * @brief 获取QAutopilotPrivate指针的辅助方法
     * @return QAutopilotPrivate指针
     */
    QAutopilotPrivate* d_func();
    const QAutopilotPrivate* d_func() const;
};

#endif // _YTY_QAUTOPILOT_H
