#include "QVehicle.h"
#include "Private/QVehiclePrivate.h"
#include <QDebug>
#include <QDateTime>
#include <sstream>

QVehicle::QVehicle(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<QVehiclePrivate>())
{
}

QVehicle::QVehicle(uint8_t unID, QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<QVehiclePrivate>())
{
    d_ptr->setUnID(unID);
}

QVehicle::~QVehicle()
{
}

uint8_t QVehicle::getUnID() const
{
    return d_ptr->getUnID();
}

void QVehicle::setUnID(uint8_t unID)
{
    if (d_ptr->getUnID() != unID) {
        d_ptr->setUnID(unID);
        emit infoUpdated();
    }
}

uint8_t QVehicle::getComponentId() const
{
    return d_ptr->getComponentId();
}

void QVehicle::setComponentId(uint8_t componentId)
{
    if (d_ptr->getComponentId() != componentId) {
        d_ptr->setComponentId(componentId);
        emit infoUpdated();
    }
}

QString QVehicle::getAutopilotType() const
{
    return d_ptr->getAutopilotType();
}

void QVehicle::setAutopilotType(const QString &autopilotType)
{
    if (d_ptr->getAutopilotType() != autopilotType) {
        d_ptr->setAutopilotType(autopilotType);
        emit infoUpdated();
    }
}

QString QVehicle::getVehicleType() const
{
    return d_ptr->getVehicleType();
}

void QVehicle::setVehicleType(const QString &vehicleType)
{
    if (d_ptr->getVehicleType() != vehicleType) {
        d_ptr->setVehicleType(vehicleType);
        emit infoUpdated();
    }
}

QString QVehicle::getFirmwareVersion() const
{
    return d_ptr->getFirmwareVersion();
}

void QVehicle::setFirmwareVersion(const QString &firmwareVersion)
{
    if (d_ptr->getFirmwareVersion() != firmwareVersion) {
        d_ptr->setFirmwareVersion(firmwareVersion);
        emit infoUpdated();
    }
}

QString QVehicle::getHardwareVersion() const
{
    return d_ptr->getHardwareVersion();
}

void QVehicle::setHardwareVersion(const QString &hardwareVersion)
{
    if (d_ptr->getHardwareVersion() != hardwareVersion) {
        d_ptr->setHardwareVersion(hardwareVersion);
        emit infoUpdated();
    }
}

QString QVehicle::getSoftwareVersion() const
{
    return d_ptr->getSoftwareVersion();
}

void QVehicle::setSoftwareVersion(const QString &softwareVersion)
{
    if (d_ptr->getSoftwareVersion() != softwareVersion) {
        d_ptr->setSoftwareVersion(softwareVersion);
        emit infoUpdated();
    }
}

bool QVehicle::isConnected() const
{
    return d_ptr->isConnected();
}

void QVehicle::setConnected(bool connected)
{
    if (d_ptr->isConnected() != connected) {
        d_ptr->setConnected(connected);
        
        emit connectionStatusChanged(connected);
        emit infoUpdated();
    }
}

bool QVehicle::hasCamera() const
{
    return d_ptr->hasCamera();
}

void QVehicle::setHasCamera(bool hasCamera)
{
    if (d_ptr->hasCamera() != hasCamera) {
        d_ptr->setHasCamera(hasCamera);
        emit infoUpdated();
    }
}

QVector<uint8_t> QVehicle::getComponentIds() const
{
    return d_ptr->getComponentIds();
}

void QVehicle::setComponentIds(const QVector<uint8_t> &componentIds)
{
    if (d_ptr->getComponentIds() != componentIds) {
        d_ptr->setComponentIds(componentIds);
        emit infoUpdated();
    }
}

QDateTime QVehicle::getLastConnectedTime() const
{
    return d_ptr->getLastConnectedTime();
}

void QVehicle::setLastConnectedTime(const QDateTime &time)
{
    d_ptr->setLastConnectedTime(time);
}

QDateTime QVehicle::getLastDisconnectedTime() const
{
    return d_ptr->getLastDisconnectedTime();
}

void QVehicle::setLastDisconnectedTime(const QDateTime &time)
{
    d_ptr->setLastDisconnectedTime(time);
}

void QVehicle::updateFromSystem(void* system)
{
    d_ptr->updateFromSystem(system);
    
    // 设置消息处理回调
    d_ptr->setupMessageHandling(this);
    
    emit infoUpdated();
}

QString QVehicle::toString() const
{
    return d_ptr->toString();
}
