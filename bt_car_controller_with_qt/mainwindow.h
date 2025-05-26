#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "joypad.h"
#include "circularslider.h"
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

    void on_forwardButton_pressed();

    void on_forwardButton_released();

    void on_rightButton_released();

    void on_backButton_released();

    void on_leftButton_released();

    void handleJoypadDirection(const QString &direction);

    void handleNormalSpeed(int value);

    void handleTurnSpeed(int value);

    void on_DisconnectBt_clicked();

    void animatePageTransition(QWidget *fromPage, QWidget *toPage);

private:
    Ui::MainWindow *ui;

    QBluetoothDeviceDiscoveryAgent *discoveryAgent;

    QBluetoothSocket *socket;

    joypad *joypadWidget;

    CircularSlider *normalSlider = new CircularSlider;

    CircularSlider *turnSlider = new CircularSlider;

    bool isControlling = false; // Slider'dan sadece aktif kontrol anında veri gönder


};
#endif // MAINWINDOW_H
