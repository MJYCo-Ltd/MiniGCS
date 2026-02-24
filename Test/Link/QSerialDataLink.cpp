#include <QSerialPort>
#include <QDebug>
#include "Link/QSerialDataLink.h"

QSerialDataLink::QSerialDataLink(const QString &portName, int baudRate, QObject *parent)
    : QDataLink(parent)
    , m_serialPort(nullptr)
    , m_portName(portName)
    , m_baudRate(baudRate)
{
}

/// 更改信息
void QSerialDataLink::changeInfo(const QString &portName, int baudRate)
{
    if(m_portName != portName || m_baudRate != baudRate){
        m_portName = portName;
        m_baudRate = baudRate;
        emit needReLink();
    }
}

bool QSerialDataLink::connectLink()
{
    // 如果还没有创建 QSerialPort，现在创建（确保在正确的线程中）
    if (!m_serialPort) {
        m_serialPort = new QSerialPort(this);

        // 连接信号槽
        connect(m_serialPort, &QSerialPort::readyRead, this,
                &QSerialDataLink::onReadyRead);
        connect(m_serialPort,
                QOverload<QSerialPort::SerialPortError>::of(
                    &QSerialPort::errorOccurred),
                this, &QSerialDataLink::onErrorOccurred);
    }

    if (m_serialPort->isOpen()) {
        return true;
    }

    // 设置串口参数
    m_serialPort->setPortName(m_portName);
    m_serialPort->setBaudRate(m_baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    // 打开串口
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        emit linkStatusChanged(true);
        return (true);
    } else {
        QString errorMsg = QString("Failed to open serial port %1: %2")
                               .arg(m_portName)
                               .arg(m_serialPort->errorString());
        emit linkStatusChanged(false);
        emit linkError(errorMsg);
        return (false);
    }
}

bool QSerialDataLink::sendLinkData(const QByteArray &data)
{
    if (!isLinkConnected()) {
        return false;
    }

    if (data.isEmpty()) {
        return true;
    }

    qint64 bytesWritten = m_serialPort->write(data);
    if (bytesWritten == -1) {
        QString errorMsg = QString("Failed to write data to serial port: %1")
        .arg(m_serialPort->errorString());
        emit linkError(errorMsg);
        return false;
    }

    if (bytesWritten != data.size()) {
        qWarning() << "QSerialDataLink: Partial write:" << bytesWritten 
                   << "of" << data.size() << "bytes";
    }

    // 确保数据被写入
    if (!m_serialPort->waitForBytesWritten(1000)) {
        return false;
    }

    return true;
}

bool QSerialDataLink::isLinkConnected() const
{
    return m_serialPort && m_serialPort->isOpen();
}

void QSerialDataLink::onReadyRead()
{
    if (!m_serialPort) {
        return;
    }

    // 读取所有可用数据
    QByteArray data = m_serialPort->readAll();
    
    if (!data.isEmpty()) {
        // 发射接收到的数据信号
        emit messageReceived(data);
    }
}

void QSerialDataLink::onErrorOccurred()
{
    if (!m_serialPort) {
        return;
    }

    QSerialPort::SerialPortError error = m_serialPort->error();
    if (error == QSerialPort::NoError) {
        return;
    }

    QString errorMsg;
    switch (error) {
    case QSerialPort::DeviceNotFoundError:
        errorMsg = "Serial port device not found";
        break;
    case QSerialPort::PermissionError:
        errorMsg = "Permission denied to open serial port";
        break;
    case QSerialPort::OpenError:
        errorMsg = "Serial port already open or cannot be opened";
        break;
    case QSerialPort::WriteError:
        errorMsg = "Error writing to serial port";
        break;
    case QSerialPort::ReadError:
        errorMsg = "Error reading from serial port";
        break;
    case QSerialPort::ResourceError:
        errorMsg = "Serial port resource error (device removed)";
        break;
    case QSerialPort::UnsupportedOperationError:
        errorMsg = "Unsupported operation on serial port";
        break;
    case QSerialPort::TimeoutError:
        errorMsg = "Serial port operation timeout";
        break;
    default:
        errorMsg = QString("Unknown serial port error: %1").arg(error);
        break;
    }
    emit linkError(errorMsg);

    // 如果是严重错误，关闭连接
    if (error == QSerialPort::ResourceError || error == QSerialPort::DeviceNotFoundError) {
        disConnectLink();
    }
}

void QSerialDataLink::onSendDataRequested(const QByteArray &data)
{
    // 在正确的线程中调用 sendLinkData
    sendLinkData(data);
}

void QSerialDataLink::closeOpenSerialPort()
{
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
        emit linkStatusChanged(false);
    }
}

