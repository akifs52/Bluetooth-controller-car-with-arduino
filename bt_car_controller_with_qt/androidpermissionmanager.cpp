#include "androidpermissionmanager.h"
#include <QDebug>

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QPermissions>
#include <QTimer>
#endif

AndroidPermissionManager::AndroidPermissionManager(QObject *parent) : QObject(parent)
{
}

void AndroidPermissionManager::requestBluetoothPermissions()
{
#ifdef Q_OS_ANDROID
    QBluetoothPermission bluetoothPermission;
    bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);

    const auto status = qApp->checkPermission(bluetoothPermission);
    if (status == Qt::PermissionStatus::Granted) {
        emit permissionGranted(QStringLiteral("bluetooth"));
        requestLocationPermission();
        return;
    }

    if (status == Qt::PermissionStatus::Denied) {
        emit permissionDenied(QStringLiteral("bluetooth"));
        requestLocationPermission();
        return;
    }

    qApp->requestPermission(bluetoothPermission, this, [this](const QPermission &permission) {
        if (permission.status() == Qt::PermissionStatus::Granted) {
            emit permissionGranted(QStringLiteral("bluetooth"));
        } else {
            emit permissionDenied(QStringLiteral("bluetooth"));
        }
        requestLocationPermission();
    });
#endif
}

void AndroidPermissionManager::requestLocationPermission()
{
#ifdef Q_OS_ANDROID
    QLocationPermission locationPermission;
    locationPermission.setAccuracy(QLocationPermission::Precise);
    locationPermission.setAvailability(QLocationPermission::WhenInUse);

    const auto status = qApp->checkPermission(locationPermission);
    if (status == Qt::PermissionStatus::Granted) {
        emit permissionGranted(QStringLiteral("location"));
        return;
    }

    if (status == Qt::PermissionStatus::Denied) {
        emit permissionDenied(QStringLiteral("location"));
        return;
    }

    qApp->requestPermission(locationPermission, this, [this](const QPermission &permission) {
        if (permission.status() == Qt::PermissionStatus::Granted) {
            emit permissionGranted(QStringLiteral("location"));
        } else {
            emit permissionDenied(QStringLiteral("location"));
        }
    });
#endif
}

bool AndroidPermissionManager::hasBluetoothPermission()
{
#ifdef Q_OS_ANDROID
    QBluetoothPermission bluetoothPermission;
    bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);
    return qApp->checkPermission(bluetoothPermission) == Qt::PermissionStatus::Granted;
#else
    return true;
#endif
}

void AndroidPermissionManager::requestAllPermissions()
{
#ifdef Q_OS_ANDROID
    QTimer::singleShot(0, this, [this]() {
        requestBluetoothPermissions();
    });
#endif
}
