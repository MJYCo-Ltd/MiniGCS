#include <QCoreApplication>
#include <QDebug>
#include <fstream>
#include <iostream>
#include <thread>

#include "Extern/XmlToMavSDK.h"
#include "QGroundControlStation.h"
#include "QSerialDataLink.h"
#include "QAutopilot.h"
#include "QGCSConfig.h"
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::ofstream log_file("mavsdk.log");
    std::streambuf *cout_buf = std::cout.rdbuf(); // 保存原始缓冲区
    std::cout.rdbuf(log_file.rdbuf());            // 重定向

    // 创建QGroundControlStation实例
    QGroundControlStation groundStation;

    // 尝试连接飞控（串口连接示例）
    groundStation.Init();
    // 创建 QSerialDataLink 时指定 parent，确保在正确的线程中
    QSerialDataLink *pSerialDataLink = new QSerialDataLink(
        QGCSConfig::instance().defaultPortName(),
        QGCSConfig::instance().defaultBaudRate(), &groundStation);
    groundStation.AddDataLink(pSerialDataLink);

    // 连接新飞控对象创建信号
    QObject::connect(
        &groundStation, &QGroundControlStation::newVehicleFind,
        [](QPlat *vehicle) {
            qDebug() << "新飞控对象创建:" << vehicle->toString();

            // 连接飞控对象的信号
            QObject::connect(vehicle, &QAutopilot::connectionStatusChanged,
                             [vehicle](bool bIsConnected) {
                                 if (bIsConnected)
                    qDebug() << "飞控已连接:" << vehicle->toString();
                                 else
                                     qDebug() << "飞控失去连接:" << vehicle->toString();
            });

            QObject::connect(vehicle, &QAutopilot::infoUpdated, [vehicle]() {
                qDebug() << "飞控信息更新:" << vehicle->toString();
            });
        });

    qDebug() << "连接成功！";

    return app.exec();
}
