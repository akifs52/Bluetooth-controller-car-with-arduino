#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "joypad.h"
#include "circularslider.h"
#include "battery.h"
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceInfo>
#include <QBluetoothServiceDiscoveryAgent>
#include <QTimer>
#include <QPropertyAnimation>
#include <QEasingCurve>

// Android iÃ§in titreÅŸim desteÄŸi
#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <qnativeinterface.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(new QBluetoothSocket)
    , joypadWidget(new joypad)
    , normalSlider(new CircularSlider)
    , turnSlider(new CircularSlider)
{
    ui->setupUi(this);

    ui->DisconnectBt->hide();

    ui->verticalLayout_6->addWidget(joypadWidget);

    ui->verticalLayout_7->addWidget(normalSlider);

    ui->verticalLayout_8->addWidget(turnSlider);

    turnSlider->setProgressColor(Qt::blue);

    // Timer'Ä± baÅŸlat
    keyRepeatTimer = new QTimer(this);
    keyRepeatTimer->setInterval(50); // 50ms arayla komut gÃ¶nder
    connect(keyRepeatTimer, &QTimer::timeout, this, &MainWindow::sendContinuousCommand);

    connect(joypadWidget, &joypad::directionPressed,
            this, &MainWindow::handleJoypadDirection);

    connect(normalSlider, &CircularSlider::valueChanged,this, &MainWindow::handleNormalSpeed);

    connect(turnSlider, &CircularSlider::valueChanged, this, &MainWindow::handleTurnSpeed);
}

// Android titreÅŸim fonksiyonu
void MainWindow::triggerHapticFeedback()
{
#ifdef Q_OS_ANDROID
    const int durationMs = 50;

    const QJniObject context = QNativeInterface::QAndroidApplication::context();
    if (!context.isValid())
        return;

    const QJniObject serviceName = QJniObject::getStaticObjectField(
        "android/content/Context",
        "VIBRATOR_SERVICE",
        "Ljava/lang/String;");
    if (!serviceName.isValid())
        return;

    const QJniObject vibrator = context.callObjectMethod(
        "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;",
        serviceName.object<jstring>());
    if (!vibrator.isValid())
        return;

    const jboolean hasVibrator = vibrator.callMethod<jboolean>("hasVibrator");
    if (!hasVibrator)
        return;

    const int sdkVersion = QNativeInterface::QAndroidApplication::sdkVersion();
    if (sdkVersion >= 26) {
        const QJniObject effect = QJniObject::callStaticObjectMethod(
            "android/os/VibrationEffect",
            "createOneShot",
            "(JI)Landroid/os/VibrationEffect;",
            (jlong)durationMs,
            (jint)-1);
        if (effect.isValid()) {
            vibrator.callMethod<void>(
                "vibrate",
                "(Landroid/os/VibrationEffect;)V",
                effect.object());
            return;
        }
    }

    vibrator.callMethod<void>("vibrate", "(J)V", (jlong)durationMs);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_connectBt_clicked()
{

    ui->BtListWidget->clear();

    if(ui->stackedWidget->currentWidget() == ui->controlPage)
    {
        QWidget *fromPage = ui->stackedWidget->currentWidget();
        QWidget *toPage = ui->BtListPage;
        animatePageTransition(fromPage, toPage);
    }

    // Bluetooth tarayÄ±cÄ±yÄ± baÅŸlat
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &MainWindow::cihazBulundu);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &MainWindow::aramaTamamlandi);

    discoveryAgent->start();
    ui->BtListWidget->addItem("Cihazlar taranÄ±yor...");
}

void MainWindow::cihazBulundu(const QBluetoothDeviceInfo &info)
{
    QString ad = info.name().isEmpty() ? "Bilinmeyen Cihaz" : info.name();
    QString adres = info.address().toString();

    if (info.isValid()) {
        QListWidgetItem *item = new QListWidgetItem(ad + " [" + adres + "]");
        item->setData(Qt::UserRole, adres);
        ui->BtListWidget->addItem(item);
    }
}

void MainWindow::aramaTamamlandi()
{
    // Ä°lk satÄ±r "Cihazlar taranÄ±yor..." idi, onu temizle
    if (ui->BtListWidget->item(0)->text() == "Cihazlar taranıyor...") {
        delete ui->BtListWidget->takeItem(0);
    }

    if (ui->BtListWidget->count() == 0) {
        ui->BtListWidget->addItem("eşleştirilmiş cihaz bulunamadı.");
    }
}


void MainWindow::onSocketError(QBluetoothSocket::SocketError error)
{
    qDebug() << "Bluetooth baÄŸlantÄ± hatasÄ±:" << error;
}


void MainWindow::on_BtListWidget_itemClicked(QListWidgetItem *item)
{
    QString deviceAddress = item->data(Qt::UserRole).toString();
    qDebug() << "[DEBUG] SeÃ§ilen cihaz adresi:" << deviceAddress;

    if (deviceAddress.isEmpty()) {
        qDebug() << "[ERROR] SeÃ§ilen cihazÄ±n MAC adresi boÅŸ!";
        return;
    }


    // Yeni soket oluÅŸtur
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    qDebug() << "[DEBUG] Yeni QBluetoothSocket oluÅŸturuldu.";

    qDebug() << "[DEBUG] Sinyaller baÄŸlanÄ±yor...";
    connect(socket, &QBluetoothSocket::connected, this, [this]() {
        qDebug() << "Bluetooth cihazÄ±na baÄŸlanÄ±ldÄ±.";

        ui->connectBt->hide();

        ui->DisconnectBt->show();

        updateBluetoothStatus(true);

    });

    connect(socket, &QBluetoothSocket::disconnected, this, [this]() {
        qDebug() << "Bluetooth baÄŸlantÄ±sÄ± kesildi.";
        updateBluetoothStatus(false);
    });

    // Socket'tan veri okuma
    connect(socket, &QBluetoothSocket::readyRead, this, [this]() {
        QByteArray data = socket->readAll();
        QString receivedData = QString::fromUtf8(data).trimmed();
        
        // BoÅŸ veriyi ignore et
        if (receivedData.isEmpty()) return;
        
        // Batarya verisini kontrol et
        if (receivedData.contains("VOLT:") && receivedData.contains("BATT:")) {
            parseBatteryData(receivedData);
        }
        
        // Debug iÃ§in sadece ilk 50 karakteri gÃ¶ster (Ã§ok log olmamasÄ± iÃ§in)
        QString debugData = receivedData.length() > 50 ? 
                         receivedData.left(47) + "..." : 
                         receivedData;
        qDebug() << "[DATA] Gelen:" << debugData;
    });



    QBluetoothAddress targetAddress(deviceAddress);
    QBluetoothServiceDiscoveryAgent *discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);

    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
            [this, targetAddress, discoveryAgent](const QBluetoothServiceInfo &service) {
                qDebug() << "Bulunan servis:" << service.serviceName()
                << "UUID:" << service.serviceUuid().toString();

                // Sadece RFCOMM protokolÃ¼nÃ¼ kullanan servisleri filtrele
                if (service.socketProtocol() == QBluetoothServiceInfo::RfcommProtocol) {
                    qDebug() << "RFCOMM servisi bulundu, baÄŸlanÄ±lÄ±yor...";
                    this->socket->connectToService(targetAddress, service.serviceUuid());
                    discoveryAgent->stop();  // BaÄŸlandÄ±ktan sonra keÅŸfi durdur
                }
            });

    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished,
            [discoveryAgent]() {
                qDebug() << "Servis keÅŸfi tamamlandÄ±";
                discoveryAgent->deleteLater();
            });

    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled,
            [discoveryAgent]() {
                qDebug() << "Servis keÅŸfi iptal edildi";
                discoveryAgent->deleteLater();
            });

    // KeÅŸfi baÅŸlat
    discoveryAgent->setRemoteAddress(targetAddress);
    discoveryAgent->start();


}



void MainWindow::on_forwardButton_pressed()
{
    if (socket && socket->isOpen()) {
        isControlling = true;
        lastButtonDirection = "F";
        keyRepeatTimer->start(100);
        triggerHapticFeedback(); // Android titreÅŸim
        qDebug() << "[FORWARD] Button pressed - F";
    }
}

void MainWindow::on_forwardButton_released()
{
    if (socket && socket->isOpen()) {
        isControlling = false;
        lastButtonDirection = "";
        keyRepeatTimer->stop();
        socket->write("S\n");
        qDebug() << "[FORWARD] Button released - STOP";
    }
}

void MainWindow::on_backButton_pressed()
{
    if (socket && socket->isOpen()) {
        isControlling = true;
        lastButtonDirection = "B";
        keyRepeatTimer->start(100);
        triggerHapticFeedback(); // Android titreÅŸim
        qDebug() << "[BACKWARD] Button pressed - B";
    }
}

void MainWindow::on_backButton_released()
{
    if (socket && socket->isOpen()) {
        isControlling = false;
        lastButtonDirection = "";
        keyRepeatTimer->stop();
        socket->write("S\n");
        qDebug() << "[BACKWARD] Button released - STOP";
    }
}

void MainWindow::on_rightButton_pressed()
{
    if (socket && socket->isOpen()) {
        isControlling = true;
        lastButtonDirection = "R";
        keyRepeatTimer->start(100);
        triggerHapticFeedback(); // Android titreÅŸim
        qDebug() << "[RIGHT] Button pressed - R";
    }
}

void MainWindow::on_rightButton_released()
{
    if (socket && socket->isOpen()) {
        isControlling = false;
        lastButtonDirection = "";
        keyRepeatTimer->stop();
        socket->write("S\n");
        qDebug() << "[RIGHT] Button released - STOP";
    }
}

void MainWindow::on_leftButton_pressed()
{
    if (socket && socket->isOpen()) {
        isControlling = true;
        lastButtonDirection = "L";
        keyRepeatTimer->start(100);
        triggerHapticFeedback(); // Android titreÅŸim
        qDebug() << "[LEFT] Button pressed - L";
    }
}

void MainWindow::on_leftButton_released()
{
    if (socket && socket->isOpen()) {
        isControlling = false;
        lastButtonDirection = "";
        keyRepeatTimer->stop();
        socket->write("S\n");
        qDebug() << "[LEFT] Button released - STOP";
    }
}

void MainWindow::handleJoypadDirection(const QString &direction)
{
    if (!socket || !socket->isOpen()) {
        qDebug() << "[ERROR] Bluetooth soketi aÃ§Ä±k deÄŸil.";
        return;
    }

    static QString lastDirection = "";
    if (lastDirection != direction) {
        triggerHapticFeedback(); // Android titreÅŸim (sadece yÃ¶n deÄŸiÅŸiminde)
        lastDirection = direction;
    }

    QByteArray komut;

    if (direction == "U") { komut = "F\n"; isControlling = true; }
    else if (direction == "D") { komut = "B\n"; isControlling = true; }
    else if (direction == "L") { komut = "L\n"; isControlling = true; }
    else if (direction == "R") { komut = "R\n"; isControlling = true; }
    else { komut = "S\n"; isControlling = false; }

    socket->write(komut);
    qDebug() << "[Joypad] Komut gÃ¶nderildi:" << komut;

    qDebug() << "[Joypad] YÃ¶n deÄŸiÅŸtirildi:" << direction;
}


void MainWindow::handleNormalSpeed(int value)
{
    if (isControlling) return;

    if (!socket || !socket->isOpen()) {
        qDebug() << "[ERROR] Bluetooth soketi yok/aÃ§Ä±k deÄŸil.";
        return;
    }

    qDebug() << "[Normal Speed]:" << value;
    socket->write(QString("N%1\n").arg(value).toUtf8());
    triggerHapticFeedback(); // Android titreÅŸim
}

void MainWindow::handleTurnSpeed(int value)
{
    if (isControlling) return;

    if (!socket || !socket->isOpen()) {
        qDebug() << "[ERROR] Bluetooth soketi yok/aÃ§Ä±k deÄŸil.";
        return;
    }

    qDebug() << "[Turning Speed]:" << value;
    socket->write(QString("T%1\n").arg(value).toUtf8());
    triggerHapticFeedback(); // Android titreÅŸim
}



void MainWindow::on_DisconnectBt_clicked()
{
    if (socket && socket->isOpen()) {
        socket->disconnectFromService();
        qDebug() << "[INFO] Bluetooth baÄŸlantÄ±sÄ± kullanÄ±cÄ± tarafÄ±ndan kesildi.";

    } else {
        qDebug() << "[INFO] Zaten baÄŸlÄ± deÄŸil.";


    }

    ui->connectBt->show();
    ui->DisconnectBt->hide();
    updateBluetoothStatus(false);

}


void MainWindow::animatePageTransition(QWidget *fromPage, QWidget *toPage)
{
    int width = ui->stackedWidget->width();

    toPage->setGeometry(width, 0, width, ui->stackedWidget->height()); // yeni sayfayÄ± saÄŸdan baÅŸlat
    ui->stackedWidget->addWidget(toPage);
    toPage->show();

    QPropertyAnimation *animFrom = new QPropertyAnimation(fromPage, "geometry");
    animFrom->setDuration(300);
    animFrom->setStartValue(fromPage->geometry());
    animFrom->setEndValue(QRect(-width, 0, width, fromPage->height()));
    animFrom->setEasingCurve(QEasingCurve::InOutQuad);

    QPropertyAnimation *animTo = new QPropertyAnimation(toPage, "geometry");
    animTo->setDuration(300);
    animTo->setStartValue(toPage->geometry());
    animTo->setEndValue(QRect(0, 0, width, toPage->height()));
    animTo->setEasingCurve(QEasingCurve::InOutQuad);

    animFrom->start(QAbstractAnimation::DeleteWhenStopped);
    animTo->start(QAbstractAnimation::DeleteWhenStopped);

    // GeÃ§iÅŸ bitince stackedWidget'ta yeni sayfayÄ± set et
    connect(animTo, &QPropertyAnimation::finished, this, [=]() {
        ui->stackedWidget->setCurrentWidget(toPage);
        ui->stackedWidget->removeWidget(fromPage);
    });
}

void MainWindow::on_pushButton_clicked()
{
    if(ui->stackedWidget->currentWidget() == ui->BtListPage)
    {
        QWidget *fromPage = ui->stackedWidget->currentWidget();
        QWidget *toPage = ui->controlPage;
        animatePageTransition(fromPage, toPage);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return; // Auto-repeat'i engelle
    
    if (socket && socket->isOpen()) {
        switch (event->key()) {
            case Qt::Key_W:
            case Qt::Key_S:
            case Qt::Key_A:
            case Qt::Key_D:
                isControlling = true;
                keyIsPressed = true;
                lastKey = static_cast<Qt::Key>(event->key()); // Qt6 enum conversion fix
                keyRepeatTimer->start(50); // 50ms
                break;
            default:
                QMainWindow::keyPressEvent(event);
                break;
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return; // Auto-repeat'i engelle
    
    if (socket && socket->isOpen()) {
        switch (event->key()) {
            case Qt::Key_W:
            case Qt::Key_S:
            case Qt::Key_A:
            case Qt::Key_D:
                isControlling = false;
                keyIsPressed = false;
                keyRepeatTimer->stop();
                socket->write("S\n");
                qDebug() << "[KEYBOARD] Key released - STOP";
                break;
            default:
                QMainWindow::keyReleaseEvent(event);
                break;
        }
    } else {
        QMainWindow::keyReleaseEvent(event);
    }
}

void MainWindow::sendContinuousCommand()
{
    if (!isControlling || !socket || !socket->isOpen()) return;

    // Ã–ncelik sÄ±rasÄ±: Klavye > Joystick > Buton
    if (keyIsPressed && lastKey != Qt::Key_unknown) {
        switch (lastKey) {
            case Qt::Key_W: socket->write("F\n"); break;
            case Qt::Key_S: socket->write("B\n"); break;
            case Qt::Key_A: socket->write("L\n"); break;
            case Qt::Key_D: socket->write("R\n"); break;
            default: break;
        }
    }
    else if (!lastJoystickDirection.isEmpty()) {
        if (lastJoystickDirection == "U") socket->write("F\n");
        else if (lastJoystickDirection == "D") socket->write("B\n");
        else if (lastJoystickDirection == "L") socket->write("L\n");
        else if (lastJoystickDirection == "R") socket->write("R\n");
        else socket->write("S\n");
    }
    else if (!lastButtonDirection.isEmpty()) {
        if (lastButtonDirection == "F") socket->write("F\n");
        else if (lastButtonDirection == "B") socket->write("B\n");
        else if (lastButtonDirection == "L") socket->write("L\n");
        else if (lastButtonDirection == "R") socket->write("R\n");
    }
}

void MainWindow::parseBatteryData(const QString &data)
{
    // Format: "VOLT:8.25V|BATT:75%"
    // Birden fazla veri gelmiÅŸ olabilir, sonuncusunu al
    QStringList lines = data.split("\n", Qt::SkipEmptyParts);
    
    for (const QString &line : lines) {
        if (line.contains("VOLT:") && line.contains("BATT:")) {
            QStringList parts = line.split("|");
            
            if (parts.size() >= 2) {
                QString voltPart = parts[0].trimmed(); // "VOLT:8.25V"
                QString battPart = parts[1].trimmed(); // "BATT:75%"
                
                // VoltajÄ± Ã§Ä±kar ve doÄŸrula
                if (voltPart.startsWith("VOLT:") && voltPart.endsWith("V")) {
                    QString voltageStr = voltPart.mid(5, voltPart.length() - 6);
                    bool ok;
                    float voltage = voltageStr.toFloat(&ok);
                    
                    if (ok && voltage >= 0.0 && voltage <= 10.0) {
                        qDebug() << "[BATTERY] Voltaj:" << voltage << "V";
                        
                        // Åarj durumu tahmini (8.0V Ã¼zeri ÅŸarj olabilir)
                        bool isCharging = (voltage >= 8.0);
                        ui->batteryWidget->setCharging(isCharging);
                    }
                }
                
                // YÃ¼zdeyi Ã§Ä±kar ve doÄŸrula
                if (battPart.startsWith("BATT:") && battPart.endsWith("%")) {
                    QString percentageStr = battPart.mid(5, battPart.length() - 6);
                    bool ok;
                    int percentage = percentageStr.toInt(&ok);
                    
                    if (ok && percentage >= 0 && percentage <= 100) {
                        // Batarya widget'Ä±nÄ± gÃ¼ncelle
                        ui->batteryWidget->setBatteryLevel(percentage);
                        qDebug() << "[BATTERY] YÃ¼zde:" << percentage << "%";
                    }
                }
            }
        }
    }
}

void MainWindow::updateBluetoothStatus(bool connected)
{
    if (connected) {
        ui->connectionStatus->setPixmap(QPixmap(":/images/img/icons8-bluetooth-connected-50 (1).png"));
        qDebug() << "[INFO] Bluetooth durumu: BaÄŸlÄ±";
    } else {
        ui->connectionStatus->setPixmap(QPixmap(":/images/img/icons8-bluetooth-50.png"));
        qDebug() << "[INFO] Bluetooth durumu: BaÄŸlÄ± deÄŸil";
    }
}

