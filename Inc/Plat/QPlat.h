#ifndef _YTY_QSTANDALONE_H
#define _YTY_QSTANDALONE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <memory>
#include "MiniGCSExport.h"

class QPlatPrivate;
class QGroundControlStation;
/**
 * @brief QPlat - 飞行平台基类
 *
 * 封装平台连接状态与版本信息。vehicleId 为业务侧平台标识。
 */
class MINIGCS_EXPORT QPlat : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString firmwareVersion READ getFirmwareVersion NOTIFY infoUpdated)
    Q_PROPERTY(QString softwareVersion READ getSoftwareVersion NOTIFY infoUpdated)
    Q_PROPERTY(int vehicleId READ vehicleId CONSTANT)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionStatusChanged)
    Q_PROPERTY(QDateTime lastConnectedTime READ getLastConnectedTime NOTIFY infoUpdated)
    Q_PROPERTY(QDateTime lastDisconnectedTime READ getLastDisconnectedTime NOTIFY infoUpdated)

public:
    explicit QPlat(QObject *parent = nullptr);
    ~QPlat();

    int vehicleId() const { return m_vehicleId; }
    /** @deprecated 使用 vehicleId() */
    int systemId() const { return vehicleId(); }

    QString getFirmwareVersion() const;
    QString getSoftwareVersion() const;
    bool isConnected() const;
    QDateTime getLastConnectedTime() const;
    void setLastConnectedTime(const QDateTime &time);
    QDateTime getLastDisconnectedTime() const;
    void setLastDisconnectedTime(const QDateTime &time);
    QString toString() const;

signals:
    void connectionStatusChanged(bool connected);
    void infoUpdated();
    /** 平台组件集合发生变化 */
    void componentsChanged();
    void errorInfo(const QString& sError);

protected slots:
    void updateConnection(bool bConnected);

protected:
    void SetPrivate(QPlatPrivate* pPlatPrivate);
    void setVehicleId(int vehicleId) { m_vehicleId = vehicleId; }
    /** @deprecated 使用 setVehicleId() */
    void setSystemId(int systemId) { setVehicleId(systemId); }

protected:
    friend class QGroundControlStationPrivate;
    friend class QGroundControlStation;
    QDateTime m_lastConnectedTime;
    QDateTime m_lastDisconnectedTime;
    bool      m_bConnected{false};
    int       m_vehicleId{-1};

    std::unique_ptr<QPlatPrivate> d_ptr;
};

#endif // _YTY_QSTANDALONE_H
