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
    
    // 创建数据链路
    QDataLink* dataLink = groundStation.createDataLink();
    
    // 连接数据链路的信号
    QObject::connect(dataLink, &QDataLink::connectionError, 
                     [](const QString &error) {
                         qWarning() << "连接错误:" << error;
                     });
    
    // 连接新飞控对象创建信号
    QObject::connect(dataLink, &QDataLink::vehicleCreated,
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

    // 尝试连接飞控（串口连接示例）
    qDebug() << "尝试连接飞控...";
    bool connected = dataLink->connectToDataLink(ConnectionType::Serial, "COM11", 57600);
    
    if (!connected) {
        qWarning() << "连接失败，尝试TCP连接...";
        // 尝试TCP连接
        connected = dataLink->connectToDataLink(ConnectionType::TCP, "localhost", 14550);
    }
    
    if (!connected) {
        qWarning() << "连接失败，尝试UDP连接...";
        // 尝试UDP连接
        connected = dataLink->connectToDataLink(ConnectionType::UDP, "localhost", 14550);
    }

    if (connected) {
        qDebug() << "连接成功！";
        
        // 等待系统连接
        int attempts = 0;
        while (!dataLink->isConnected() && attempts < 50) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            QCoreApplication::processEvents();
            attempts++;
        }
        
        if (dataLink->isConnected()) {
            qDebug() << "飞控已连接，系统数量:" << dataLink->getSystemCount();
            
            // 获取所有飞控对象
            auto vehicles = dataLink->getAllVehicles();
            for (auto vehicle : vehicles) {
                qDebug() << "飞控信息:" << vehicle->toString();
            }
            
            // 获取系统句柄用于XmlToMavSDK
            auto systemIds = dataLink->getVehicleIDs();
            if (!systemIds.isEmpty()) {
                auto systemHandle = dataLink->getSystemHandle(systemIds.first());
                if (systemHandle) {
                    // 使用XmlToMavSDK
                    XmlToMavSDK testXML("ardupilotmega.xml");
                    qDebug() << "可用命令:" << testXML.listCmdNames() << "数量:" << testXML.listCmdNames().size();
                    
                    // 将void*转换为std::shared_ptr<mavsdk::System>
                    auto system = std::shared_ptr<mavsdk::System>(
                        static_cast<mavsdk::System*>(systemHandle), 
                        [](mavsdk::System*){} // 空删除器，因为QDataLink管理生命周期
                    );
                    testXML.setSystem(system);
                    
                    qDebug() << "地面站运行中，按Ctrl+C退出...";
                    
                    // 运行事件循环
                    return app.exec();
                }
            }
        } else {
            qWarning() << "等待连接超时";
        }
    } else {
        qWarning() << "所有连接方式都失败了";
    }

    return 0;
}
