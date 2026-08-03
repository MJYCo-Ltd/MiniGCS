#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtQml>

#include "Link/QLinkManager.h"
#include "Link/QDataLink.h"
#include "QTestGCSConfig.h"
#include "QDroneControlManager.h"
#include "QGroundControlStation.h"
#include "Plat/QAutoVehicleType.h"
#include "Plat/QPlat.h"

static LinkKind linkKindFromString(const QString &type)
{
    if (type == LinkType::Serial) return LinkKind::Serial;
    if (type == LinkType::TcpServer) return LinkKind::TcpServer;
    if (type == LinkType::TcpClient) return LinkKind::TcpClient;
    if (type == LinkType::UdpServer) return LinkKind::UdpServer;
    if (type == LinkType::UdpClient) return LinkKind::UdpClient;
    return LinkKind::Raw;
}

static LinkParams linkParamsFromConfig(const QVariantMap &c, LinkKind kind)
{
    LinkParams p;
    switch (kind) {
    case LinkKind::TcpServer:
    case LinkKind::UdpServer:
        p.hostName = c.value(LinkConfigKeys::HostName).toString().trimmed();
        p.port = static_cast<quint16>(c.value(LinkConfigKeys::Port).toUInt());
        break;
    case LinkKind::TcpClient:
    case LinkKind::UdpClient:
        p.hostName = c.value(LinkConfigKeys::HostName).toString().trimmed();
        p.port = static_cast<quint16>(c.value(LinkConfigKeys::Port).toUInt());
        break;
    case LinkKind::Serial:
        p.portName = c.value(LinkConfigKeys::PortName).toString().trimmed();
        p.baudRate = c.value(LinkConfigKeys::BaudRate).toInt();
        break;
    default:
        break;
    }
    return p;
}

static void addLinksFromConfig(QGroundControlStation *pGroundStation)
{
    auto *config = QTestGCSConfig::instance();
    auto *linkManager = pGroundStation->linkManager();

    for (int i = 0; i < config->linkCount(); ++i) {
        QVariantMap c = config->linkConfigAt(i);
        QString typeStr = c.value(LinkConfigKeys::Type).toString();
        LinkKind kind = linkKindFromString(typeStr);
        if (kind == LinkKind::Raw) continue;

        LinkParams params = linkParamsFromConfig(c, kind);
        QDataLink *link = linkManager->addLink(kind, params);
        if (link) {
            qDebug() << "链路添加成功:" << typeStr;
            // 可设置重连: link->setReconnectCount(5); link->setAutoReconnect(true);
        }
    }
}

int main(int argc, char *argv[]) {
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);

    QGuiApplication app(argc, argv);

    QTestGCSConfig::instance()->init();

    QGroundControlStation *pGroundStation = new QGroundControlStation;

    pGroundStation->Init();
    addLinksFromConfig(pGroundStation);

    QObject::connect(
        pGroundStation, &QGroundControlStation::newPlatFind, [](QPlat *vehicle) {
            qDebug() << "新飞控对象创建: vehicleId=" << vehicle->vehicleId();

            QObject::connect(vehicle, &QPlat::connectionStatusChanged,
                             [vehicle](bool bIsConnected) {
                                 qDebug() << (bIsConnected
                                                  ? "飞控已连接:"
                                                  : "飞控失去连接:")
                                          << "vehicleId="
                                          << vehicle->vehicleId();
                             });

            QObject::connect(vehicle, &QPlat::infoUpdated, [vehicle]() {
                qDebug() << "飞控信息更新: vehicleId="
                         << vehicle->vehicleId()
                         << "firmware=" << vehicle->getFirmwareVersion()
                         << "software=" << vehicle->getSoftwareVersion();
            });
            QObject::connect(vehicle, &QPlat::errorInfo,
                             [](const QString &sErrorInfo) {
                                 qDebug() << "异常消息：" << sErrorInfo;
                             });
        });

    QObject::connect(pGroundStation->linkManager(), &QLinkManager::linkCreateFailed,
                     [](const QString &reason) {
                         qWarning() << "链路创建失败:" << reason;
                     });

    qmlRegisterType<QGroundControlStation>("MiniGCS", 1, 0, "GroundControlStation");
    qmlRegisterType<QPlat>("MiniGCS", 1, 0, "Plat");
    qmlRegisterUncreatableMetaObject(
        QAutoVehicleType::staticMetaObject,
        "MiniGCS", 1, 0, "AutoVehicleType",
        "AutoVehicleType only provides vehicle and autopilot enums");
    auto *droneControl =
        new QDroneControlManager(pGroundStation, &app);
    qmlRegisterSingletonInstance(
        "MiniGCS", 1, 0, "DroneControl", droneControl);
    qmlRegisterSingletonInstance(
        "MiniGCS", 1, 0, "AppConfig", QTestGCSConfig::instance());

    int exitCode = 0;
    {
        QQmlApplicationEngine engine;
        QObject::connect(
            &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
            []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
        engine.loadFromModule("MiniGCS", "Main");
        exitCode = app.exec();
    }

    // QML 已完全销毁后再释放其单例及 MAVSDK，避免退出信号处理中
    // 同步销毁连接线程。
    delete droneControl;
    delete pGroundStation;
    QTestGCSConfig::instance()->release();

    return exitCode;
}
