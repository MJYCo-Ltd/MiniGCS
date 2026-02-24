#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtQml>

#include "Link/QSerialDataLink.h"
#include "Plat/QAutopilot.h"
#include "QTestGCSConfig.h"
#include "QGroundControlStation.h"
#include "Plat/QPlat.h"

int main(int argc, char *argv[]) {
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);

    QGuiApplication app(argc, argv);

    QTestGCSConfig::instance()->init();
    auto oldHandler = qInstallMessageHandler(&QGCSConfig::qtLogHandler);

    // 创建QGroundControlStation实例
    QGroundControlStation *pGroundStation = new QGroundControlStation;

    // 尝试连接飞控（串口连接示例）
    pGroundStation->Init();
    // 创建 QSerialDataLink 时指定 parent，确保在正确的线程中
    QSerialDataLink *pSerialDataLink = new QSerialDataLink(
        QTestGCSConfig::instance()->defaultPortName(),
        QTestGCSConfig::instance()->defaultBaudRate(), pGroundStation);
    pGroundStation->AddDataLink(pSerialDataLink);

    // 连接新飞控对象创建信号
    QObject::connect(
        pGroundStation, &QGroundControlStation::newPlatFind, [](QPlat *vehicle) {
            qDebug() << " 新飞控对象创建:" << vehicle->toString();

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
            QObject::connect(vehicle,&QAutopilot::errorInfo,[](const QString& sErrorInfo){
                qDebug()<< "异常消息："<<sErrorInfo;
            });
        });

    // aboutToQuit 在主线程同步执行，用于做必要的善后工作（短且快速）
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        qInstallMessageHandler(oldHandler);
        // 尽量在这里执行快速、确定性的清理，避免耗时阻塞
        // 顺序必须
        delete pGroundStation;
        pGroundStation = nullptr;

        QTestGCSConfig::instance()->release();
    });

    // 注册 QML 类型
    qmlRegisterType<QGroundControlStation>("MiniGCS", 1, 0, "GroundControlStation");
    qmlRegisterType<QPlat>("MiniGCS", 1, 0, "Plat");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("MiniGCS", "Main");

    return app.exec();
}

