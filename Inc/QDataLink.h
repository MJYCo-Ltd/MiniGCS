#ifndef QDATALINK_H
#define QDATALINK_H

#include <QObject>
#include <QByteArray>

/**
 * @brief QDataLink类 - 数据链路基类
 * 
 * 该类定义了数据链路的基本接口，包括接收消息的信号和发送数据的方法
 */
class QDataLink : public QObject
{
    Q_OBJECT

public:
    explicit QDataLink(QObject *parent = nullptr):QObject(parent){}
    virtual ~QDataLink(){}

    /**
     * @brief 发送数据
     * @param data 要发送的数据
     * @return 是否发送成功
     */
    virtual bool sendData(const QByteArray &data) = 0;

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    virtual bool isConnected() const = 0;

signals:
    /**
     * @brief 接收到原始数据信号
     * @param data 接收到的原始数据
     */
    void messageReceived(const QByteArray &data);
};

#endif // QDATALINK_H

