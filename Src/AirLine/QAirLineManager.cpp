#include <QDebug>
#include "AirLine/QAirLineManager.h"

QAirLineManager::QAirLineManager(QObject *parent)
    : QObject(parent)
{
}

QAirLineManager::~QAirLineManager()
{
    // 航线均以本对象为父对象，由 QObject 统一销毁。
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
    connect(airline, &QObject::destroyed, this, [this, airline]() {
        if (m_airlines.removeOne(airline)) {
            emit airlinesChanged();
        }
    });
    emit airlineAdded(airline);
    emit airlinesChanged();
    return true;
}

QAirLine* QAirLineManager::createAirLine(const QString &name)
{
    QAirLine *airline = new QAirLine(name);
    addAirLine(airline);
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
    airline->deleteLater();
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
    airline->deleteLater();
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

    const QList<QAirLine *> airlinesToRemove = m_airlines;
    m_airlines.clear();

    for (QAirLine *airline : airlinesToRemove) {
        emit airlineRemoved(airline);
        airline->deleteLater();
    }

    emit airlinesChanged();
}

