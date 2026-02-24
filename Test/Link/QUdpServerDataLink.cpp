#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include "Link/QUdpServerDataLink.h"

QUdpServerDataLink::QUdpServerDataLink(quint16 port, QObject *parent)
    : QDataLink(parent)
    , m_udpSocket(nullptr)
    , m_port(port)
    , m_cleanupTimer(nullptr)
{
}

QUdpServerDataLink::~QUdpServerDataLink()
{
}

bool QUdpServerDataLink::connectLink()
{
    if (!m_udpSocket) {
        m_udpSocket = new QUdpSocket(this);
        connect(m_udpSocket, &QUdpSocket::readyRead, this, &QUdpServerDataLink::onReadyRead);
    }

    if (m_udpSocket->state() == QAbstractSocket::BoundState ||
        m_udpSocket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    // 绑定到指定端口进行监听
    if (m_udpSocket->bind(QHostAddress::Any, m_port)) {
        // 创建并启动清理定时器
        if (!m_cleanupTimer) {
            m_cleanupTimer = new QTimer(this);
            connect(m_cleanupTimer, &QTimer::timeout, this, &QUdpServerDataLink::cleanupInactiveClients);
            // 每5秒检查一次超时客户端
            m_cleanupTimer->start(5000);
        }
        emit linkStatusChanged(true);
        return true;
    } else {
        QString errorMsg = QString("Failed to bind UDP socket to port %1: %2")
                               .arg(m_port)
                               .arg(m_udpSocket->errorString());
        emit linkStatusChanged(false);
        emit linkError(errorMsg);
        return false;
    }
}

void QUdpServerDataLink::disConnectLink()
{
    if (m_cleanupTimer) {
        m_cleanupTimer->stop();
        m_cleanupTimer->deleteLater();
        m_cleanupTimer = nullptr;
    }

    if (m_udpSocket && (m_udpSocket->state() == QAbstractSocket::BoundState ||
                        m_udpSocket->state() == QAbstractSocket::ConnectedState)) {
        m_udpSocket->close();
        emit linkStatusChanged(false);
    }

    m_clientMap.clear();
}

bool QUdpServerDataLink::sendLinkData(const QByteArray &data)
{
    if (!isLinkConnected()) {
        return false;
    }

    if (data.isEmpty()) {
        return true;
    }

    if (!m_udpSocket) {
        return false;
    }

    // 如果没有客户端列表，无法发送
    if (m_clientMap.isEmpty()) {
        emit linkError("No clients available for UDP send");
        return false;
    }

    bool allSuccess = true;

    // 向所有保存的客户端发送
    QMap<UdpClientKey, UdpClientInfo>::const_iterator it = m_clientMap.constBegin();
    while (it != m_clientMap.constEnd()) {
        const UdpClientInfo &clientInfo = it.value();

        qint64 bytesWritten = m_udpSocket->writeDatagram(data, clientInfo.address, clientInfo.port);
        if (bytesWritten == -1) {
            QString errorMsg = QString("Failed to write UDP datagram to %1:%2: %3")
                                   .arg(clientInfo.address.toString())
                                   .arg(clientInfo.port)
                                   .arg(m_udpSocket->errorString());
            emit linkError(errorMsg);
            allSuccess = false;
        } else if (bytesWritten != data.size()) {
            qWarning() << "QUdpServerDataLink: Partial write to" << clientInfo.address.toString()
                       << ":" << clientInfo.port << ":" << bytesWritten
                       << "of" << data.size() << "bytes";
        }

        ++it;
    }

    return allSuccess;
}

bool QUdpServerDataLink::isLinkConnected() const
{
    return m_udpSocket && (m_udpSocket->state() == QAbstractSocket::BoundState ||
                           m_udpSocket->state() == QAbstractSocket::ConnectedState);
}

void QUdpServerDataLink::onReadyRead()
{
    if (!m_udpSocket) {
        return;
    }

    // 读取所有可用的UDP数据报
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress senderAddress;
        quint16 senderPort;

        qint64 bytesRead = m_udpSocket->readDatagram(datagram.data(), datagram.size(),
                                                      &senderAddress, &senderPort);

        if (bytesRead > 0 && !datagram.isEmpty()) {
            // 更新或添加客户端信息
            updateClientInfo(senderAddress, senderPort);

            // 发射接收到的数据信号
            emit messageReceived(datagram);
        }
    }
}

void QUdpServerDataLink::onSendDataRequested(const QByteArray &data)
{
    // 在正确的线程中调用 sendLinkData
    sendLinkData(data);
}

void QUdpServerDataLink::updateClientInfo(const QHostAddress &address, quint16 port)
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    UdpClientKey key = getClientKey(address, port);

    // 更新或添加客户端信息
    if (m_clientMap.contains(key)) {
        // 更新最后活跃时间
        m_clientMap[key].lastActiveTime = currentTime;
    } else {
        // 添加新客户端
        m_clientMap[key] = UdpClientInfo(address, port, currentTime);
    }
}

UdpClientKey QUdpServerDataLink::getClientKey(const QHostAddress &address, quint16 port) const
{
    return UdpClientKey(address.toString(), port);
}

void QUdpServerDataLink::cleanupInactiveClients()
{
    if (m_clientMap.isEmpty()) {
        return;
    }

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    QMap<UdpClientKey, UdpClientInfo>::iterator it = m_clientMap.begin();

    while (it != m_clientMap.end()) {
        qint64 inactiveTime = currentTime - it.value().lastActiveTime;

        if (inactiveTime > CLIENT_TIMEOUT_MS) {
            // 移除超时的客户端
            qDebug() << "QUdpServerDataLink: Removing inactive client"
                     << it.value().address.toString() << ":" << it.value().port
                     << "(inactive for" << inactiveTime / 1000 << "seconds)";
            it = m_clientMap.erase(it);
        } else {
            ++it;
        }
    }
}
