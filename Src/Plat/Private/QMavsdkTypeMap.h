#ifndef QMAVSDKTYPEMAP_H
#define QMAVSDKTYPEMAP_H

#include "Plat/QAutopilot.h"
#include "Plat/QAutoVehicleType.h"

#include <mavsdk/autopilot.h>
#include <mavsdk/vehicle.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

namespace MavsdkTypeMap {

inline QAutopilot::FlightMode toFlightMode(mavsdk::Telemetry::FlightMode mode)
{
    switch (mode) {
    case mavsdk::Telemetry::FlightMode::Ready:
        return QAutopilot::FlightModeReady;
    case mavsdk::Telemetry::FlightMode::Takeoff:
        return QAutopilot::FlightModeTakeoff;
    case mavsdk::Telemetry::FlightMode::Hold:
        return QAutopilot::FlightModeHold;
    case mavsdk::Telemetry::FlightMode::Mission:
        return QAutopilot::FlightModeMission;
    case mavsdk::Telemetry::FlightMode::ReturnToLaunch:
        return QAutopilot::FlightModeReturnToLaunch;
    case mavsdk::Telemetry::FlightMode::Land:
        return QAutopilot::FlightModeLand;
    case mavsdk::Telemetry::FlightMode::Offboard:
        return QAutopilot::FlightModeOffboard;
    case mavsdk::Telemetry::FlightMode::FollowMe:
        return QAutopilot::FlightModeFollowMe;
    case mavsdk::Telemetry::FlightMode::Manual:
        return QAutopilot::FlightModeManual;
    case mavsdk::Telemetry::FlightMode::Altctl:
        return QAutopilot::FlightModeAltitudeControl;
    case mavsdk::Telemetry::FlightMode::Posctl:
        return QAutopilot::FlightModePositionControl;
    case mavsdk::Telemetry::FlightMode::Acro:
        return QAutopilot::FlightModeAcro;
    case mavsdk::Telemetry::FlightMode::Stabilized:
        return QAutopilot::FlightModeStabilized;
    case mavsdk::Telemetry::FlightMode::Rattitude:
        return QAutopilot::FlightModeRattitude;
    case mavsdk::Telemetry::FlightMode::Unknown:
    default:
        return QAutopilot::FlightModeUnknown;
    }
}

inline QAutopilot::LandedState toLandedState(mavsdk::Telemetry::LandedState state)
{
    switch (state) {
    case mavsdk::Telemetry::LandedState::OnGround:
        return QAutopilot::LandedOnGround;
    case mavsdk::Telemetry::LandedState::InAir:
        return QAutopilot::LandedInAir;
    case mavsdk::Telemetry::LandedState::TakingOff:
        return QAutopilot::LandedTakingOff;
    case mavsdk::Telemetry::LandedState::Landing:
        return QAutopilot::LandedLanding;
    case mavsdk::Telemetry::LandedState::Unknown:
    default:
        return QAutopilot::LandedStateUnknown;
    }
}

inline QAutoVehicleType::Autopilot toAutopilot(mavsdk::Autopilot type)
{
    switch (type) {
    case mavsdk::Autopilot::Px4:
        return QAutoVehicleType::Px4;
    case mavsdk::Autopilot::ArduPilot:
        return QAutoVehicleType::ArduPilot;
    case mavsdk::Autopilot::Unknown:
    default:
        return QAutoVehicleType::Autopilot_Unknown;
    }
}

inline QAutoVehicleType::Vehicle toVehicle(mavsdk::Vehicle type)
{
    switch (type) {
    case mavsdk::Vehicle::Generic:
        return QAutoVehicleType::Generic;
    case mavsdk::Vehicle::FixedWing:
        return QAutoVehicleType::FixedWing;
    case mavsdk::Vehicle::Quadrotor:
        return QAutoVehicleType::Quadrotor;
    case mavsdk::Vehicle::Coaxial:
        return QAutoVehicleType::Coaxial;
    case mavsdk::Vehicle::Helicopter:
        return QAutoVehicleType::Helicopter;
    case mavsdk::Vehicle::Airship:
        return QAutoVehicleType::Airship;
    case mavsdk::Vehicle::FreeBalloon:
        return QAutoVehicleType::FreeBalloon;
    case mavsdk::Vehicle::Rocket:
        return QAutoVehicleType::Rocket;
    case mavsdk::Vehicle::GroundRover:
        return QAutoVehicleType::GroundRover;
    case mavsdk::Vehicle::SurfaceBoat:
        return QAutoVehicleType::SurfaceBoat;
    case mavsdk::Vehicle::Submarine:
        return QAutoVehicleType::Submarine;
    case mavsdk::Vehicle::Hexarotor:
        return QAutoVehicleType::Hexarotor;
    case mavsdk::Vehicle::Octorotor:
        return QAutoVehicleType::Octorotor;
    case mavsdk::Vehicle::Tricopter:
        return QAutoVehicleType::Tricopter;
    case mavsdk::Vehicle::FlappingWing:
        return QAutoVehicleType::FlappingWing;
    case mavsdk::Vehicle::Kite:
        return QAutoVehicleType::Kite;
    case mavsdk::Vehicle::VtolTailsitterDuorotor:
        return QAutoVehicleType::VtolTailsitterDuorotor;
    case mavsdk::Vehicle::VtolTailsitterQuadrotor:
        return QAutoVehicleType::VtolTailsitterQuadrotor;
    case mavsdk::Vehicle::VtolTiltrotor:
        return QAutoVehicleType::VtolTiltrotor;
    case mavsdk::Vehicle::VtolFixedrotor:
        return QAutoVehicleType::VtolFixedrotor;
    case mavsdk::Vehicle::VtolTailsitter:
        return QAutoVehicleType::VtolTailsitter;
    case mavsdk::Vehicle::VtolTiltwing:
        return QAutoVehicleType::VtolTiltwing;
    case mavsdk::Vehicle::Parafoil:
        return QAutoVehicleType::Parafoil;
    case mavsdk::Vehicle::Dodecarotor:
        return QAutoVehicleType::Dodecarotor;
    case mavsdk::Vehicle::Decarotor:
        return QAutoVehicleType::Decarotor;
    case mavsdk::Vehicle::Parachute:
        return QAutoVehicleType::Parachute;
    case mavsdk::Vehicle::GenericMultirotor:
        return QAutoVehicleType::GenericMultirotor;
    case mavsdk::Vehicle::Unknown:
    default:
        return QAutoVehicleType::Vehicle_Unknown;
    }
}

} // namespace MavsdkTypeMap

#endif // QMAVSDKTYPEMAP_H
