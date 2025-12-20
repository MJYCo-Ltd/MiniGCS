#include "Plat/QAutoVehicleType.h"

QString QAutoVehicleType::getVehicleName(Vehicle type) {
    switch (type) {
    case Vehicle::Generic:
        return "通用微型飞行器";
    case Vehicle::FixedWing:
        return "固定翼飞机";
    case Vehicle::Quadrotor:
        return "四旋翼";
    case Vehicle::Coaxial:
        return "共轴直升机";
    case Vehicle::Helicopter:
        return "直升机";
    case Vehicle::Airship:
        return "飞艇";
    case Vehicle::FreeBalloon:
        return "自由气球";
    case Vehicle::Rocket:
        return "火箭";
    case Vehicle::GroundRover:
        return "地面漫游车";
    case Vehicle::SurfaceBoat:
        return "水面船只";
    case Vehicle::Submarine:
        return "潜艇";
    case Vehicle::Hexarotor:
        return "六旋翼";
    case Vehicle::Octorotor:
        return "八旋翼";
    case Vehicle::Tricopter:
        return "三旋翼";
    case Vehicle::FlappingWing:
        return "扑翼";
    case Vehicle::Kite:
        return "风筝";
    case Vehicle::VtolTailsitterDuorotor:
        return "双旋翼尾座式垂直起降";
    case Vehicle::VtolTailsitterQuadrotor:
        return "四旋翼尾座式垂直起降";
    case Vehicle::VtolTiltrotor:
        return "倾转旋翼垂直起降";
    case Vehicle::VtolFixedrotor:
        return "独立固定旋翼垂直起降";
    case Vehicle::VtolTailsitter:
        return "尾座式垂直起降";
    case Vehicle::VtolTiltwing:
        return "倾转翼垂直起降";
    case Vehicle::Parafoil:
        return "可操纵非刚性翼伞";
    case Vehicle::Dodecarotor:
        return "十二旋翼";
    case Vehicle::Decarotor:
        return "十旋翼";
    case Vehicle::Parachute:
        return "降落伞";
    case Vehicle::GenericMultirotor:
        return "通用多旋翼";
    default:
        return "未知";
    }
}

QString QAutoVehicleType::getVehicleName(int type) {
    return getVehicleName(static_cast<Vehicle>(type));
}

QString QAutoVehicleType::getAutopilotName(Autopilot type) {
    switch (type) {
    case Autopilot::Px4:
        return "PX4";
    case Autopilot::ArduPilot:
        return "ArduPilot";
    default:
        return "未知";
    }
}

QString QAutoVehicleType::getAutopilotName(int type) {
    return getAutopilotName(static_cast<Autopilot>(type));
}

