#ifndef _YTY_QDRONEPOSITION_H
#define _YTY_QDRONEPOSITION_H

#include <QObject>
#include <QMetaType>

/**
 * @brief 无人机位置信息结构体
 * 
 * 包含经度、纬度、高度以及NED坐标系下的位置信息
 */
class QDronePosition
{
    Q_GADGET
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude)
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude)
    Q_PROPERTY(double altitude READ altitude WRITE setAltitude)
    Q_PROPERTY(double nedX READ nedX WRITE setNedX)
    Q_PROPERTY(double nedY READ nedY WRITE setNedY)
    Q_PROPERTY(double nedZ READ nedZ WRITE setNedZ)

public:
    QDronePosition();
    QDronePosition(double longitude, double latitude, double altitude, 
                   double nedX = 0.0, double nedY = 0.0, double nedZ = 0.0);
    
    double longitude() const { return m_longitude; }
    void setLongitude(double longitude);
    
    double latitude() const { return m_latitude; }
    void setLatitude(double latitude);
    
    double altitude() const { return m_altitude; }
    void setAltitude(double altitude);
    
    double nedX() const { return m_nedX; }
    void setNedX(double nedX);
    
    double nedY() const { return m_nedY; }
    void setNedY(double nedY);
    
    double nedZ() const { return m_nedZ; }
    void setNedZ(double nedZ);
    
    bool operator==(const QDronePosition &other) const;
    bool operator!=(const QDronePosition &other) const;

private:
    double m_longitude = 0.0;  ///< 经度
    double m_latitude = 0.0;   ///< 纬度
    double m_altitude = 0.0;    ///< 高度
    double m_nedX = 0.0;        ///< NED坐标系X
    double m_nedY = 0.0;        ///< NED坐标系Y
    double m_nedZ = 0.0;        ///< NED坐标系Z
};

Q_DECLARE_METATYPE(QDronePosition)
Q_DECLARE_METATYPE(QList<QDronePosition>)

#endif // _YTY_QDRONEPOSITION_H

