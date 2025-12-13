#ifndef _YTY_QSTANDALONE_H
#define _YTY_QSTANDALONE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <memory>

class QPlatPrivate;
/**
 * @brief QPlat类 - 平台类
 * 
 * 该类封装了平台的一些版本信息等
 */
class QPlat : public QObject
{
    Q_OBJECT

public:
    explicit QPlat(QObject *parent = nullptr);
    ~QPlat();

    /**
     * @brief 获取固件版本
     * @return 固件版本
     */
    QString getFirmwareVersion() const;

    /**
     * @brief 获取硬件版本
     * @return 硬件版本
     */
    QString getHardwareVersion() const;

    /**
     * @brief 获取软件版本
     * @return 软件版本
     */
    QString getSoftwareVersion() const;

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool isConnected() const;

    /**
     * @brief 获取最后连接时间
     * @return 最后连接时间
     */
    QDateTime getLastConnectedTime() const;

    /**
     * @brief 设置最后连接时间
     * @param time 最后连接时间
     */
    void setLastConnectedTime(const QDateTime &time);

    /**
     * @brief 获取最后断开时间
     * @return 最后断开时间
     */
    QDateTime getLastDisconnectedTime() const;

    /**
     * @brief 设置最后断开时间
     * @param time 最后断开时间
     */
    void setLastDisconnectedTime(const QDateTime &time);

    /**
     * @brief 转换为字符串表示
     * @return 字符串表示
     */
    QString toString() const;

    /**
     * @brief 检查是否有相机
     * @return 是否有相机
     */
    bool hasCamera() const;

signals:

    /**
     * @brief 连接状态变化信号
     * @param connected 是否连接
     */
    void connectionStatusChanged(bool connected);

    /**
     * @brief 信息更新信号
     */
    void infoUpdated();
protected slots:
    void updateConnection(bool bConnected);
protected:
    void SetPrivate(QPlatPrivate* pPlatPrivate);

protected:
    friend class QGroundControlStationPrivate;
    QDateTime m_lastConnectedTime;          ///< 最后连接时间
    QDateTime m_lastDisconnectedTime;       ///< 最后断开时间
    bool      m_bConnected{true};
    std::unique_ptr<QPlatPrivate> d_ptr;    ///< 私有实现指针
};

#endif // _YTY_QSTANDALONE_H
