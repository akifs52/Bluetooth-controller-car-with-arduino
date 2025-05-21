#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceInfo>
#include <QBluetoothServiceDiscoveryAgent>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->connectedButton->hide();

    ui->DisconnectBt->hide();
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
        ui->stackedWidget->setCurrentWidget(ui->BtListPage);
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
        ui->stackedWidget->setCurrentWidget(ui->controlPage);
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
    connect(socket, &QBluetoothSocket::connected, this, []() {
        qDebug() << "Bluetooth cihazına bağlanıldı.";
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

    ui->disconnectedButton->hide();

    ui->connectedButton->show();

}



void MainWindow::on_forwardButton_pressed()
{
    if (socket && socket->isOpen()) {
        QByteArray ileriKomutu = "F"; // F → Forward (ileri) komutu
        socket->write(ileriKomutu);
        qDebug() << "[INFO] İleri komutu gönderildi: F";
    } else {
        qDebug() << "[ERROR] Bluetooth soketi açık değil, komut gönderilemedi.";
    }
}

void MainWindow::on_forwardButton_released()
{
    if (socket && socket->isOpen()) {
        QByteArray ileriKomutu = "S"; // F → Forward (ileri) komutu
        socket->write(ileriKomutu);
        qDebug() << "[INFO] İleri komutu gönderildi: S";
    } else {
        qDebug() << "[ERROR] Bluetooth soketi açık değil, komut gönderilemedi.";
    }
}



void MainWindow::on_backButton_pressed()
{
    if (socket && socket->isOpen()) {
        QByteArray ileriKomutu = "B"; // F → Forward (ileri) komutu
        socket->write(ileriKomutu);
        qDebug() << "[INFO] İleri komutu gönderildi: B";
    } else {
        qDebug() << "[ERROR] Bluetooth soketi açık değil, komut gönderilemedi.";
    }
}


void MainWindow::on_backButton_released()
{
    if (socket && socket->isOpen()) {
        QByteArray ileriKomutu = "S"; // F → Forward (ileri) komutu
        socket->write(ileriKomutu);
        qDebug() << "[INFO] İleri komutu gönderildi: S";
    } else {
        qDebug() << "[ERROR] Bluetooth soketi açık değil, komut gönderilemedi.";
    }
}


void MainWindow::on_rightButton_pressed()
{
    if (socket && socket->isOpen()) {
        QByteArray ileriKomutu = "R"; // F → Forward (ileri) komutu
        socket->write(ileriKomutu);
        qDebug() << "[INFO] İleri komutu gönderildi: R";
    } else {
        qDebug() << "[ERROR] Bluetooth soketi açık değil, komut gönderilemedi.";
    }
}

void MainWindow::on_rightButton_released()
{
    if (socket && socket->isOpen()) {
        QByteArray ileriKomutu = "S"; // F → Forward (ileri) komutu
        socket->write(ileriKomutu);
        qDebug() << "[INFO] İleri komutu gönderildi: S";
    } else {
        qDebug() << "[ERROR] Bluetooth soketi açık değil, komut gönderilemedi.";
    }
}


void MainWindow::on_leftButton_pressed()
{
    if (socket && socket->isOpen()) {
        QByteArray ileriKomutu = "L"; // F → Forward (ileri) komutu
        socket->write(ileriKomutu);
        qDebug() << "[INFO] İleri komutu gönderildi: L";
    } else {
        qDebug() << "[ERROR] Bluetooth soketi açık değil, komut gönderilemedi.";
    }
}


void MainWindow::on_leftButton_released()
{
    if (socket && socket->isOpen()) {
        QByteArray ileriKomutu = "S"; // F → Forward (ileri) komutu
        socket->write(ileriKomutu);
        qDebug() << "[INFO] İleri komutu gönderildi: S";
    } else {
        qDebug() << "[ERROR] Bluetooth soketi açık değil, komut gönderilemedi.";
    }
}
void MainWindow::on_stopButton_clicked()
{
    if (socket && socket->isOpen()) {
        QByteArray ileriKomutu = "S"; // F → Forward (ileri) komutu
        socket->write(ileriKomutu);
        qDebug() << "[INFO] İleri komutu gönderildi: S";
    } else {
        qDebug() << "[ERROR] Bluetooth soketi açık değil, komut gönderilemedi.";
    }
}













