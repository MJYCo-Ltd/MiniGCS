#ifndef _YTY_QAIRLINE_H
#define _YTY_QAIRLINE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMetaType>
#include "QGpsPosition.h"
#include "MiniGCSExport.h"

/**
 * @brief 航线类
 * 
 * 该类表示一条航线，包含多个航点位置
 */
class MINIGCS_EXPORT QAirLine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QList<QGpsPosition> waypoints READ waypoints WRITE setWaypoints NOTIFY waypointsChanged)
    Q_PROPERTY(int waypointCount READ waypointCount NOTIFY waypointsChanged)

public:
    explicit QAirLine(QObject *parent = nullptr);
    explicit QAirLine(const QString &name, QObject *parent = nullptr);
    ~QAirLine();

    /**
     * @brief 获取航线名称
     * @return 航线名称
     */
    QString name() const;

    /**
     * @brief 设置航线名称
     * @param name 航线名称
     */
    void setName(const QString &name);

    /**
     * @brief 获取航点列表
     * @return 航点列表
     */
    QList<QGpsPosition> waypoints() const;

    /**
     * @brief 设置航点列表
     * @param waypoints 航点列表
     */
    void setWaypoints(const QList<QGpsPosition> &waypoints);

    /**
     * @brief 获取航点数量
     * @return 航点数量
     */
    int waypointCount() const;

    /**
     * @brief 添加航点
     * @param position 航点位置
     */
    Q_INVOKABLE void addWaypoint(const QGpsPosition &position);

    /**
     * @brief 在指定索引插入航点
     * @param index 索引
     * @param position 航点位置
     */
    Q_INVOKABLE void insertWaypoint(int index, const QGpsPosition &position);

    /**
     * @brief 移除指定索引的航点
     * @param index 索引
     */
    Q_INVOKABLE void removeWaypoint(int index);

    /**
     * @brief 获取指定索引的航点
     * @param index 索引
     * @return 航点位置
     */
    Q_INVOKABLE QGpsPosition getWaypoint(int index) const;

    /**
     * @brief 清空所有航点
     */
    Q_INVOKABLE void clearWaypoints();

signals:
    /**
     * @brief 航线名称变化信号
     * @param name 新的航线名称
     */
    void nameChanged(const QString &name);

    /**
     * @brief 航点列表变化信号
     */
    void waypointsChanged();

private:
    QString m_name;                          ///< 航线名称
    QList<QGpsPosition> m_waypoints;        ///< 航点列表
};

Q_DECLARE_METATYPE(QAirLine*)

#endif // _YTY_QAIRLINE_H

