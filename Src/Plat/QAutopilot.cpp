#include "QAutopilot.h"
#include "Private/QAutopilotPrivate.h"
#include <QDebug>
#include <QDateTime>

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
