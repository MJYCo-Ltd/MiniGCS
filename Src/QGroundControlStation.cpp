#include "QGroundControlStation.h"
#include "QVehicle.h"
#include "QDataLink.h"
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <chrono>
#include <sstream>
#include <mavsdk/mavsdk.h>
#include <mavsdk/system.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/info/info.h>

QGroundControlStation::QGroundControlStation(QObject *parent)
    : QObject(parent)
{
}

QGroundControlStation::~QGroundControlStation()
{
    disconnect();
}

QDataLink* QGroundControlStation::createDataLink()
{
    // 创建新的数据链路
    QDataLink* dataLink = new QDataLink(this);
    dataLink->initializeMavsdk();
    
    // 数据链路创建完成，用户可以自己连接需要的信号
    
    // 添加到列表
    m_dataLinks.append(dataLink);
    
    qDebug() << "QGroundControlStation: Created new data link, total:" << m_dataLinks.size();
    return dataLink;
}

void QGroundControlStation::disconnect()
{
    // 断开所有数据链路
    for (auto dataLink : m_dataLinks) {
        if (dataLink) {
            dataLink->disconnect();
        }
    }
    m_dataLinks.clear();
    qDebug() << "QGroundControlStation: Disconnected from all data links";
}

void QGroundControlStation::removeDataLink(QDataLink* dataLink)
{
    if (!dataLink) {
        return;
    }
    
    // 断开数据链路
    dataLink->disconnect();
    
    // 从列表中移除
    m_dataLinks.removeOne(dataLink);
    
    // 删除对象
    dataLink->deleteLater();
    
    qDebug() << "QGroundControlStation: Removed data link, remaining:" << m_dataLinks.size();
}





QVector<QDataLink*> QGroundControlStation::getAllDataLinks() const
{
    return m_dataLinks;
}

