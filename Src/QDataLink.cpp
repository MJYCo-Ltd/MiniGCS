#include "QDataLink.h"
#include "QVehicle.h"
#include "Private/QDataLinkPrivate.h"
#include <QDebug>
#include <QMetaObject>
#include <QMetaMethod>

QDataLink::QDataLink(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<QDataLinkPrivate>())
{
}

QDataLink::~QDataLink()
{
    disconnect();
}

void QDataLink::setConnectionString(const QString &connectionString)
{
    d_ptr->initializeMavsdk();
    d_ptr->setupConnectionErrorHandling(this);
    d_ptr->setupNewSystemDiscoveryCallback(this);
    d_ptr->setupSystemConnectionCallbacks(this);
    d_ptr->setConnectionString(connectionString);
}

bool QDataLink::connectToDataLink()
{
    return d_ptr->connectToDataLink();
}

void QDataLink::disconnect()
{
    d_ptr->disconnect();
}

bool QDataLink::isConnected() const
{
    return d_ptr->isConnected();
}

int QDataLink::getVehicleCount() const
{
    return d_ptr->getSystemCount();
}

QVector<QVehicle*> QDataLink::getAllVehicles() const
{
    return d_ptr->getAllVehicles();
}
