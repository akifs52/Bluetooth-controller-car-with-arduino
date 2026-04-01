#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "joypad.h"
#include "circularslider.h"
#include "battery.h"
#include <QMainWindow>
#include <QListWidgetItem>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QKeyEvent>
#include <QTimer>


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

    void on_pushButton_clicked();

    void sendContinuousCommand();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Ui::MainWindow *ui;

    QBluetoothDeviceDiscoveryAgent *discoveryAgent;

    QBluetoothSocket *socket;

    joypad *joypadWidget;

    CircularSlider *normalSlider = new CircularSlider;

    CircularSlider *turnSlider = new CircularSlider;

    bool isControlling = false; // Slider'dan sadece aktif kontrol anında veri gönder
    bool keyIsPressed = false; // Klavye tuşunun basılı tutulup tutulmadığını takip et
    QTimer *keyRepeatTimer; // Tuş tekrarını önlemek için timer
    Qt::Key lastKey = Qt::Key_unknown; // Son basılan tuşu takip et
    QString lastJoystickDirection = ""; // Son joystick yönünü takip et
    QString lastButtonDirection = ""; // Son buton yönünü takip et

    void parseBatteryData(const QString &data); // Arduino'dan gelen batarya verisini işle
    void updateBluetoothStatus(bool connected); // Bluetooth bağlantı durumunu güncelle


};
#endif // MAINWINDOW_H
