#include "mainwindow.h"
#include "androidpermissionmanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Android için izinleri otomatik iste
#ifdef Q_OS_ANDROID
    AndroidPermissionManager permissionManager;
    permissionManager.requestAllPermissions();
#endif
    
    MainWindow w;
    w.show();
    return a.exec();
}
