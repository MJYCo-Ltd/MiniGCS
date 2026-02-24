#include <QUdpSocket>
#include <QHostAddress>
#include <QHostInfo>
#include <QDebug>
#include "Link/QUdpClientDataLink.h"

QUdpClientDataLink::QUdpClientDataLink(const QString &hostName, quint16 port, QObject *parent)
    : QDataLink(parent)
    , m_udpSocket(nullptr)
    , m_hostName(hostName)
    , m_port(port)
{
}

QUdpClientDataLink::~QUdpClientDataLink()
{
}

bool QUdpClientDataLink::connectLink()
{
    if (!m_udpSocket) {
        m_udpSocket = new QUdpSocket(this);
        connect(m_udpSocket, &QUdpSocket::readyRead, this, &QUdpClientDataLink::onReadyRead);
    }

    if (m_udpSocket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    if (m_udpSocket->state() != QAbstractSocket::UnconnectedState) {
        m_udpSocket->close();
    }

    // 解析主机名
    QHostAddress addr(m_hostName);
    if (addr.isNull()) {
        QHostInfo info = QHostInfo::fromName(m_hostName);
        if (!info.addresses().isEmpty()) {
            addr = info.addresses().first();
        } else {
            QString errorMsg = QString("QUdpClientDataLink: Cannot resolve host %1").arg(m_hostName);
            emit linkError(errorMsg);
            emit linkStatusChanged(false);
            return false;
        }
    }
    m_remoteAddress = addr;

    if (!m_udpSocket->bind()) {
        QString errorMsg = QString("QUdpClientDataLink: Failed to bind: %1").arg(m_udpSocket->errorString());
        emit linkError(errorMsg);
        emit linkStatusChanged(false);
        return false;
    }

    m_udpSocket->connectToHost(m_remoteAddress, m_port);
    if (m_udpSocket->waitForConnected(3000)) {
        emit linkStatusChanged(true);
        qDebug() << "QUdpClientDataLink: Connected to" << m_hostName << ":" << m_port;
        return true;
    }

    QString errorMsg = QString("QUdpClientDataLink: Failed to connect to %1:%2: %3")
                           .arg(m_hostName).arg(m_port).arg(m_udpSocket->errorString());
    emit linkError(errorMsg);
    emit linkStatusChanged(false);
    return false;
}

void QUdpClientDataLink::disConnectLink()
{
    if (m_udpSocket) {
        m_udpSocket->disconnectFromHost();
        if (m_udpSocket->state() != QAbstractSocket::UnconnectedState) {
            m_udpSocket->waitForDisconnected(1000);
        }
        m_udpSocket->close();
        emit linkStatusChanged(false);
        qDebug() << "QUdpClientDataLink: Disconnected from" << m_hostName << ":" << m_port;
    }
}

bool QUdpClientDataLink::sendLinkData(const QByteArray &data)
{
    if (!isLinkConnected() || !m_udpSocket) {
        return false;
    }
    if (data.isEmpty()) {
        return true;
    }

    qint64 bytesWritten = m_udpSocket->write(data);
    if (bytesWritten == -1) {
        emit linkError(QString("QUdpClientDataLink: Failed to write: %1").arg(m_udpSocket->errorString()));
        return false;
    }
    if (bytesWritten != data.size()) {
        qWarning() << "QUdpClientDataLink: Partial write" << bytesWritten << "of" << data.size();
    }
    return true;
}

bool QUdpClientDataLink::isLinkConnected() const
{
    return m_udpSocket && m_udpSocket->state() == QAbstractSocket::ConnectedState;
}

void QUdpClientDataLink::onReadyRead()
{
    if (!m_udpSocket) return;
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(m_udpSocket->pendingDatagramSize()));
        qint64 size = m_udpSocket->readDatagram(datagram.data(), datagram.size());
        if (size > 0) {
            datagram.resize(static_cast<int>(size));
            emit messageReceived(datagram);
        }
    }
}

void QUdpClientDataLink::onSendDataRequested(const QByteArray &data)
{
    sendLinkData(data);
}
