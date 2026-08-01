#ifndef _YTY_QNEDPOSITION_H
#define _YTY_QNEDPOSITION_H

#include <QObject>
#include <QMetaType>
#include "MiniGCSExport.h"

/**
 * @brief NED 局部位移（北、东、地，单位米）
 */
class MINIGCS_EXPORT QNEDPosition
{
    Q_GADGET
    Q_PROPERTY(float north READ north WRITE setNorth)
    Q_PROPERTY(float east READ east WRITE setEast)
    Q_PROPERTY(float down READ down WRITE setDown)

public:
    QNEDPosition();
    QNEDPosition(float north, float east, float down);

    float north() const { return m_north; }
    void setNorth(float north);

    float east() const { return m_east; }
    void setEast(float east);

    float down() const { return m_down; }
    void setDown(float down);

    bool operator==(const QNEDPosition &other) const;
    bool operator!=(const QNEDPosition &other) const;

private:
    float m_north{0.0f};
    float m_east{0.0f};
    float m_down{0.0f};
};

Q_DECLARE_METATYPE(QNEDPosition)
Q_DECLARE_METATYPE(QList<QNEDPosition>)

#endif // _YTY_QNEDPOSITION_H
