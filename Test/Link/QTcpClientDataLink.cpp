#include <QTcpSocket>
#include <QHostAddress>
#include <QDebug>
#include "Link/QTcpClientDataLink.h"

QTcpClientDataLink::QTcpClientDataLink(const QString &hostName, quint16 port, QObject *parent)
    : QDataLink(parent)
    , m_tcpSocket(nullptr)
    , m_hostName(hostName)
    , m_port(port)
{
}

QTcpClientDataLink::~QTcpClientDataLink()
{
}

bool QTcpClientDataLink::connectLink()
{
    if (!m_tcpSocket) {
        m_tcpSocket = new QTcpSocket(this);
        connect(m_tcpSocket, &QTcpSocket::readyRead, this, &QTcpClientDataLink::onSocketReadyRead);
        connect(m_tcpSocket, &QTcpSocket::errorOccurred, this, &QTcpClientDataLink::onErrorOccurred);
        connect(m_tcpSocket, &QTcpSocket::disconnected, this, &QTcpClientDataLink::onSocketDisconnected);
        connect(m_tcpSocket, &QTcpSocket::connected, this, [this]() {
            emit linkStatusChanged(true);
            qDebug() << "QTcpClientDataLink: Connected to" << m_hostName << ":" << m_port;
        });
    }

    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    if (m_tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        m_tcpSocket->abort();
    }

    m_tcpSocket->connectToHost(m_hostName, m_port);
    if (m_tcpSocket->waitForConnected(5000)) {
        return true;
    }

    QString errorMsg = QString("QTcpClientDataLink: Failed to connect to %1:%2: %3")
                           .arg(m_hostName).arg(m_port).arg(m_tcpSocket->errorString());
    emit linkStatusChanged(false);
    emit linkError(errorMsg);
    return false;
}

void QTcpClientDataLink::disConnectLink()
{
    if (m_tcpSocket) {
        m_tcpSocket->disconnectFromHost();
        if (m_tcpSocket->state() != QAbstractSocket::UnconnectedState) {
            m_tcpSocket->waitForDisconnected(3000);
        }
        m_tcpSocket->abort();
        emit linkStatusChanged(false);
        qDebug() << "QTcpClientDataLink: Disconnected from" << m_hostName << ":" << m_port;
    }
}

bool QTcpClientDataLink::sendLinkData(const QByteArray &data)
{
    if (!isLinkConnected() || !m_tcpSocket) {
        return false;
    }
    if (data.isEmpty()) {
        return true;
    }

    qint64 bytesWritten = m_tcpSocket->write(data);
    if (bytesWritten == -1) {
        QString errorMsg = QString("QTcpClientDataLink: Failed to write: %1").arg(m_tcpSocket->errorString());
        emit linkError(errorMsg);
        return false;
    }
    if (bytesWritten != data.size()) {
        qWarning() << "QTcpClientDataLink: Partial write" << bytesWritten << "of" << data.size();
    }
    return true;
}

bool QTcpClientDataLink::isLinkConnected() const
{
    return m_tcpSocket && m_tcpSocket->state() == QAbstractSocket::ConnectedState;
}

void QTcpClientDataLink::onSocketReadyRead()
{
    if (!m_tcpSocket) return;
    QByteArray data = m_tcpSocket->readAll();
    if (!data.isEmpty()) {
        emit messageReceived(data);
    }
}

void QTcpClientDataLink::onErrorOccurred()
{
    if (!m_tcpSocket) return;
    QAbstractSocket::SocketError error = m_tcpSocket->error();
    if (error == QAbstractSocket::UnknownSocketError || error == QAbstractSocket::RemoteHostClosedError) {
        return;
    }
    emit linkError(QString("QTcpClientDataLink: %1").arg(m_tcpSocket->errorString()));
}

void QTcpClientDataLink::onSocketDisconnected()
{
    emit linkStatusChanged(false);
    qDebug() << "QTcpClientDataLink: Server closed connection" << m_hostName << ":" << m_port;
}

void QTcpClientDataLink::onSendDataRequested(const QByteArray &data)
{
    sendLinkData(data);
}
