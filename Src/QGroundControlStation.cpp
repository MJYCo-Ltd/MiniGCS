#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include "QGroundControlStation.h"
#include "Plat/QAutopilot.h"
#include "Link/QDataLink.h"
#include "Private/QGroundControlStationPrivate.h"

QGroundControlStation::QGroundControlStation(QObject *parent)
    : QObject(parent)
    , d_ptr(new QGroundControlStationPrivate)
{
}

QGroundControlStation::~QGroundControlStation()
{
    ClearAllDataLink();

    // 如果有 d_ptr 的清理需要，确保删除
    if (d_ptr) {
        d_ptr.release();
    }
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
    m_listLink.push_back(pDataLink);

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
    QMetaObject::invokeMethod(pDataLink, "connectLink",
                              Qt::QueuedConnection);

    emit dataLinksChanged();

    return true;
}

void QGroundControlStation::RemoveDatLink(QDataLink* pDataLink)
{
    if (!pDataLink) {
        return;
    }

    auto itor = m_mapLink.find(pDataLink);
    if (m_mapLink.end() != itor)
    {
        dealDataLinkThread(itor);
        // 从集合中移除
        m_mapLink.erase(itor);
        m_listLink.removeAll(pDataLink);

        emit dataLinksChanged();
    }
}

void QGroundControlStation::ClearAllDataLink()
{
    // 断开并停止所有链接与线程
    for (auto itor = m_mapLink.begin(); itor != m_mapLink.end(); ++itor) {
        dealDataLinkThread(itor);
    }

    // 清空集合
    m_mapLink.clear();
    m_listLink.clear();
    emit dataLinksChanged();
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
    bool bNewPlatCreated = false;
    if (nullptr == pPlat) {
        if (bIsAutopilot) {
            pPlat = new QAutopilot(this);
        } else {
            pPlat = new QPlat(this);
        }
        m_mapId2Standalone.insert(uId,pPlat);
        m_listPlat.push_back(pPlat);
        bNewPlatCreated = true;
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

    // 如果创建了新的平台，发出数量变化信号
    if (bNewPlatCreated) {
        emit platsChanged();
    }

    return(pPlat);
}

void QGroundControlStation::dealDataLinkThread(
    QMap<QDataLink *, QThread *>::Iterator itor) {
    QDataLink *pDataLink = itor.key();
    QThread *thread = itor.value();

    // 在 link 所在线程安全地断开
    QMetaObject::invokeMethod(pDataLink, "disConnectLink",
                              Qt::QueuedConnection);
    QObject::disconnect(pDataLink, nullptr, this, nullptr);

    thread->quit();
    if (!thread->wait(2000)) {
        qWarning() << "Thread did not exit in time during ClearAllDataLink(), "
                      "terminating.";
        thread->terminate();
        thread->wait();
    }

    thread->deleteLater();
}