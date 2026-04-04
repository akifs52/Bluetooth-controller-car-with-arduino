#include "bluetoothmanager.h"

#include <QBluetoothDeviceInfo>
#include <QDebug>
#include <QRegularExpression>
#include <QStringList>
#include <QVariantMap>
#include <QtConcurrent/QtConcurrentRun>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2bth.h>
#include <bluetoothapis.h>
#endif

namespace {
#ifdef Q_OS_WIN
constexpr int kNativeReadIntervalMs = 50;

#ifdef SerialPortServiceClass_UUID
const GUID kSerialPortServiceUuid = SerialPortServiceClass_UUID;
#else
const GUID kSerialPortServiceUuid =
{ 0x00001101, 0x0000, 0x1000, { 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } };
#endif

QString formatWindowsBluetoothAddressLocal(quint64 rawAddress)
{
    BLUETOOTH_ADDRESS address;
    address.ullLong = static_cast<BTH_ADDR>(rawAddress);

    return QStringLiteral("%1:%2:%3:%4:%5:%6")
        .arg(address.rgBytes[5], 2, 16, QLatin1Char('0'))
        .arg(address.rgBytes[4], 2, 16, QLatin1Char('0'))
        .arg(address.rgBytes[3], 2, 16, QLatin1Char('0'))
        .arg(address.rgBytes[2], 2, 16, QLatin1Char('0'))
        .arg(address.rgBytes[1], 2, 16, QLatin1Char('0'))
        .arg(address.rgBytes[0], 2, 16, QLatin1Char('0'))
        .toUpper();
}

void upsertScannedDevice(QVariantList *devices, const QVariantMap &entry)
{
    if (!devices) {
        return;
    }

    for (int i = 0; i < devices->size(); ++i) {
        const QVariantMap existing = devices->at(i).toMap();
        if (existing.value(QStringLiteral("address")).toString()
            == entry.value(QStringLiteral("address")).toString()) {
            (*devices)[i] = entry;
            return;
        }
    }

    devices->append(entry);
}

int scanDevicesForRadio(HANDLE radioHandle, bool issueInquiry, QVariantList *outDevices)
{
    if (!outDevices) {
        return 0;
    }

    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams;
    ZeroMemory(&searchParams, sizeof(searchParams));
    searchParams.dwSize = sizeof(searchParams);
    searchParams.fReturnAuthenticated = TRUE;
    searchParams.fReturnRemembered = TRUE;
    searchParams.fReturnUnknown = TRUE;
    searchParams.fReturnConnected = TRUE;
    searchParams.fIssueInquiry = issueInquiry ? TRUE : FALSE;
    searchParams.cTimeoutMultiplier = issueInquiry ? 4 : 2;
    searchParams.hRadio = radioHandle;

    BLUETOOTH_DEVICE_INFO deviceInfo;
    ZeroMemory(&deviceInfo, sizeof(deviceInfo));
    deviceInfo.dwSize = sizeof(deviceInfo);

    int foundCount = 0;
    HBLUETOOTH_DEVICE_FIND findHandle = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
    if (!findHandle) {
        return 0;
    }

    do {
        const QString name = QString::fromWCharArray(deviceInfo.szName).trimmed();
        const QString address = formatWindowsBluetoothAddressLocal(deviceInfo.Address.ullLong);
        const int rssi = -1; // Win32 klasik API RSSI vermez, geçersiz değer.
        const bool paired = deviceInfo.fAuthenticated || deviceInfo.fRemembered;

        QVariantMap entry;
        entry.insert(QStringLiteral("name"), name.isEmpty() ? address : name);
        entry.insert(QStringLiteral("address"), address);
        entry.insert(QStringLiteral("rssi"), rssi);
        entry.insert(QStringLiteral("paired"), paired);

        upsertScannedDevice(outDevices, entry);
        ++foundCount;

        qDebug() << "[BT][WIN] Cihaz bulundu:"
                 << entry.value(QStringLiteral("name")).toString()
                 << entry.value(QStringLiteral("address")).toString()
                 << (entry.value(QStringLiteral("paired")).toBool() ? "[eşleşmiş]" : "[eşleşmemiş]");

        ZeroMemory(&deviceInfo, sizeof(deviceInfo));
        deviceInfo.dwSize = sizeof(deviceInfo);
    } while (BluetoothFindNextDevice(findHandle, &deviceInfo));

    BluetoothFindDeviceClose(findHandle);
    return foundCount;
}

QVariantList scanNativeWindowsDevices()
{
    QVariantList devices;
    int discoveredCount = 0;

    BLUETOOTH_FIND_RADIO_PARAMS radioSearchParams;
    ZeroMemory(&radioSearchParams, sizeof(radioSearchParams));
    radioSearchParams.dwSize = sizeof(radioSearchParams);

    HANDLE radioHandle = nullptr;
    HBLUETOOTH_RADIO_FIND radioFind = BluetoothFindFirstRadio(&radioSearchParams, &radioHandle);
    if (radioFind) {
        do {
            BLUETOOTH_RADIO_INFO radioInfo;
            ZeroMemory(&radioInfo, sizeof(radioInfo));
            radioInfo.dwSize = sizeof(radioInfo);
            if (BluetoothGetRadioInfo(radioHandle, &radioInfo) == ERROR_SUCCESS) {
                qDebug() << "[BT][WIN] Radio:"
                         << QString::fromWCharArray(radioInfo.szName).trimmed();
            }

            discoveredCount += scanDevicesForRadio(radioHandle, true, &devices);

            CloseHandle(radioHandle);
            radioHandle = nullptr;
        } while (BluetoothFindNextRadio(radioFind, &radioHandle));

        BluetoothFindRadioClose(radioFind);
    } else {
        qDebug() << "[BT][WIN] Bluetooth radio bulunamadı. GetLastError:" << GetLastError();
    }

    // Bazı sistemlerde radio handle ile tarama boş dönebiliyor; global fallback.
    if (discoveredCount == 0) {
        discoveredCount += scanDevicesForRadio(nullptr, true, &devices);
    }

    // Inquiry başarısızsa eşleşmiş cihazları hızlı şekilde çek.
    if (discoveredCount == 0) {
        discoveredCount += scanDevicesForRadio(nullptr, false, &devices);
    }

    qDebug() << "[BT][WIN] Tarama tamamlandı. Bulunan cihaz:" << devices.count();
    return devices;
}
#endif
} // namespace

BluetoothDeviceModel::BluetoothDeviceModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int BluetoothDeviceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_devices.count();
}

QVariant BluetoothDeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_devices.count()) {
        return {};
    }

    const auto &device = m_devices.at(index.row());
    switch (role) {
    case NameRole:
        return device.name;
    case AddressRole:
        return device.address;
    case RssiRole:
        return device.rssi;
    case PairedRole:
        return device.paired;
    default:
        return {};
    }
}

QHash<int, QByteArray> BluetoothDeviceModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { AddressRole, "address" },
        { RssiRole, "rssi" },
        { PairedRole, "paired" }
    };
}

void BluetoothDeviceModel::clear()
{
    if (m_devices.isEmpty()) {
        return;
    }
    beginResetModel();
    m_devices.clear();
    endResetModel();
}

void BluetoothDeviceModel::upsertDevice(const BluetoothDeviceEntry &entry)
{
    const int index = indexOfDevice(entry);
    if (index >= 0) {
        m_devices[index] = entry;
        const QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, { NameRole, AddressRole, RssiRole, PairedRole });
        return;
    }

    beginInsertRows(QModelIndex(), m_devices.count(), m_devices.count());
    m_devices.append(entry);
    endInsertRows();
    
    // HC ve Car ile başlayan cihazları öne çıkar
    sortDevices();
}

void BluetoothDeviceModel::sortDevices()
{
    beginResetModel();
    std::sort(m_devices.begin(), m_devices.end(), [](const BluetoothDeviceEntry &a, const BluetoothDeviceEntry &b) {
        const bool aPriority = a.name.startsWith("HC", Qt::CaseInsensitive) || a.name.startsWith("Car", Qt::CaseInsensitive);
        const bool bPriority = b.name.startsWith("HC", Qt::CaseInsensitive) || b.name.startsWith("Car", Qt::CaseInsensitive);
        
        if (aPriority && !bPriority) return true;
        if (!aPriority && bPriority) return false;
        
        // Her ikisi de öncelikli veya değilse RSSI'e göre sırala
        return a.rssi > b.rssi;
    });
    endResetModel();
}

QList<BluetoothDeviceEntry> BluetoothDeviceModel::devices() const
{
    return m_devices;
}

int BluetoothDeviceModel::indexOfDevice(const BluetoothDeviceEntry &entry) const
{
    for (int i = 0; i < m_devices.count(); ++i) {
        const auto &device = m_devices.at(i);
        if (!entry.address.isEmpty() && device.address == entry.address) {
            return i;
        }
        if (entry.address.isEmpty() && !entry.name.isEmpty() && device.name == entry.name) {
            return i;
        }
    }
    return -1;
}

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
#ifdef Q_OS_WIN
    , m_discoveryAgent(nullptr)
#else
    , m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this))
#endif
    , m_serviceDiscoveryAgent(nullptr)
    , m_socket(nullptr)
    , m_localDevice()
    , m_pendingAddress()
    , m_isConnected(false)
    , m_isConnecting(false)
    , m_serviceFound(false)
    , m_scanning(false)
    , m_connectionStatus(QStringLiteral("Hazır"))
    , m_deviceName(QStringLiteral("Bağlı değil"))
    , m_batteryLevel(75)
    , m_isCharging(false)
    , m_primaryDeviceRssi(0)
#ifdef Q_OS_WIN
    , m_nativeSocket(static_cast<qintptr>(INVALID_SOCKET))
    , m_nativeReadTimer(new QTimer(this))
    , m_nativeDiscoveryWatcher(new QFutureWatcher<QVariantList>(this))
    , m_winsockReady(false)
#endif
{
    if (m_discoveryAgent) {
        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                this, &BluetoothManager::handleDeviceDiscovered);
        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
                this, &BluetoothManager::handleDiscoveryFinished);
        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
                this, &BluetoothManager::handleDiscoveryFinished);
        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
                this, &BluetoothManager::handleDiscoveryError);
    }

    if (m_localDevice.isValid()
        && m_localDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        m_localDevice.powerOn();
    }

#ifdef Q_OS_WIN
    m_nativeReadTimer->setInterval(kNativeReadIntervalMs);
    connect(m_nativeReadTimer, &QTimer::timeout,
            this, &BluetoothManager::pollNativeSocket);
    connect(m_nativeDiscoveryWatcher, &QFutureWatcher<QVariantList>::finished,
            this, &BluetoothManager::handleNativeDiscoveryFinished);
#endif

    qDebug() << "[BT] Bluetooth Manager başlatıldı";
}

bool BluetoothManager::isConnected() const
{
    return m_isConnected;
}

QString BluetoothManager::connectionStatus() const
{
    return m_connectionStatus;
}

QString BluetoothManager::deviceName() const
{
    return m_deviceName;
}

int BluetoothManager::batteryLevel() const
{
    return m_batteryLevel;
}

bool BluetoothManager::isCharging() const
{
    return m_isCharging;
}

bool BluetoothManager::scanning() const
{
    return m_scanning;
}

QString BluetoothManager::primaryDeviceName() const
{
    return m_primaryDeviceName;
}

QString BluetoothManager::primaryDeviceAddress() const
{
    return m_primaryDeviceAddress;
}

int BluetoothManager::primaryDeviceRssi() const
{
    return m_primaryDeviceRssi;
}

int BluetoothManager::devicesCount() const
{
    return m_deviceModel.rowCount();
}

QAbstractListModel *BluetoothManager::devicesModel()
{
    return &m_deviceModel;
}

void BluetoothManager::startDiscovery()
{
#ifdef Q_OS_WIN
    startDiscoveryNativeWindows();
    return;
#endif

    if (!m_discoveryAgent) {
        return;
    }

    if (m_discoveryAgent->isActive()) {
        m_discoveryAgent->stop();
    }

    m_deviceModel.clear();
    m_primaryDeviceName.clear();
    m_primaryDeviceAddress.clear();
    m_primaryDeviceRssi = -1;
    emit devicesUpdated();

    m_scanning = true;
    emit scanningChanged();
    m_connectionStatus = QStringLiteral("Cihazlar taranıyor...");
    emit connectionStatusChanged();

    qDebug() << "[BT] Cihaz taraması başlatılıyor...";
    const auto methods = m_discoveryAgent->supportedDiscoveryMethods();
    if (methods.testFlag(QBluetoothDeviceDiscoveryAgent::ClassicMethod)) {
        m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);
    } else {
        m_discoveryAgent->start();
    }
}

void BluetoothManager::connectToDevice(const QString &address)
{
#ifdef Q_OS_WIN
    connectToDeviceNativeWindows(address);
    return;
#endif

    if (address.isEmpty()) {
        return;
    }

    const QBluetoothAddress targetAddress(address);
    if (targetAddress.isNull()) {
        m_connectionStatus = QStringLiteral("Geçersiz cihaz adresi");
        emit connectionStatusChanged();
        qDebug() << "[BT] Geçersiz adres:" << address;
        return;
    }

    finalizeServiceDiscovery();
    resetSocket();

    m_pendingAddress = targetAddress;
    m_serviceFound = false;
    m_isConnecting = true;
    m_isConnected = false;

    qDebug() << "[BT] Cihaza bağlanılıyor:" << address;
    emit connectionStateChanged();
    m_connectionStatus = QStringLiteral("Bağlanıyor...");
    emit connectionStatusChanged();

    m_serviceDiscoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
    connect(m_serviceDiscoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
            this, &BluetoothManager::handleServiceDiscovered);
    connect(m_serviceDiscoveryAgent, &QBluetoothServiceDiscoveryAgent::finished,
            this, &BluetoothManager::handleServiceDiscoveryFinished);
    connect(m_serviceDiscoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled,
            this, &BluetoothManager::handleServiceDiscoveryFinished);
    connect(m_serviceDiscoveryAgent, &QBluetoothServiceDiscoveryAgent::errorOccurred,
            this, &BluetoothManager::handleServiceDiscoveryError);

    m_serviceDiscoveryAgent->setRemoteAddress(targetAddress);
    m_serviceDiscoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
}

void BluetoothManager::disconnect()
{
#ifdef Q_OS_WIN
    disconnectNativeWindows();
    return;
#endif

    finalizeServiceDiscovery();

    m_isConnecting = false;
    m_isConnected = false;
    m_pendingAddress = QBluetoothAddress();

    if (m_socket) {
        m_socket->disconnectFromService();
    }
    resetSocket();

    m_deviceName = QStringLiteral("Bağlı değil");
    m_connectionStatus = QStringLiteral("Bağlantı kesildi");
    qDebug() << "[BT] Bağlantı kesildi";
    emit connectionStateChanged();
    emit connectionStatusChanged();
    emit deviceChanged();
}

void BluetoothManager::sendCommand(const QString &command)
{
#ifdef Q_OS_WIN
    sendCommandNativeWindows(command);
    return;
#endif

    if (!m_socket || m_socket->state() != QBluetoothSocket::SocketState::ConnectedState) {
        qDebug() << "[BT] Komut gönderilemedi, soket bağlı değil";
        return;
    }

    QByteArray payload = command.toUtf8();
    if (!payload.endsWith('\n')) {
        payload.append('\n');
    }

    const qint64 bytesWritten = m_socket->write(payload);
    if (bytesWritten < 0) {
        qDebug() << "[BT] Komut gönderme hatası:" << m_socket->errorString();
    } else {
        qDebug() << "[BT] Komut gönderildi:" << command;
    }
}

void BluetoothManager::sendJoystickCommand(const QString &command)
{
#ifdef Q_OS_WIN
    sendCommandNativeWindows(command);
    return;
#endif

    // Socket durumunu gerçek zamanlı kontrol et
    bool actuallyConnected = m_socket && m_socket->state() == QBluetoothSocket::SocketState::ConnectedState && m_socket->isOpen();
    
    if (actuallyConnected != m_isConnected) {
        qDebug() << "[DEBUG] Syncing connection state. UI says:" << m_isConnected 
                 << "Actually:" << actuallyConnected;
        m_isConnected = actuallyConnected;
        emit connectionStateChanged();
    }
    
    if (!actuallyConnected) {
        qDebug() << "[ERROR] Bluetooth soketi açık değil. Connected:" << m_isConnected 
                 << "Socket:" << (m_socket != nullptr) 
                 << "State:" << (m_socket ? static_cast<int>(m_socket->state()) : -1)
                 << "Open:" << (m_socket ? m_socket->isOpen() : false);
        return;
    }

    static QString lastDirection = "";
    if (lastDirection != command) {
        lastDirection = command;
    }

    QByteArray komut;

    if (command == "F") { komut = "F\n"; }
    else if (command == "B") { komut = "B\n"; }
    else if (command == "L") { komut = "L\n"; }
    else if (command == "R") { komut = "R\n"; }
    else { komut = "S\n"; }

    const qint64 bytesWritten = m_socket->write(komut);
    if (bytesWritten < 0) {
        qDebug() << "[BT] Komut gönderme hatası:" << m_socket->errorString();
    } else {
        qDebug() << "[BT] Joystick komut gönderildi:" << komut.trimmed();
    }
}

void BluetoothManager::sendGaugeCommand(const QString &command, int value)
{
QString komut;

#ifdef Q_OS_WIN
    komut = QString("%1%2\n").arg(command).arg(value);
    sendCommandNativeWindows(komut);
    return;
#endif

    // Socket durumunu gerçek zamanlı kontrol et
    bool actuallyConnected = m_socket && m_socket->state() == QBluetoothSocket::SocketState::ConnectedState && m_socket->isOpen();
    
    if (actuallyConnected != m_isConnected) {
        qDebug() << "[DEBUG] Syncing connection state. UI says:" << m_isConnected 
                 << "Actually:" << actuallyConnected;
        m_isConnected = actuallyConnected;
        emit connectionStateChanged();
    }
    
    if (!actuallyConnected) {
        qDebug() << "[ERROR] Bluetooth soketi açık değil. Connected:" << m_isConnected 
                 << "Socket:" << (m_socket != nullptr) 
                 << "State:" << (m_socket ? static_cast<int>(m_socket->state()) : -1)
                 << "Open:" << (m_socket ? m_socket->isOpen() : false);
        return;
    }

    komut = QString("%1%2\n").arg(command).arg(value);
    const qint64 bytesWritten = m_socket->write(komut.toUtf8());
    if (bytesWritten < 0) {
        qDebug() << "[BT] Komut gönderme hatası:" << m_socket->errorString();
    } else {
        qDebug() << "[BT] Gauge komut gönderildi:" << komut.trimmed();
    }
}

void BluetoothManager::sendNormalSpeed(int value)
{
    sendGaugeCommand("N", value);
}

void BluetoothManager::sendTurnSpeed(int value)
{
    sendGaugeCommand("T", value);
}

void BluetoothManager::requestBatteryStatus()
{
    if (m_isConnected && m_socket->isOpen()) {
        m_socket->write("BATT?\n");
        qDebug() << "[BT] Batarya durumu istendi";
    }
}

void BluetoothManager::handleDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    BluetoothDeviceEntry entry;
    entry.name = info.name().isEmpty() ? info.address().toString() : info.name();
    entry.address = info.address().toString();
    entry.rssi = info.rssi();

    if (m_localDevice.isValid() && !entry.address.isEmpty()) {
        const auto pairing = m_localDevice.pairingStatus(info.address());
        entry.paired = pairing == QBluetoothLocalDevice::Paired
            || pairing == QBluetoothLocalDevice::AuthorizedPaired;
    }

    m_deviceModel.upsertDevice(entry);
    updatePrimaryDevice();

    QStringList deviceNames;
    const auto devices = m_deviceModel.devices();
    deviceNames.reserve(devices.count());
    for (const auto &device : devices) {
        deviceNames.append(QStringLiteral("%1 [%2]").arg(device.name, device.address));
    }
    emit devicesDiscovered(deviceNames);
    emit devicesUpdated();
}

void BluetoothManager::handleDiscoveryFinished()
{
    if (m_scanning) {
        m_scanning = false;
        emit scanningChanged();
    }
    if (!m_isConnected && !m_isConnecting && m_connectionStatus != QStringLiteral("Bağlanıyor...")) {
        m_connectionStatus = QStringLiteral("Hazır");
        emit connectionStatusChanged();
    }
    emit devicesUpdated();
    qDebug() << "[BT] Cihaz taraması tamamlandı";
}

void BluetoothManager::handleDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    Q_UNUSED(error)
    if (m_scanning) {
        m_scanning = false;
        emit scanningChanged();
    }
    m_connectionStatus = QStringLiteral("Tarama hatası");
    emit connectionStatusChanged();
    qDebug() << "[BT] Cihaz taraması hatası";
}

void BluetoothManager::handleServiceDiscovered(const QBluetoothServiceInfo &service)
{
    if (m_serviceFound || !m_isConnecting) {
        return;
    }

    qDebug() << "[BT] Bulunan servis:" << service.serviceName()
             << "UUID:" << service.serviceUuid().toString();

    if (service.socketProtocol() != QBluetoothServiceInfo::RfcommProtocol) {
        return;
    }

    m_serviceFound = true;
    qDebug() << "[BT] RFCOMM servisi bulundu, bağlanılıyor...";
    beginSocketConnection(m_pendingAddress, service.serviceUuid());

    if (m_serviceDiscoveryAgent && m_serviceDiscoveryAgent->isActive()) {
        m_serviceDiscoveryAgent->stop();
    }
}

void BluetoothManager::handleServiceDiscoveryFinished()
{
    const bool shouldFallbackToSerial = m_isConnecting && !m_serviceFound;
    finalizeServiceDiscovery();

    if (shouldFallbackToSerial) {
        qDebug() << "[BT] RFCOMM servis bulunamadı, SPP UUID ile doğrudan deneniyor";
        beginSocketConnection(
            m_pendingAddress,
            QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
    }
}

void BluetoothManager::handleServiceDiscoveryError(QBluetoothServiceDiscoveryAgent::Error error)
{
    Q_UNUSED(error)
    qDebug() << "[BT] Servis keşfi hatası";
    const bool shouldFallbackToSerial = m_isConnecting && !m_serviceFound;
    finalizeServiceDiscovery();

    if (shouldFallbackToSerial) {
        beginSocketConnection(
            m_pendingAddress,
            QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
    } else {
        m_isConnecting = false;
        m_connectionStatus = QStringLiteral("Servis keşfi hatası");
        emit connectionStatusChanged();
    }
}

void BluetoothManager::handleSocketConnected()
{
    m_isConnecting = false;
    m_isConnected = true;
    emit connectionStateChanged();

    const QString peerAddress = m_socket ? m_socket->peerAddress().toString() : m_pendingAddress.toString();
    const QString resolvedName = resolveDeviceNameByAddress(peerAddress);
    m_deviceName = resolvedName.isEmpty() ? peerAddress : resolvedName;
    emit deviceChanged();

    m_connectionStatus = QStringLiteral("Bağlandı");
    emit connectionStatusChanged();
    qDebug() << "[BT] Bluetooth cihazına bağlanıldı:" << m_deviceName;
    qDebug() << "[DEBUG] Socket connected. State:" << (m_socket ? static_cast<int>(m_socket->state()) : -1) 
         << "Open:" << (m_socket ? m_socket->isOpen() : false)
         << "Writable:" << (m_socket ? m_socket->isWritable() : false);
}

void BluetoothManager::handleSocketDisconnected()
{
    const bool hadConnection = m_isConnected || m_isConnecting;
    m_isConnecting = false;
    m_isConnected = false;
    emit connectionStateChanged();

    m_connectionStatus = QStringLiteral("Bağlantı kesildi");
    emit connectionStatusChanged();

    if (hadConnection) {
        qDebug() << "[BT] Bluetooth bağlantısı kesildi";
    }
}

void BluetoothManager::handleSocketReadyRead()
{
    if (!m_socket) {
        return;
    }

    const QByteArray data = m_socket->readAll();
    const QString receivedData = QString::fromUtf8(data).trimmed();
    if (receivedData.isEmpty()) {
        return;
    }

    parseIncomingData(receivedData);

    const QString debugData = receivedData.size() > 50
        ? receivedData.left(47) + QStringLiteral("...")
        : receivedData;
    qDebug() << "[DATA] Gelen:" << debugData;
}

void BluetoothManager::handleSocketError(QBluetoothSocket::SocketError error)
{
    Q_UNUSED(error)
    m_isConnecting = false;
    if (!m_isConnected) {
        m_connectionStatus = QStringLiteral("Bağlantı hatası");
        emit connectionStatusChanged();
    }
    qDebug() << "[BT] Soket hatası:" << (m_socket ? m_socket->errorString() : QStringLiteral("Bilinmeyen hata"));
}

void BluetoothManager::beginSocketConnection(const QBluetoothAddress &targetAddress, const QBluetoothUuid &serviceUuid)
{
    if (targetAddress.isNull()) {
        m_isConnecting = false;
        m_connectionStatus = QStringLiteral("Geçersiz cihaz adresi");
        emit connectionStatusChanged();
        return;
    }

    resetSocket();

    m_socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    qDebug() << "[BT] Yeni QBluetoothSocket oluşturuldu";

    connect(m_socket, &QBluetoothSocket::connected,
            this, &BluetoothManager::handleSocketConnected);
    connect(m_socket, &QBluetoothSocket::disconnected,
            this, &BluetoothManager::handleSocketDisconnected);
    connect(m_socket, &QBluetoothSocket::readyRead,
            this, &BluetoothManager::handleSocketReadyRead);
    connect(m_socket, &QBluetoothSocket::errorOccurred,
            this, &BluetoothManager::handleSocketError);

    m_socket->connectToService(targetAddress, serviceUuid);
}

void BluetoothManager::finalizeServiceDiscovery()
{
    if (!m_serviceDiscoveryAgent) {
        return;
    }

    QObject::disconnect(m_serviceDiscoveryAgent, nullptr, this, nullptr);
    m_serviceDiscoveryAgent->deleteLater();
    m_serviceDiscoveryAgent = nullptr;
}

void BluetoothManager::resetSocket()
{
    if (!m_socket) {
        return;
    }

    QObject::disconnect(m_socket, nullptr, this, nullptr);
    m_socket->deleteLater();
    m_socket = nullptr;
    
    // Bağlantı durumunu güncelle
    if (m_isConnected) {
        m_isConnected = false;
        emit connectionStateChanged();
        m_connectionStatus = QStringLiteral("Bağlantı kesildi");
        emit connectionStatusChanged();
        qDebug() << "[BT] Socket reset - bağlantı durumu güncellendi";
    }
}

void BluetoothManager::parseIncomingData(const QString &receivedData)
{
    qDebug() << "[DEBUG] Gelen veri:" << receivedData;
    
    if (receivedData.contains(QStringLiteral("VOLT:"))
        && receivedData.contains(QStringLiteral("BATT:"))) {
        qDebug() << "[DEBUG] Batarya verisi algılandı!";
        parseBatteryData(receivedData);
    } else {
        qDebug() << "[DEBUG] Batarya verisi bulunamadı. VOLT: veya BATT: içermiyor.";
    }
}

void BluetoothManager::parseBatteryData(const QString &receivedData)
{
    static const QRegularExpression batteryRegex(QStringLiteral("BATT\\s*:?\\s*(\\d{1,3})"),
                                                 QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression chargingOnRegex(QStringLiteral("(CHG|CHARGING)\\s*:?\\s*(1|ON|TRUE)"),
                                                    QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression chargingOffRegex(QStringLiteral("(CHG|CHARGING)\\s*:?\\s*(0|OFF|FALSE)"),
                                                     QRegularExpression::CaseInsensitiveOption);

    const auto batteryMatch = batteryRegex.match(receivedData);
    if (batteryMatch.hasMatch()) {
        const int parsedValue = qBound(0, batteryMatch.captured(1).toInt(), 100);
        if (parsedValue != m_batteryLevel) {
            m_batteryLevel = parsedValue;
            emit batteryChanged();
        }
    }

    bool hasChargingField = false;
    bool newChargingValue = m_isCharging;
    if (chargingOnRegex.match(receivedData).hasMatch()) {
        newChargingValue = true;
        hasChargingField = true;
    } else if (chargingOffRegex.match(receivedData).hasMatch()) {
        newChargingValue = false;
        hasChargingField = true;
    }

    if (hasChargingField && newChargingValue != m_isCharging) {
        m_isCharging = newChargingValue;
        emit chargingChanged();
    }
}

QString BluetoothManager::resolveDeviceNameByAddress(const QString &address) const
{
    const auto devices = m_deviceModel.devices();
    for (const auto &device : devices) {
        if (QString::compare(device.address, address, Qt::CaseInsensitive) == 0) {
            return device.name;
        }
    }
    return {};
}

void BluetoothManager::updatePrimaryDevice()
{
    const auto devices = m_deviceModel.devices();
    if (devices.isEmpty()) {
        m_primaryDeviceName.clear();
        m_primaryDeviceAddress.clear();
        m_primaryDeviceRssi = -1;
        return;
    }

    const BluetoothDeviceEntry *best = nullptr;
    for (const auto &device : devices) {
        if (!best || device.rssi > best->rssi) {
            best = &device;
        }
    }

    if (best) {
        m_primaryDeviceName = best->name;
        m_primaryDeviceAddress = best->address;
        m_primaryDeviceRssi = best->rssi;
    }
}

#ifdef Q_OS_WIN
void BluetoothManager::startDiscoveryNativeWindows()
{
    if (m_nativeDiscoveryWatcher && m_nativeDiscoveryWatcher->isRunning()) {
        qDebug() << "[BT][WIN] Tarama zaten devam ediyor, yeni istek atlandı";
        return;
    }

    m_scanning = true;
    emit scanningChanged();
    m_connectionStatus = QStringLiteral("Cihazlar taranıyor...");
    emit connectionStatusChanged();

    qDebug() << "[BT][WIN] Windows yerel Bluetooth taraması başlatılıyor...";
    m_nativeDiscoveryWatcher->setFuture(QtConcurrent::run(scanNativeWindowsDevices));
}

void BluetoothManager::handleNativeDiscoveryFinished()
{
    if (!m_nativeDiscoveryWatcher) {
        return;
    }

    const QVariantList scannedResults = m_nativeDiscoveryWatcher->result();
    QList<BluetoothDeviceEntry> scannedDevices;
    scannedDevices.reserve(scannedResults.count());
    for (const QVariant &item : scannedResults) {
        const QVariantMap map = item.toMap();
        BluetoothDeviceEntry entry;
        entry.name = map.value(QStringLiteral("name")).toString();
        entry.address = map.value(QStringLiteral("address")).toString();
        entry.rssi = map.value(QStringLiteral("rssi")).toInt();
        entry.paired = map.value(QStringLiteral("paired")).toBool();
        if (!entry.address.isEmpty()) {
            scannedDevices.append(entry);
        }
    }

    qDebug() << "[BT][WIN] UI'ye aktarılacak cihaz sayısı:" << scannedDevices.count();

    // HC ve Car ile başlayan cihazları öne çıkar
    std::sort(scannedDevices.begin(), scannedDevices.end(), [](const BluetoothDeviceEntry &a, const BluetoothDeviceEntry &b) {
        const bool aPriority = a.name.startsWith("HC", Qt::CaseInsensitive) || a.name.startsWith("Car", Qt::CaseInsensitive);
        const bool bPriority = b.name.startsWith("HC", Qt::CaseInsensitive) || b.name.startsWith("Car", Qt::CaseInsensitive);
        
        if (aPriority && !bPriority) return true;
        if (!aPriority && bPriority) return false;
        
        // Her ikisi de öncelikli veya değilse RSSI'e göre sırala
        return a.rssi > b.rssi;
    });

    m_deviceModel.clear();
    for (const auto &entry : scannedDevices) {
        m_deviceModel.upsertDevice(entry);
    }
    qDebug() << "[BT][WIN] Model satır sayısı:" << m_deviceModel.rowCount();
    updatePrimaryDevice();

    QStringList deviceNames;
    deviceNames.reserve(scannedDevices.count());
    for (const auto &device : scannedDevices) {
        deviceNames.append(QStringLiteral("%1 [%2]").arg(device.name, device.address));
    }

    emit devicesDiscovered(deviceNames);
    emit devicesUpdated();

    if (m_scanning) {
        m_scanning = false;
        emit scanningChanged();
    }

    if (scannedDevices.isEmpty()) {
        m_connectionStatus = QStringLiteral("Cihaz bulunamadı");
    } else if (!m_isConnected && !m_isConnecting) {
        m_connectionStatus = QStringLiteral("Hazır");
    }
    emit connectionStatusChanged();
}

void BluetoothManager::connectToDeviceNativeWindows(const QString &address)
{
    if (address.isEmpty()) {
        return;
    }

    quint64 rawAddress = 0;
    if (!parseWindowsBluetoothAddress(address, &rawAddress)) {
        m_connectionStatus = QStringLiteral("Geçersiz cihaz adresi");
        emit connectionStatusChanged();
        qDebug() << "[BT][WIN] Geçersiz adres:" << address;
        return;
    }

    if (!ensureWinsock()) {
        m_connectionStatus = QStringLiteral("Windows soket başlatılamadı");
        emit connectionStatusChanged();
        return;
    }

    finalizeServiceDiscovery();
    resetSocket();
    closeNativeSocket();

    m_isConnecting = true;
    m_isConnected = false;
    emit connectionStateChanged();

    m_connectionStatus = QStringLiteral("Bağlanıyor...");
    emit connectionStatusChanged();

    SOCKET socketHandle = ::socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (socketHandle == INVALID_SOCKET) {
        m_isConnecting = false;
        emit connectionStateChanged();
        m_connectionStatus = QStringLiteral("Bluetooth soketi oluşturulamadı");
        emit connectionStatusChanged();
        qDebug() << "[BT][WIN] socket() hatası:" << WSAGetLastError();
        return;
    }

    SOCKADDR_BTH remoteAddress;
    ZeroMemory(&remoteAddress, sizeof(remoteAddress));
    remoteAddress.addressFamily = AF_BTH;
    remoteAddress.btAddr = static_cast<BTH_ADDR>(rawAddress);
    remoteAddress.serviceClassId = kSerialPortServiceUuid;
    remoteAddress.port = BT_PORT_ANY;

    int connectResult = ::connect(socketHandle,
                                  reinterpret_cast<sockaddr *>(&remoteAddress),
                                  sizeof(remoteAddress));

    if (connectResult == SOCKET_ERROR) {
        const int firstError = WSAGetLastError();

        // Bazı cihazlarda SPP servisi kanal 1 üzerinden erişilebilir.
        remoteAddress.serviceClassId = GUID{};
        remoteAddress.port = 1;

        connectResult = ::connect(socketHandle,
                                  reinterpret_cast<sockaddr *>(&remoteAddress),
                                  sizeof(remoteAddress));
        if (connectResult == SOCKET_ERROR) {
            const int secondError = WSAGetLastError();
            ::closesocket(socketHandle);
            m_isConnecting = false;
            emit connectionStateChanged();
            m_connectionStatus = QStringLiteral("Bağlantı hatası");
            emit connectionStatusChanged();
            qDebug() << "[BT][WIN] connect() başarısız. İlk hata:" << firstError
                     << "İkinci hata:" << secondError;
            return;
        }
    }

    u_long nonBlocking = 1;
    if (::ioctlsocket(socketHandle, FIONBIO, &nonBlocking) == SOCKET_ERROR) {
        qDebug() << "[BT][WIN] ioctlsocket(FIONBIO) hatası:" << WSAGetLastError();
    }

    m_nativeSocket = static_cast<qintptr>(socketHandle);
    m_nativeReceiveBuffer.clear();
    if (m_nativeReadTimer) {
        m_nativeReadTimer->start();
    }

    m_isConnecting = false;
    m_isConnected = true;
    emit connectionStateChanged();

    const QString resolvedName = resolveDeviceNameByAddress(address);
    m_deviceName = resolvedName.isEmpty() ? address : resolvedName;
    emit deviceChanged();

    m_connectionStatus = QStringLiteral("Bağlandı");
    emit connectionStatusChanged();
    qDebug() << "[BT][WIN] Cihaza bağlanıldı:" << m_deviceName << address;
}

void BluetoothManager::disconnectNativeWindows()
{
    const bool hadConnection = m_isConnected || m_isConnecting;

    m_isConnecting = false;
    m_isConnected = false;
    closeNativeSocket();

    m_deviceName = QStringLiteral("Bağlı değil");
    m_connectionStatus = QStringLiteral("Bağlantı kesildi");

    emit connectionStateChanged();
    emit connectionStatusChanged();
    emit deviceChanged();

    if (hadConnection) {
        qDebug() << "[BT][WIN] Bluetooth bağlantısı kesildi";
    }
}

void BluetoothManager::sendCommandNativeWindows(const QString &command)
{
    if (!m_isConnected || m_nativeSocket == static_cast<qintptr>(INVALID_SOCKET)) {
        qDebug() << "[BT][WIN] Komut gönderilemedi, soket bağlı değil";
        return;
    }

    SOCKET socketHandle = static_cast<SOCKET>(m_nativeSocket);

    QByteArray payload = command.toUtf8();
    if (!payload.endsWith('\n')) {
        payload.append('\n');
    }

    const int sent = ::send(socketHandle, payload.constData(), payload.size(), 0);
    if (sent == SOCKET_ERROR) {
        qDebug() << "[BT][WIN] Komut gönderme hatası:" << WSAGetLastError();
        return;
    }

    qDebug() << "[BT][WIN] Komut gönderildi:" << command;
}

void BluetoothManager::pollNativeSocket()
{
    if (m_nativeSocket == static_cast<qintptr>(INVALID_SOCKET)) {
        return;
    }

    SOCKET socketHandle = static_cast<SOCKET>(m_nativeSocket);

    char buffer[1024];
    while (true) {
        const int received = ::recv(socketHandle, buffer, sizeof(buffer), 0);
        if (received > 0) {
            m_nativeReceiveBuffer.append(buffer, received);
            continue;
        }

        if (received == 0) {
            qDebug() << "[BT][WIN] Uzak cihaz bağlantıyı kapattı";
            disconnectNativeWindows();
            return;
        }

        const int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            break;
        }

        qDebug() << "[BT][WIN] recv() hatası:" << error;
        disconnectNativeWindows();
        return;
    }

    int newlineIndex = m_nativeReceiveBuffer.indexOf('\n');
    while (newlineIndex >= 0) {
        const QByteArray packet = m_nativeReceiveBuffer.left(newlineIndex);
        m_nativeReceiveBuffer.remove(0, newlineIndex + 1);

        const QString receivedData = QString::fromUtf8(packet).trimmed();
        if (!receivedData.isEmpty()) {
            parseIncomingData(receivedData);
            const QString debugData = receivedData.size() > 50
                ? receivedData.left(47) + QStringLiteral("...")
                : receivedData;
            qDebug() << "[DATA] Gelen:" << debugData;
        }

        newlineIndex = m_nativeReceiveBuffer.indexOf('\n');
    }

    if (m_nativeReceiveBuffer.size() > 2048) {
        const QString receivedData = QString::fromUtf8(m_nativeReceiveBuffer).trimmed();
        m_nativeReceiveBuffer.clear();
        if (!receivedData.isEmpty()) {
            parseIncomingData(receivedData);
            qDebug() << "[DATA] Gelen (parçalı):" << receivedData.left(50);
        }
    }
}

bool BluetoothManager::ensureWinsock()
{
    if (m_winsockReady) {
        return true;
    }

    WSADATA wsaData;
    const int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        qDebug() << "[BT][WIN] WSAStartup hatası:" << result;
        return false;
    }

    m_winsockReady = true;
    return true;
}

void BluetoothManager::closeNativeSocket()
{
    if (m_nativeReadTimer && m_nativeReadTimer->isActive()) {
        m_nativeReadTimer->stop();
    }

    if (m_nativeSocket == static_cast<qintptr>(INVALID_SOCKET)) {
        m_nativeReceiveBuffer.clear();
        return;
    }

    SOCKET socketHandle = static_cast<SOCKET>(m_nativeSocket);
    ::shutdown(socketHandle, SD_BOTH);
    ::closesocket(socketHandle);
    m_nativeSocket = static_cast<qintptr>(INVALID_SOCKET);
    m_nativeReceiveBuffer.clear();
}

QString BluetoothManager::formatWindowsBluetoothAddress(quint64 rawAddress)
{
    BLUETOOTH_ADDRESS address;
    address.ullLong = static_cast<BTH_ADDR>(rawAddress);

    return QStringLiteral("%1:%2:%3:%4:%5:%6")
        .arg(address.rgBytes[5], 2, 16, QLatin1Char('0'))
        .arg(address.rgBytes[4], 2, 16, QLatin1Char('0'))
        .arg(address.rgBytes[3], 2, 16, QLatin1Char('0'))
        .arg(address.rgBytes[2], 2, 16, QLatin1Char('0'))
        .arg(address.rgBytes[1], 2, 16, QLatin1Char('0'))
        .arg(address.rgBytes[0], 2, 16, QLatin1Char('0'))
        .toUpper();
}

bool BluetoothManager::parseWindowsBluetoothAddress(const QString &addressText, quint64 *outRawAddress)
{
    if (!outRawAddress) {
        return false;
    }

    const QStringList parts = addressText.split(':');
    if (parts.size() != 6) {
        return false;
    }

    BLUETOOTH_ADDRESS address;
    ZeroMemory(&address, sizeof(address));

    for (int i = 0; i < parts.size(); ++i) {
        bool ok = false;
        const int parsed = parts.at(i).toInt(&ok, 16);
        if (!ok || parsed < 0 || parsed > 0xFF) {
            return false;
        }
        address.rgBytes[5 - i] = static_cast<UCHAR>(parsed);
    }

    *outRawAddress = static_cast<quint64>(address.ullLong);
    return true;
}
#endif
