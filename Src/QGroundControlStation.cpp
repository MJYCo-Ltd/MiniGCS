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

QDataLink* QGroundControlStation::createDataLink(ConnectionType connectionType, const QString &address, int portOrBaudRate)
{
    // 生成连接字符串
    QString connectionString = generateConnectionString(connectionType, address, portOrBaudRate);
    
    // 使用内部实现创建数据链路
    return createDataLinkInternal(connectionString);
}

void QGroundControlStation::disconnect()
{
    // 断开所有数据链路
    for (auto dataLink : m_connectionStringMap) {
        if (dataLink) {
            dataLink->disconnect();
        }
    }
    m_connectionStringMap.clear();
    
    // 发射数量变化信号
    emitDataLinkCountChanged();
    
    qDebug() << "QGroundControlStation: Disconnected from all data links";
}

void QGroundControlStation::removeDataLink(QDataLink* dataLink)
{
    if (!dataLink) {
        return;
    }
    
    // 断开数据链路
    dataLink->disconnect();
    
    // 从映射中移除（需要找到对应的连接字符串）
    QString connectionStringToRemove;
    for (auto it = m_connectionStringMap.begin(); it != m_connectionStringMap.end(); ++it) {
        if (it.value() == dataLink) {
            connectionStringToRemove = it.key();
            break;
        }
    }
    if (!connectionStringToRemove.isEmpty()) {
        m_connectionStringMap.remove(connectionStringToRemove);
    }
    
    // 删除对象
    dataLink->deleteLater();
    
    // 发射数量变化信号
    emitDataLinkCountChanged();
    
    qDebug() << "QGroundControlStation: Removed data link, remaining:" << m_connectionStringMap.size();
}





QVector<QDataLink*> QGroundControlStation::getAllDataLinks() const
{
    QVector<QDataLink*> result;
    for (auto dataLink : m_connectionStringMap) {
        result.append(dataLink);
    }
    return result;
}

QString QGroundControlStation::generateConnectionString(ConnectionType connectionType, const QString &address, int portOrBaudRate) const
{
    QString result;
    
    switch (static_cast<int>(connectionType)) {
        case 0: // Serial
            result = QString("serial://%1:%2").arg(address).arg(portOrBaudRate);
            break;
            
        case 1: // TCP
            result = QString("tcp://%1:%2").arg(address).arg(portOrBaudRate);
            break;
            
        case 2: // UDP
            if (address.isEmpty() || address == "0.0.0.0") {
                // 如果地址为空或者是默认地址，使用端口号作为监听端口
                result = QString("udp://0.0.0.0:%1").arg(portOrBaudRate);
            } else {
                result = QString("udp://%1:%2").arg(address).arg(portOrBaudRate);
            }
            break;
    }
    
    return result;
}

QDataLink* QGroundControlStation::findExistingDataLink(const QString &connectionString) const
{
    return m_connectionStringMap.value(connectionString, nullptr);
}


QDataLink* QGroundControlStation::createDataLinkInternal(const QString &connectionString)
{
    // 检查是否已经存在相同的连接
    QDataLink* existingDataLink = findExistingDataLink(connectionString);
    if (existingDataLink) {
        qDebug() << "QGroundControlStation: Found existing data link for" << connectionString;
        return existingDataLink;
    }
    
    // 创建新的数据链路
    QDataLink* dataLink = new QDataLink(this);
    
    // 设置连接字符串
    dataLink->setConnectionString(connectionString);
    
    // 添加到映射
    m_connectionStringMap.insert(connectionString, dataLink);
    
    // 发射数量变化信号
    emitDataLinkCountChanged();
    
    qDebug() << "QGroundControlStation: Created and connected new data link, total:" << m_connectionStringMap.size();
    return dataLink;
}

void QGroundControlStation::emitDataLinkCountChanged()
{
    emit dataLinkCountChanged(m_connectionStringMap.size());
}

