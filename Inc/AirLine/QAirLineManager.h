#ifndef _YTY_QAIRLINEMANAGER_H
#define _YTY_QAIRLINEMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include "QAirLine.h"

/**
 * @brief 航线管理器类
 * 
 * 该类管理多条航线，提供添加、删除、查找等功能
 */
class QAirLineManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> airlines READ airlines NOTIFY airlinesChanged)
    Q_PROPERTY(int airlineCount READ airlineCount NOTIFY airlinesChanged)

public:
    explicit QAirLineManager(QObject *parent = nullptr);
    ~QAirLineManager();

    /**
     * @brief 获取所有航线列表
     * @return 航线列表
     */
    QList<QObject*> airlines() const;

    /**
     * @brief 获取航线数量
     * @return 航线数量
     */
    int airlineCount() const;

    /**
     * @brief 添加航线
     * @param airline 航线对象
     * @return 是否添加成功
     */
    Q_INVOKABLE bool addAirLine(QAirLine *airline);

    /**
     * @brief 创建并添加新航线
     * @param name 航线名称
     * @return 创建的航线对象
     */
    Q_INVOKABLE QAirLine* createAirLine(const QString &name = QString());

    /**
     * @brief 移除航线
     * @param airline 航线对象
     * @return 是否移除成功
     */
    Q_INVOKABLE bool removeAirLine(QAirLine *airline);

    /**
     * @brief 根据索引移除航线
     * @param index 索引
     * @return 是否移除成功
     */
    Q_INVOKABLE bool removeAirLineAt(int index);

    /**
     * @brief 根据名称查找航线
     * @param name 航线名称
     * @return 航线对象，如果未找到返回nullptr
     */
    Q_INVOKABLE QAirLine* findAirLineByName(const QString &name) const;

    /**
     * @brief 根据索引获取航线
     * @param index 索引
     * @return 航线对象，如果索引无效返回nullptr
     */
    Q_INVOKABLE QAirLine* getAirLineAt(int index) const;

    /**
     * @brief 清空所有航线
     */
    Q_INVOKABLE void clearAllAirlines();

signals:
    /**
     * @brief 航线列表变化信号
     */
    void airlinesChanged();

    /**
     * @brief 航线添加信号
     * @param airline 添加的航线
     */
    void airlineAdded(QAirLine *airline);

    /**
     * @brief 航线移除信号
     * @param airline 移除的航线
     */
    void airlineRemoved(QAirLine *airline);

private:
    QList<QAirLine*> m_airlines;    ///< 航线列表
};

#endif // _YTY_QAIRLINEMANAGER_H

