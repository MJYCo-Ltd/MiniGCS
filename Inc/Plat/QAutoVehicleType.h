#ifndef _YTY_QAUTOVEHICLETYPE_H
#define _YTY_QAUTOVEHICLETYPE_H

#include <QObject>
#include <QMetaType>
#include <QString>

/**
 * @brief 自动驾驶仪载具类型枚举
 * 
 * 定义各种载具类型，可在 QML 中使用
 */
class QAutoVehicleType
{
    Q_GADGET

public:
    enum Vehicle{
        Vehicle_Unknown,                    ///< 未知载具
        Generic,                    ///< 通用微型飞行器
        FixedWing,                  ///< 固定翼飞机
        Quadrotor,                  ///< 四旋翼
        Coaxial,                    ///< 共轴直升机
        Helicopter,                 ///< 带尾桨的常规直升机
        Airship,                    ///< 飞艇（可控）
        FreeBalloon,                ///< 自由气球（不可控）
        Rocket,                     ///< 火箭
        GroundRover,                ///< 地面漫游车
        SurfaceBoat,                ///< 水面船只
        Submarine,                  ///< 潜艇
        Hexarotor,                  ///< 六旋翼
        Octorotor,                  ///< 八旋翼
        Tricopter,                  ///< 三旋翼
        FlappingWing,               ///< 扑翼
        Kite,                       ///< 风筝
        VtolTailsitterDuorotor,     ///< 双旋翼尾座式垂直起降
        VtolTailsitterQuadrotor,    ///< 四旋翼尾座式垂直起降
        VtolTiltrotor,              ///< 倾转旋翼垂直起降
        VtolFixedrotor,             ///< 独立固定旋翼垂直起降
        VtolTailsitter,             ///< 尾座式垂直起降
        VtolTiltwing,               ///< 倾转翼垂直起降
        Parafoil,                   ///< 可操纵非刚性翼伞
        Dodecarotor,                ///< 十二旋翼
        Decarotor,                  ///< 十旋翼
        Parachute,                  ///< 降落伞
        GenericMultirotor           ///< 通用多旋翼
    };
    Q_ENUM(Vehicle)

    enum Autopilot {
        Autopilot_Unknown,                    ///< 未知自动驾驶仪
        Px4,                        ///< PX4
        ArduPilot                   ///< ArduPilot
    };
    Q_ENUM(Autopilot)

    /**
     * @brief 获取载具类型的中文名称
     * @param type 载具类型枚举值
     * @return 中文名称
     */
    static QString getVehicleName(Vehicle type);

    /**
     * @brief 获取载具类型的中文名称（重载版本，接受 int）
     * @param type 载具类型枚举值（int）
     * @return 中文名称
     */
    static QString getVehicleName(int type);

    /**
     * @brief 获取自动驾驶仪类型的中文名称
     * @param type 自动驾驶仪类型枚举值
     * @return 中文名称
     */
    static QString getAutopilotName(Autopilot type);

    /**
     * @brief 获取自动驾驶仪类型的中文名称（重载版本，接受 int）
     * @param type 自动驾驶仪类型枚举值（int）
     * @return 中文名称
     */
    static QString getAutopilotName(int type);
};

Q_DECLARE_METATYPE(QAutoVehicleType::Vehicle)
Q_DECLARE_METATYPE(QAutoVehicleType::Autopilot)

#endif // _YTY_QAUTOVEHICLETYPE_H

