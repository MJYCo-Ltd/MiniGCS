#ifndef _YTY_QVELOCITY_H
#define _YTY_QVELOCITY_H

#include <QObject>
#include <QMetaType>
#include "MiniGCSExport.h"

/**
 * @brief 速度（NED 分量与水平/垂直标量，单位 m/s）
 */
class MINIGCS_EXPORT QVelocity
{
    Q_GADGET
    Q_PROPERTY(double northMS READ northMS WRITE setNorthMS)
    Q_PROPERTY(double eastMS READ eastMS WRITE setEastMS)
    Q_PROPERTY(double downMS READ downMS WRITE setDownMS)
    Q_PROPERTY(double groundSpeedMS READ groundSpeedMS WRITE setGroundSpeedMS)
    Q_PROPERTY(double verticalSpeedMS READ verticalSpeedMS WRITE setVerticalSpeedMS)

public:
    QVelocity() = default;
    QVelocity(double northMS, double eastMS, double downMS);

    double northMS() const { return m_northMS; }
    void setNorthMS(double northMS);

    double eastMS() const { return m_eastMS; }
    void setEastMS(double eastMS);

    double downMS() const { return m_downMS; }
    void setDownMS(double downMS);

    double groundSpeedMS() const { return m_groundSpeedMS; }
    void setGroundSpeedMS(double groundSpeedMS);

    double verticalSpeedMS() const { return m_verticalSpeedMS; }
    void setVerticalSpeedMS(double verticalSpeedMS);

    /** 由 NED 分量刷新水平/垂直标量 */
    void refreshScalars();

    bool operator==(const QVelocity &other) const;
    bool operator!=(const QVelocity &other) const;

private:
    double m_northMS{0.0};
    double m_eastMS{0.0};
    double m_downMS{0.0};
    double m_groundSpeedMS{0.0};
    double m_verticalSpeedMS{0.0};
};

Q_DECLARE_METATYPE(QVelocity)

#endif // _YTY_QVELOCITY_H
