#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectBt_clicked();

    void on_ControlButton_clicked();

    void on_BtListWidget_itemClicked(QListWidgetItem *item);

    void cihazBulundu(const QBluetoothDeviceInfo &info);

    void aramaTamamlandi();

    void onSocketError(QBluetoothSocket::SocketError error);

    void on_backButton_pressed();

    void on_rightButton_pressed();

    void on_leftButton_pressed();

    void on_stopButton_clicked();

    void on_forwardButton_pressed();

    void on_forwardButton_released();

    void on_rightButton_released();

    void on_backButton_released();

    void on_leftButton_released();

private:
    Ui::MainWindow *ui;

    QBluetoothDeviceDiscoveryAgent *discoveryAgent;

    QBluetoothSocket *socket;
};
#endif // MAINWINDOW_H
