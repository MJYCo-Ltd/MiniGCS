#include <iostream>
#include <fstream>
#include <thread>
#include <QCoreApplication>
#include <QDebug>

#include "Extern/XmlToMavSDK.h"
#include "QGroundControlStation.h"
#include "QVehicle.h"
#include "QDataLink.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    std::ofstream log_file("mavsdk.log");
    std::streambuf* cout_buf = std::cout.rdbuf(); // 保存原始缓冲区
    std::cout.rdbuf(log_file.rdbuf());            // 重定向

    // 创建QGroundControlStation实例
    QGroundControlStation groundStation;
    
    // 尝试连接飞控（串口连接示例）
    qDebug() << "尝试连接飞控...";
    QDataLink* dataLink = groundStation.createDataLink(ConnectionType::Serial, "COM11", 57600);
    dataLink->connectToDataLink();
    
    if (!dataLink->isConnected()) {
        qWarning() << "串口连接失败，尝试TCP连接...";
        // 尝试TCP连接
        dataLink = groundStation.createDataLink(ConnectionType::TCP, "localhost", 14550);
        dataLink->connectToDataLink();
    }
    
    if (!dataLink->isConnected()) {
        qWarning() << "TCP连接失败，尝试UDP连接...";
        // 尝试UDP连接
        dataLink = groundStation.createDataLink(ConnectionType::UDP, "localhost", 14550);
        dataLink->connectToDataLink();
    }
    
    if (!dataLink->isConnected()) {
        qWarning() << "所有连接方式都失败了";
        return 0;
    }
    
    // 连接数据链路的信号
    QObject::connect(dataLink, &QDataLink::connectionError, 
                     [](const QString &error) {
                         qWarning() << "连接错误:" << error;
                     });
    
    // 连接新飞控对象创建信号
    QObject::connect(dataLink, &QDataLink::newVehicleFind,
                     [](QVehicle* vehicle) {
                         qDebug() << "新飞控对象创建:" << vehicle->toString();
                         
                         // 连接飞控对象的信号
                         QObject::connect(vehicle, &QVehicle::connected, [vehicle]() {
                             qDebug() << "飞控已连接:" << vehicle->toString();
                         });
                         
                         QObject::connect(vehicle, &QVehicle::disconnected, [vehicle](const QString &reason) {
                             qDebug() << "飞控已断开:" << vehicle->toString() << "原因:" << reason;
                         });
                         
                         QObject::connect(vehicle, &QVehicle::communicationLost, [vehicle](const QString &error) {
                             qWarning() << "飞控通信链路断开:" << vehicle->toString() << "错误:" << error;
                         });
                         
                         QObject::connect(vehicle, &QVehicle::infoUpdated, [vehicle]() {
                             qDebug() << "飞控信息更新:" << vehicle->toString();
                         });
                     });

    qDebug() << "连接成功！";
    
    // 等待系统连接
    int attempts = 0;
    while (!dataLink->isConnected() && attempts < 50) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        QCoreApplication::processEvents();
        attempts++;
    }
    
    if (dataLink->isConnected()) {
        qDebug() << "飞控已连接，系统数量:" << dataLink->getVehicleCount();
        
        // 获取所有飞控对象
        auto vehicles = dataLink->getAllVehicles();
        for (auto vehicle : vehicles) {
            qDebug() << "飞控信息:" << vehicle->toString();
        }

        return app.exec();
    } else {
        qWarning() << "等待连接超时";
    }

    return 0;
}
