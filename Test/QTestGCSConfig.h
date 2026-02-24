#ifndef QTESTGCSCONFIG_H
#define QTESTGCSCONFIG_H

#include "QGCSConfig.h"

/**
 * @brief QTestGCSConfig - Test 工程配置类，继承 QGCSConfig
 *
 * 在基类基础上增加串口、地图等 Test 工程专用配置项
 */
class QTestGCSConfig : public QGCSConfig
{
    Q_OBJECT
public:
    static QTestGCSConfig *instance();

    Q_INVOKABLE QStringList refreshPortName() const;
    Q_INVOKABLE QStringList standardBaudRates() const;

    Q_INVOKABLE QString defaultPortName() const;
    Q_INVOKABLE void setDefaultPortName(const QString &portName);

    Q_INVOKABLE int defaultBaudRate() const;
    Q_INVOKABLE void setDefaultBaudRate(int baudRate);

    Q_INVOKABLE QString mapName() const;
    Q_INVOKABLE void setMapName(const QString &mapName);

    void release() override;

protected:
    void initializeDefaults() override;

private:
    QTestGCSConfig(QObject *parent = nullptr);
    ~QTestGCSConfig() override;
    Q_DISABLE_COPY(QTestGCSConfig)

    static QTestGCSConfig *s_instance;
};

#endif // QTESTGCSCONFIG_H
