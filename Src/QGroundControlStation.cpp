#include "QGroundControlStation.h"
#include "QVehicle.h"
#include "QDataLink.h"
#include "Private/QGroundControlStationPrivate.h"
#include <QDebug>

QGroundControlStation::QGroundControlStation(QObject *parent)
    : QObject(parent)
    , d_ptr(std::make_unique<QGroundControlStationPrivate>())
{
}

QGroundControlStation::~QGroundControlStation()
{
    ClearAllDataLink();
}

void QGroundControlStation::Init()
{
    d_ptr->initializeMavsdk();
    d_ptr->setupConnectionErrorHandling(this);
    d_ptr->setupNewSystemDiscoveryCallback(this);
    
    // 设置RawBytes回调，用于将MAVSDK需要发送的数据通过DataLink发送出去
    // 传递 this 指针确保线程安全
    d_ptr->setupRawBytesToBeSentCallback([this](const QByteArray &data) {
        this->sendDataToAllLinks(data);
    }, this);
}

bool QGroundControlStation::AddDataLink(const QDataLink* pDataLink)
{
    if (!pDataLink) {
        qWarning() << "QGroundControlStation: Cannot add null DataLink";
        return false;
    }

    // 检查是否已经存在
    if (m_setLink.contains(pDataLink)) {
        qDebug() << "QGroundControlStation: DataLink already exists";
        return false;
    }

    // 添加到集合
    m_setLink.insert(pDataLink);

    // 连接信号槽，处理接收到的消息
    QObject::connect(pDataLink, &QDataLink::messageReceived, this, 
                     [this](const QByteArray &data) {
                         // 将接收到的数据传递给MAVSDK处理
                         this->processDataLinkMessage(data);
                     });

    qDebug() << "QGroundControlStation: DataLink added, total:" << m_setLink.size();
    return true;
}

void QGroundControlStation::RemoveDatLink(const QDataLink* pDataLink)
{
    if (!pDataLink) {
        return;
    }

    // 断开信号连接
    QObject::disconnect(pDataLink, nullptr, this, nullptr);

    // 从集合中移除
    if (m_setLink.remove(pDataLink)) {
        qDebug() << "QGroundControlStation: DataLink removed, remaining:" << m_setLink.size();
    }
}

void QGroundControlStation::ClearAllDataLink()
{
    // 断开所有信号连接
    for (const QDataLink* link : m_setLink) {
        if (link) {
            QObject::disconnect(link, nullptr, this, nullptr);
        }
    }

    // 清空集合
    m_setLink.clear();
    qDebug() << "QGroundControlStation: All DataLinks cleared";
}

void QGroundControlStation::processDataLinkMessage(const QByteArray &data)
{
    // 将接收到的数据传递给MAVSDK处理
    if (d_ptr) {
        d_ptr->processReceivedRawData(data);
    }
}

void QGroundControlStation::sendDataToAllLinks(const QByteArray &data)
{
    if (data.isEmpty()) {
        return;
    }

    int sentCount = 0;
    for (const QDataLink* link : m_setLink) {
        if (link && link->isConnected()) {
            // 使用const_cast因为sendData不是const方法，但我们需要发送数据
            if (const_cast<QDataLink*>(link)->sendData(data)) {
                sentCount++;
            }
        }
    }
}

int QGroundControlStation::getVehicleCount() const
{
    return d_ptr->getSystemCount();
}

QVector<QVehicle*> QGroundControlStation::getAllVehicles() const
{
    return d_ptr->getAllVehicles();
}

