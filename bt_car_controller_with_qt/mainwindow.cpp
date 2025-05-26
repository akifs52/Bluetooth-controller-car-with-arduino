#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceInfo>
#include <QBluetoothServiceDiscoveryAgent>
#include <QTimer>
#include <QPropertyAnimation>
#include <QEasingCurve>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(new QBluetoothSocket)
    , joypadWidget(new joypad)
    , normalSlider(new CircularSlider)
    , turnSlider(new CircularSlider)
{
    ui->setupUi(this);

    ui->connectedButton->hide();

    ui->DisconnectBt->hide();

    ui->verticalLayout_6->addWidget(joypadWidget);

    ui->verticalLayout_7->addWidget(normalSlider);

    ui->verticalLayout_8->addWidget(turnSlider);

    turnSlider->setProgressColor(Qt::blue);

    connect(joypadWidget, &joypad::directionPressed,
            this, &MainWindow::handleJoypadDirection);

    connect(normalSlider, &CircularSlider::valueChanged,this, &MainWindow::handleNormalSpeed);

    connect(turnSlider, &CircularSlider::valueChanged, this, &MainWindow::handleTurnSpeed);
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

    // Bluetooth tarayıcıyı başlat
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &MainWindow::cihazBulundu);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &MainWindow::aramaTamamlandi);

    discoveryAgent->start();
    ui->BtListWidget->addItem("Cihazlar taranıyor...");
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
    // İlk satır "Cihazlar taranıyor..." idi, onu temizle
    if (ui->BtListWidget->item(0)->text() == "Cihazlar taranıyor...") {
        delete ui->BtListWidget->takeItem(0);
    }

    if (ui->BtListWidget->count() == 0) {
        ui->BtListWidget->addItem("Eşleştirilmiş cihaz bulunamadı.");
    }
}

void MainWindow::on_ControlButton_clicked()
{
    if(ui->stackedWidget->currentWidget() == ui->BtListPage)
    {
        QWidget *fromPage = ui->stackedWidget->currentWidget();
        QWidget *toPage = ui->controlPage;
        animatePageTransition(fromPage, toPage);

    }

}

void MainWindow::onSocketError(QBluetoothSocket::SocketError error)
{
    qDebug() << "Bluetooth bağlantı hatası:" << error;
}


void MainWindow::on_BtListWidget_itemClicked(QListWidgetItem *item)
{
    QString deviceAddress = item->data(Qt::UserRole).toString();
    qDebug() << "[DEBUG] Seçilen cihaz adresi:" << deviceAddress;

    if (deviceAddress.isEmpty()) {
        qDebug() << "[ERROR] Seçilen cihazın MAC adresi boş!";
        return;
    }


    // Yeni soket oluştur
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    qDebug() << "[DEBUG] Yeni QBluetoothSocket oluşturuldu.";

    qDebug() << "[DEBUG] Sinyaller bağlanıyor...";
    connect(socket, &QBluetoothSocket::connected, this, [this]() {
        qDebug() << "Bluetooth cihazına bağlanıldı.";

        ui->connectBt->hide();

        ui->DisconnectBt->show();

        ui->disconnectedButton->hide();

        ui->connectedButton->show();
    });

    connect(socket, &QBluetoothSocket::disconnected, this, []() {
        qDebug() << "Bluetooth bağlantısı kesildi.";
    });



    QBluetoothAddress targetAddress(deviceAddress);
    QBluetoothServiceDiscoveryAgent *discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);

    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
            [this, targetAddress, discoveryAgent](const QBluetoothServiceInfo &service) {
                qDebug() << "Bulunan servis:" << service.serviceName()
                << "UUID:" << service.serviceUuid().toString();

                // Sadece RFCOMM protokolünü kullanan servisleri filtrele
                if (service.socketProtocol() == QBluetoothServiceInfo::RfcommProtocol) {
                    qDebug() << "RFCOMM servisi bulundu, bağlanılıyor...";
                    this->socket->connectToService(targetAddress, service.serviceUuid());
                    discoveryAgent->stop();  // Bağlandıktan sonra keşfi durdur
                }
            });

    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished,
            [discoveryAgent]() {
                qDebug() << "Servis keşfi tamamlandı";
                discoveryAgent->deleteLater();
            });

    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled,
            [discoveryAgent]() {
                qDebug() << "Servis keşfi iptal edildi";
                discoveryAgent->deleteLater();
            });

    // Keşfi başlat
    discoveryAgent->setRemoteAddress(targetAddress);
    discoveryAgent->start();


}



void MainWindow::on_forwardButton_pressed()
{
    if (socket && socket->isOpen()) {
        isControlling = true;
        socket->write("F\n");
        qDebug() << "[FORWARD] Gönderildi: F";
    }
}

void MainWindow::on_forwardButton_released()
{
    if (socket && socket->isOpen()) {
        isControlling = false;
        socket->write("S\n");
        qDebug() << "[STOP] Gönderildi: S";
    }
}

void MainWindow::on_backButton_pressed()
{
    if (socket && socket->isOpen()) {
        isControlling = true;
        socket->write("B\n");
        qDebug() << "[BACKWARD] Gönderildi: B";
    }
}

void MainWindow::on_backButton_released()
{
    if (socket && socket->isOpen()) {
        isControlling = false;
        socket->write("S\n");
        qDebug() << "[STOP] Gönderildi: S";
    }
}

void MainWindow::on_rightButton_pressed()
{
    if (socket && socket->isOpen()) {
        isControlling = true;
        socket->write("R\n");
        qDebug() << "[RIGHT] Gönderildi: R";
    }
}

void MainWindow::on_rightButton_released()
{
    if (socket && socket->isOpen()) {
        isControlling = false;
        socket->write("S\n");
        qDebug() << "[STOP] Gönderildi: S";
    }
}

void MainWindow::on_leftButton_pressed()
{
    if (socket && socket->isOpen()) {
        isControlling = true;
        socket->write("L\n");
        qDebug() << "[LEFT] Gönderildi: L";
    }
}

void MainWindow::on_leftButton_released()
{
    if (socket && socket->isOpen()) {
        isControlling = false;
        socket->write("S\n");
        qDebug() << "[STOP] Gönderildi: S";
    }
}

void MainWindow::handleJoypadDirection(const QString &direction)
{

    if (!socket || !socket->isOpen()) {
        qDebug() << "[ERROR] Bluetooth soketi açık değil.";
        return;
    }

    QByteArray komut;

    if (direction == "U") { komut = "F\n"; isControlling = true; }
    else if (direction == "D") { komut = "B\n"; isControlling = true; }
    else if (direction == "L") { komut = "L\n"; isControlling = true; }
    else if (direction == "R") { komut = "R\n"; isControlling = true; }
    else { komut = "S\n"; isControlling = false; }

    socket->write(komut);
    qDebug() << "[Joypad] Komut gönderildi:" << komut;
}


void MainWindow::handleNormalSpeed(int value)
{
    if (isControlling) return;

    if (!socket || !socket->isOpen()) {
        qDebug() << "[ERROR] Bluetooth soketi yok/açık değil.";
        return;
    }

    qDebug() << "[Normal Speed]:" << value;
    socket->write(QString("N%1\n").arg(value).toUtf8());
}

void MainWindow::handleTurnSpeed(int value)
{
    if (isControlling) return;

    if (!socket || !socket->isOpen()) {
        qDebug() << "[ERROR] Bluetooth soketi yok/açık değil.";
        return;
    }

    qDebug() << "[Turning Speed]:" << value;
    socket->write(QString("T%1\n").arg(value).toUtf8());
}



void MainWindow::on_DisconnectBt_clicked()
{
    if (socket && socket->isOpen()) {
        socket->disconnectFromService();
        qDebug() << "[INFO] Bluetooth bağlantısı kullanıcı tarafından kesildi.";

    } else {
        qDebug() << "[INFO] Zaten bağlı değil.";


    }

    ui->connectBt->show();
    ui->DisconnectBt->hide();
    ui->connectedButton->hide();
    ui->disconnectedButton->show();
}


void MainWindow::animatePageTransition(QWidget *fromPage, QWidget *toPage)
{
    int width = ui->stackedWidget->width();

    toPage->setGeometry(width, 0, width, ui->stackedWidget->height()); // yeni sayfayı sağdan başlat
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

    // Geçiş bitince stackedWidget'ta yeni sayfayı set et
    connect(animTo, &QPropertyAnimation::finished, this, [=]() {
        ui->stackedWidget->setCurrentWidget(toPage);
        ui->stackedWidget->removeWidget(fromPage);
    });
}
