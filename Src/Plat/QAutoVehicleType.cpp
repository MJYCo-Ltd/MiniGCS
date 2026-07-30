#include "Plat/QAutoVehicleType.h"

#include "Private/QMavsdkTextCatalog.h"

QString QAutoVehicleType::getVehicleName(Vehicle type)
{
    return QMavsdkTextCatalog::text(
        QStringLiteral("vehicle"), static_cast<int>(type));
}

QString QAutoVehicleType::getVehicleName(int type)
{
    return QMavsdkTextCatalog::text(QStringLiteral("vehicle"), type);
}

QString QAutoVehicleType::getAutopilotName(Autopilot type)
{
    return QMavsdkTextCatalog::text(
        QStringLiteral("autopilot"), static_cast<int>(type));
}

QString QAutoVehicleType::getAutopilotName(int type)
{
    return QMavsdkTextCatalog::text(QStringLiteral("autopilot"), type);
}
