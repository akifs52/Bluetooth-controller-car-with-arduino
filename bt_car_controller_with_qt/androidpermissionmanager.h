#ifndef ANDROIDPERMISSIONMANAGER_H
#define ANDROIDPERMISSIONMANAGER_H

#include <QObject>
#include <QString>

class AndroidPermissionManager : public QObject
{
    Q_OBJECT
public:
    explicit AndroidPermissionManager(QObject *parent = nullptr);

    void requestBluetoothPermissions();
    void requestAllPermissions();
    bool hasBluetoothPermission();

signals:
    void permissionGranted(const QString &permission);
    void permissionDenied(const QString &permission);

private:
    void requestLocationPermission();
};

#endif // ANDROIDPERMISSIONMANAGER_H
