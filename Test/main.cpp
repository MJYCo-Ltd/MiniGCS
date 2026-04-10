#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtQml>

#include "Link/QLinkManager.h"
#include "Link/QDataLink.h"
#include "QTestGCSConfig.h"
#include "QGroundControlStation.h"
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
        p.port = c.value(LinkConfigKeys::Port).toUInt();
        break;
    case LinkKind::TcpClient:
    case LinkKind::UdpClient:
        p.hostName = c.value(LinkConfigKeys::HostName).toString();
        p.port = c.value(LinkConfigKeys::Port).toUInt();
        break;
    case LinkKind::Serial:
        p.portName = c.value(LinkConfigKeys::PortName).toString();
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
            qDebug() << " 新飞控对象创建:" << vehicle->toString();

            QObject::connect(vehicle, &QPlat::connectionStatusChanged,
                             [vehicle](bool bIsConnected) {
                                 if (bIsConnected)
                                     qDebug() << "飞控已连接:" << vehicle->toString();
                                 else
                                     qDebug() << "飞控失去连接:" << vehicle->toString();
                             });

            QObject::connect(vehicle, &QPlat::infoUpdated, [vehicle]() {
                qDebug() << "飞控信息更新:" << vehicle->toString();
            });
            QObject::connect(vehicle,&QPlat::errorInfo,[](const QString& sErrorInfo){
                qDebug()<< "异常消息："<<sErrorInfo;
            });
        });

    QObject::connect(pGroundStation->linkManager(), &QLinkManager::linkCreateFailed,
                     [](const QString &reason) {
                         qWarning() << "链路创建失败:" << reason;
                     });

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        delete pGroundStation;
        pGroundStation = nullptr;

        QTestGCSConfig::instance()->release();
    });

    qmlRegisterType<QGroundControlStation>("MiniGCS", 1, 0, "GroundControlStation");
    qmlRegisterType<QPlat>("MiniGCS", 1, 0, "Plat");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("MiniGCS", "Main");

    return app.exec();
}
