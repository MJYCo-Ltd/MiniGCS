#include "QGroundControlStation.h"
#include "QAutopilot.h"
#include "QDataLink.h"
#include "Private/QGroundControlStationPrivate.h"
#include <QDebug>
#include <QTimer>

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

bool QGroundControlStation::AddDataLink(QDataLink* pDataLink)
{
    if (!pDataLink) {
        return false;
    }

    // 检查是否已经存在
    if (m_mapLink.contains(pDataLink)) {
        return false;
    }

    // 添加到集合
    auto pThread = new QThread(this);
    m_mapLink.insert(pDataLink,pThread);

    // 连接信号槽，处理接收到的消息
    QObject::connect(pDataLink, &QDataLink::messageReceived, this, 
                     [this](const QByteArray &data) {
                         this->processDataLinkMessage(data);
                     });
    
    // 先移除父对象，然后移动到目标线程
    pDataLink->setParent(nullptr);
    pDataLink->moveToThread(pThread);
    
    // 启动线程
    pThread->start();
    
    // 在目标线程的事件循环中调用 connectLink()，确保 QSerialPort 在正确的线程中创建
    // 使用 QTimer::singleShot 确保在目标线程的事件循环中执行
    QTimer::singleShot(0, pDataLink, [pDataLink]() {
        pDataLink->connectLink();
    });

    return true;
}

void QGroundControlStation::RemoveDatLink(QDataLink* pDataLink)
{
    if (!pDataLink) {
        return;
    }
    pDataLink->disConnectLink();
    // 断开信号连接
    QObject::disconnect(pDataLink, nullptr, this, nullptr);
    auto itor = m_mapLink.find(pDataLink);
    if(m_mapLink.end() != itor)
    {
        itor.value()->exit();
        pDataLink->moveToThread(QThread::currentThread());
        pDataLink->setParent(this);
    }
    // 从集合中移除
    m_mapLink.erase(itor);
}

void QGroundControlStation::ClearAllDataLink()
{
    // 断开所有信号连接
    for (auto itor = m_mapLink.begin(); itor != m_mapLink.end();++itor) {
            itor.key()->disConnectLink();
            QObject::disconnect(itor.key(), nullptr, this, nullptr);
            itor.value()->exit();
            itor.key()->moveToThread(QThread::currentThread());
            itor.key()->setParent(this);
    }

    // 清空集合
    m_mapLink.clear();
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

    // 在目标线程中调用 sendLinkData，确保 QSerialPort 操作在正确的线程中执行
    for (auto itor = m_mapLink.begin(); itor != m_mapLink.end();++itor) {
        QDataLink* pDataLink = itor.key();
        if (pDataLink->isLinkConnected()) {
            // 使用 QMetaObject::invokeMethod 在目标线程中调用 onSendDataRequested 槽函数
            // 使用 Qt::QueuedConnection 确保异步执行，不阻塞
            QMetaObject::invokeMethod(pDataLink, "onSendDataRequested", Qt::QueuedConnection,
                                      Q_ARG(QByteArray, data));
        }
    }
}

QPlat *QGroundControlStation::getOrCreatePlat(uint8_t uId,bool bIsAutopilot)
{
    QPlat * pPlat = m_mapId2Standalone.value(uId,nullptr);
    if (nullptr == pPlat) {
        if (bIsAutopilot) {
            pPlat = new QAutopilot(this);
        } else {
            pPlat = new QPlat(this);
        }
        m_mapId2Standalone.insert(uId,pPlat);
    }else{
        /// 如果系统与之前保存的类型不一致，删除原来的重新构建
        if (!bIsAutopilot) {
            if (nullptr != qobject_cast<QAutopilot *>(pPlat)) {
                pPlat->deleteLater();
                pPlat = new QPlat(this);
                m_mapId2Standalone[uId] = pPlat;
            }
        }else{
            if(nullptr == qobject_cast<QAutopilot*>(pPlat)){
                pPlat->deleteLater();
                pPlat = new QAutopilot(this);
                m_mapId2Standalone[uId] = pPlat;
            }
        }
    }

    return(pPlat);
}

int QGroundControlStation::getVehicleCount() const
{
    return d_ptr->getSystemCount();
}

QVector<QPlat *> QGroundControlStation::getAllStandalone() const
{
    QVector<QPlat*> result;
    for (auto standalone : m_mapId2Standalone) {
        result.append(standalone);
    }
    return result;
}

