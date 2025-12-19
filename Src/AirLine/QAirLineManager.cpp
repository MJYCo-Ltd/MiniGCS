#include <QDebug>
#include "AirLine/QAirLineManager.h"

QAirLineManager::QAirLineManager(QObject *parent)
    : QObject(parent)
{
}

QAirLineManager::~QAirLineManager()
{
    // 清理所有航线
    qDeleteAll(m_airlines);
    m_airlines.clear();
}

QList<QObject*> QAirLineManager::airlines() const
{
    QList<QObject*> result;
    for (QAirLine *airline : m_airlines) {
        result.append(airline);
    }
    return result;
}

int QAirLineManager::airlineCount() const
{
    return m_airlines.size();
}

bool QAirLineManager::addAirLine(QAirLine *airline)
{
    if (!airline) {
        qWarning() << "QAirLineManager::addAirLine: 航线对象为空";
        return false;
    }

    if (m_airlines.contains(airline)) {
        qWarning() << "QAirLineManager::addAirLine: 航线已存在";
        return false;
    }

    // 设置父对象，确保生命周期管理
    airline->setParent(this);
    m_airlines.append(airline);
    emit airlineAdded(airline);
    emit airlinesChanged();
    return true;
}

QAirLine* QAirLineManager::createAirLine(const QString &name)
{
    QAirLine *airline = new QAirLine(name, this);
    m_airlines.append(airline);
    emit airlineAdded(airline);
    emit airlinesChanged();
    return airline;
}

bool QAirLineManager::removeAirLine(QAirLine *airline)
{
    if (!airline) {
        qWarning() << "QAirLineManager::removeAirLine: 航线对象为空";
        return false;
    }

    int index = m_airlines.indexOf(airline);
    if (index == -1) {
        qWarning() << "QAirLineManager::removeAirLine: 航线不存在";
        return false;
    }

    m_airlines.removeAt(index);
    emit airlineRemoved(airline);
    emit airlinesChanged();
    
    // 注意：不删除对象，因为可能在其他地方仍在使用
    // 如果需要立即删除，可以调用 airline->deleteLater()
    return true;
}

bool QAirLineManager::removeAirLineAt(int index)
{
    if (index < 0 || index >= m_airlines.size()) {
        qWarning() << "QAirLineManager::removeAirLineAt: 索引超出范围" << index;
        return false;
    }

    QAirLine *airline = m_airlines.at(index);
    m_airlines.removeAt(index);
    emit airlineRemoved(airline);
    emit airlinesChanged();
    return true;
}

QAirLine* QAirLineManager::findAirLineByName(const QString &name) const
{
    for (QAirLine *airline : m_airlines) {
        if (airline->name() == name) {
            return airline;
        }
    }
    return nullptr;
}

QAirLine* QAirLineManager::getAirLineAt(int index) const
{
    if (index < 0 || index >= m_airlines.size()) {
        qWarning() << "QAirLineManager::getAirLineAt: 索引超出范围" << index;
        return nullptr;
    }
    return m_airlines.at(index);
}

void QAirLineManager::clearAllAirlines()
{
    if (m_airlines.isEmpty()) {
        return;
    }

    // 发送移除信号
    for (QAirLine *airline : m_airlines) {
        emit airlineRemoved(airline);
    }

    m_airlines.clear();
    emit airlinesChanged();
}

