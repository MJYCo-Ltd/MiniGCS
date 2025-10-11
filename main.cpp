#include <iostream>
#include <fstream>
#include <thread>
#include <QCoreApplication>
#include <QDebug>

#include "Extern/XmlToMavSDK.h"
#include "QGroundStation.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    std::ofstream log_file("mavsdk.log");
    std::streambuf* cout_buf = std::cout.rdbuf(); // 保存原始缓冲区
    std::cout.rdbuf(log_file.rdbuf());            // 重定向

    // 创建QGroundStation实例
    QGroundStation groundStation;
    
    // 连接信号槽
    QObject::connect(&groundStation, &QGroundStation::vehicleConnected,
                     [](uint8_t systemId) {
                         qDebug() << "系统已连接，ID:" << systemId;
                     });
    
    QObject::connect(&groundStation, &QGroundStation::vehicleDisconnected,
                     [](uint8_t systemId) {
                         qDebug() << "系统已断开，ID:" << systemId;
                     });
    
    QObject::connect(&groundStation, &QGroundStation::connectionError, 
                     [](const QString &error) {
                         qWarning() << "连接错误:" << error;
                     });
    
    QObject::connect(&groundStation, &QGroundStation::vehicleInfoUpdated, 
                     [](const VehicleInfo &info) {
                         qDebug() << "飞控信息更新:";
                         qDebug() << "  系统ID:" << info.systemId;
                         qDebug() << "  自动驾驶仪类型:" << info.autopilotType;
                         qDebug() << "  载具类型:" << info.vehicleType;
                         qDebug() << "  是否连接:" << info.isConnected;
                         qDebug() << "  是否有相机:" << info.hasCamera;
                         qDebug() << "  组件数量:" << info.componentIds.size();
                     });

    // 尝试连接飞控（串口连接示例）
    qDebug() << "尝试连接飞控...";
    bool connected = groundStation.connectToVehicle(ConnectionType::Serial, "COM11:57600");
    
    if (!connected) {
        qWarning() << "连接失败，尝试TCP连接...";
        // 尝试TCP连接
        connected = groundStation.connectToVehicle(ConnectionType::TCP, "localhost:14550");
    }
    
    if (!connected) {
        qWarning() << "连接失败，尝试UDP连接...";
        // 尝试UDP连接
        connected = groundStation.connectToVehicle(ConnectionType::UDP, "14550");
    }

    if (connected) {
        qDebug() << "连接成功！";
        
        // 等待系统连接
        int attempts = 0;
        while (!groundStation.isConnected() && attempts < 50) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            QCoreApplication::processEvents();
            attempts++;
        }
        
        if (groundStation.isConnected()) {
            qDebug() << "飞控已连接，系统数量:" << groundStation.getSystemCount();
            
            // 获取所有飞控信息
            auto vehicleInfos = groundStation.getAllVehicleInfo();
            for (const auto &info : vehicleInfos) {
                qDebug() << "飞控信息:";
                qDebug() << "  系统ID:" << info.systemId;
                qDebug() << "  自动驾驶仪类型:" << info.autopilotType;
                qDebug() << "  载具类型:" << info.vehicleType;
                qDebug() << "  固件版本:" << info.firmwareVersion;
                qDebug() << "  硬件版本:" << info.hardwareVersion;
                qDebug() << "  软件版本:" << info.softwareVersion;
                qDebug() << "  是否连接:" << info.isConnected;
                qDebug() << "  是否有相机:" << info.hasCamera;
                qDebug() << "  组件ID列表:" << info.componentIds;
            }
            
            // 获取系统句柄用于XmlToMavSDK
            auto systemIds = groundStation.getConnectedSystemIds();
            if (!systemIds.isEmpty()) {
                auto systemHandle = groundStation.getSystemHandle(systemIds.first());
                if (systemHandle) {
                    // 使用XmlToMavSDK
                    XmlToMavSDK testXML("ardupilotmega.xml");
                    qDebug() << "可用命令:" << testXML.listCmdNames() << "数量:" << testXML.listCmdNames().size();
                    
                    // 将void*转换为std::shared_ptr<mavsdk::System>
                    auto system = std::shared_ptr<mavsdk::System>(
                        static_cast<mavsdk::System*>(systemHandle), 
                        [](mavsdk::System*){} // 空删除器，因为QGroundStation管理生命周期
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
