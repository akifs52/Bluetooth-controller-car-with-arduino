#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include <QQuickView>
#include <QQuickStyle>

#include "bluetoothmanager.h"
#include "androidpermissionmanager.h"

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    QQuickStyle::setFallbackStyle(QStringLiteral("Basic"));
    QGuiApplication app(argc, argv);

#ifdef Q_OS_ANDROID
    AndroidPermissionManager permissionManager;
    permissionManager.requestAllPermissions();
#endif

    BluetoothManager btManager;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("bluetoothManager"), &btManager);
    
    // Otomatik cihaz taraması başlat
    btManager.startDiscovery();

    const QUrl url(QStringLiteral("qrc:/qml_bt_car/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
