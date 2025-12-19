#include <QDebug>
#include <QDateTime>
#include "Plat/QAutopilot.h"
#include "Plat/Private/QAutopilotPrivate.h"

QAutopilot::QAutopilot(QObject *parent)
{
}

QAutopilot::~QAutopilot()
{
}

QAutopilotPrivate* QAutopilot::d_func()
{
    return static_cast<QAutopilotPrivate*>(d_ptr.get());
}

const QAutopilotPrivate* QAutopilot::d_func() const
{
    return static_cast<const QAutopilotPrivate*>(d_ptr.get());
}

QString QAutopilot::getAutopilotType() const
{
    return d_func()->getAutopilotType();
}

void QAutopilot::setAutopilotType(const QString &autopilotType)
{
    if (d_func()->getAutopilotType() != autopilotType) {
        d_func()->setAutopilotType(autopilotType);
        emit infoUpdated();
    }
}

QString QAutopilot::getVehicleType() const
{
    return d_func()->getVehicleType();
}

void QAutopilot::setVehicleType(const QString &vehicleType)
{
    if (d_func()->getVehicleType() != vehicleType) {
        d_func()->setVehicleType(vehicleType);
        emit infoUpdated();
    }
}

void QAutopilot::positionUpdate(double dLon, double dLat, float dH)
{
    m_recPostion.setLongitude(dLon);
    m_recPostion.setLatitude(dLat);
    m_recPostion.setAltitude(dH);
}

void QAutopilot::nedUpdate(float dNorth, float dEast, float dDown)
{
    m_recPostion.setNedX(dNorth);
    m_recPostion.setNedY(dEast);
    m_recPostion.setNedZ(dDown);
}
