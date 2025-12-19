#ifndef _YTY_QGPSPOSITION_H
#define _YTY_QGPSPOSITION_H

#include <QObject>
#include <QMetaType>

/**
 * @brief GPS位置信息结构体
 * 
 * 包含经度、纬度、高度信息
 */
class QGpsPosition
{
    Q_GADGET
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude)
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude)
    Q_PROPERTY(float altitude READ altitude WRITE setAltitude)

public:
    QGpsPosition();
    QGpsPosition(double longitude, double latitude, double altitude);
    
    double longitude() const { return m_longitude; }
    void setLongitude(double longitude);
    
    double latitude() const { return m_latitude; }
    void setLatitude(double latitude);
    
    float altitude() const { return m_altitude; }
    void setAltitude(float altitude);
    
    bool operator==(const QGpsPosition &other) const;
    bool operator!=(const QGpsPosition &other) const;

private:
    double m_longitude{0.0};  ///< 经度
    double m_latitude{0.0};   ///< 纬度
    float m_altitude{0.0};    ///< 高度
};

Q_DECLARE_METATYPE(QGpsPosition)
Q_DECLARE_METATYPE(QList<QGpsPosition>)

#endif // _YTY_QGPSPOSITION_H

