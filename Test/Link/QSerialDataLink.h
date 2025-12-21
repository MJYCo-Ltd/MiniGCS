#ifndef QSERIALDATALINK_H
#define QSERIALDATALINK_H

#include "Link/QDataLink.h"
#include <QString>

// 前向声明
class QSerialPort;

/**
 * @brief QSerialDataLink类 - 串口数据链路类
 * 
 * 该类继承自QDataLink，实现基于串口的数据通信
 */
class QSerialDataLink : public QDataLink
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param portName 串口名称（如 "COM1", "/dev/ttyUSB0"）
     * @param baudRate 波特率（如 57600, 115200）
     * @param parent 父对象
     */
    explicit QSerialDataLink(const QString &portName, int baudRate, QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    virtual ~QSerialDataLink();

    /**
     * @brief 打开串口连接
     * @return 是否打开成功
     */
    bool connectLink() override;

    /**
     * @brief 关闭串口连接
     */
    void disConnectLink() override{closeOpenSerialPort();}

    /**
     * @brief 发送数据
     * @param data 要发送的数据
     * @return 是否发送成功
     */
    bool sendLinkData(const QByteArray &data) override;

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isLinkConnected() const override;

    /**
     * @brief 获取串口名称
     * @return 串口名称
     */
    QString portName() const;

    /**
     * @brief 获取波特率
     * @return 波特率
     */
    int baudRate() const;

private slots:
    /**
     * @brief 处理串口数据接收
     */
    void onReadyRead();

    /**
     * @brief 处理串口错误
     */
    void onErrorOccurred();
    
    /**
     * @brief 线程安全的发送数据槽函数
     * @param data 要发送的数据
     */
    void onSendDataRequested(const QByteArray &data) override;
protected:
    void closeOpenSerialPort();
private:
    QSerialPort* m_serialPort;    ///< 串口对象
    QString m_portName;            ///< 串口名称
    int m_baudRate;                ///< 波特率
};

#endif // QSERIALDATALINK_H


