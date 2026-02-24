#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
#include <QDebug>
#include "Link/QTcpServerDataLink.h"

QTcpServerDataLink::QTcpServerDataLink(quint16 port, QObject *parent)
    : QDataLink(parent)
    , m_tcpServer(nullptr)
    , m_port(port)
{
}

QTcpServerDataLink::~QTcpServerDataLink()
{
}

bool QTcpServerDataLink::connectLink()
{
    if (!m_tcpServer) {
        m_tcpServer = new QTcpServer(this);
        connect(m_tcpServer, &QTcpServer::newConnection, this, &QTcpServerDataLink::onNewConnection);
    }

    if (m_tcpServer->isListening()) {
        return true;
    }

    if (m_tcpServer->listen(QHostAddress::Any, m_port)) {
        emit linkStatusChanged(true);
        return true;
    } else {
        QString errorMsg = QString("Failed to start TCP server on port %1: %2")
                               .arg(m_port)
                               .arg(m_tcpServer->errorString());
        emit linkStatusChanged(false);
        emit linkError(errorMsg);
        return false;
    }
}

void QTcpServerDataLink::disConnectLink()
{
    if (m_tcpServer && m_tcpServer->isListening()) {
        m_tcpServer->close();
        emit linkStatusChanged(false);
    }

    // 断开并清理所有客户端连接
    for (QTcpSocket* socket : m_clientSockets) {
        if (socket) {
            socket->disconnectFromHost();
            socket->deleteLater();
        }
    }
    m_clientSockets.clear();
}

bool QTcpServerDataLink::sendLinkData(const QByteArray &data)
{
    if (!isLinkConnected()) {
        return false;
    }

    if (data.isEmpty()) {
        return true;
    }

    // 向所有活跃的客户端socket发送
    if (m_clientSockets.isEmpty()) {
        return false;
    }

    bool allSuccess = true;

    // 移除已断开连接的socket
    QList<QTcpSocket*>::iterator it = m_clientSockets.begin();
    while (it != m_clientSockets.end()) {
        QTcpSocket* socket = *it;
        if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
            if (socket) {
                socket->deleteLater();
            }
            it = m_clientSockets.erase(it);
            continue;
        }

        qint64 bytesWritten = socket->write(data);
        if (bytesWritten == -1) {
            QString errorMsg = QString("Failed to write data to TCP socket %1:%2: %3")
                                   .arg(socket->peerAddress().toString())
                                   .arg(socket->peerPort())
                                   .arg(socket->errorString());
            emit linkError(errorMsg);
            allSuccess = false;
        } else if (bytesWritten != data.size()) {
            qWarning() << "QTcpServerDataLink: Partial write to" << socket->peerAddress().toString()
                       << ":" << socket->peerPort() << ":" << bytesWritten
                       << "of" << data.size() << "bytes";
        }

        ++it;
    }

    // 如果清理后没有客户端了，更新连接状态
    if (m_clientSockets.isEmpty()) {
        emit linkStatusChanged(false);
    }

    return allSuccess;
}

bool QTcpServerDataLink::isLinkConnected() const
{
    return m_tcpServer && m_tcpServer->isListening() && !m_clientSockets.isEmpty();
}

void QTcpServerDataLink::onSocketReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        return;
    }

    // 读取所有可用数据
    QByteArray data = socket->readAll();

    if (!data.isEmpty()) {
        // 发射接收到的数据信号
        emit messageReceived(data);
    }
}

void QTcpServerDataLink::onErrorOccurred()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        return;
    }

    QAbstractSocket::SocketError error = socket->error();
    if (error == QAbstractSocket::UnknownSocketError || error == QAbstractSocket::RemoteHostClosedError) {
        // 远程主机关闭连接是正常的，不需要报告错误
        return;
    }

    QString errorMsg = QString("TCP socket error from %1:%2: %3")
                           .arg(socket->peerAddress().toString())
                           .arg(socket->peerPort())
                           .arg(socket->errorString());
    emit linkError(errorMsg);

    // 错误socket会被onSocketDisconnected自动移除
}

void QTcpServerDataLink::onNewConnection()
{
    if (!m_tcpServer) {
        return;
    }

    // 接受新连接
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket* socket = m_tcpServer->nextPendingConnection();
        if (socket) {
            // 添加到客户端列表
            m_clientSockets.append(socket);

            // 连接信号
            connect(socket, &QTcpSocket::readyRead, this, &QTcpServerDataLink::onSocketReadyRead);
            connect(socket, &QTcpSocket::errorOccurred, this, &QTcpServerDataLink::onErrorOccurred);
            connect(socket, &QTcpSocket::disconnected, this, &QTcpServerDataLink::onSocketDisconnected);

            qDebug() << "QTcpServerDataLink: New client connected:" << socket->peerAddress().toString()
                     << ":" << socket->peerPort() << "(Total clients:" << m_clientSockets.size() << ")";
            emit linkStatusChanged(true);
        }
    }
}

void QTcpServerDataLink::onSocketDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        removeClientSocket(socket);

        qDebug() << "QTcpServerDataLink: Client disconnected:" << socket->peerAddress().toString()
                 << ":" << socket->peerPort() << "(Remaining clients:" << m_clientSockets.size() << ")";

        // 如果没有活跃客户端了，更新连接状态
        if (m_clientSockets.isEmpty()) {
            emit linkStatusChanged(false);
        }
    }
}

void QTcpServerDataLink::removeClientSocket(QTcpSocket* socket)
{
    if (!socket) {
        return;
    }

    // 从列表中移除
    m_clientSockets.removeAll(socket);

    // 断开信号连接
    socket->disconnect();

    // 删除socket
    socket->deleteLater();
}

void QTcpServerDataLink::onSendDataRequested(const QByteArray &data)
{
    // 在正确的线程中调用 sendLinkData
    sendLinkData(data);
}
