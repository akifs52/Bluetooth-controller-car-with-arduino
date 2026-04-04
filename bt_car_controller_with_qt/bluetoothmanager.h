#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QAbstractListModel>
#include <QBluetoothAddress>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothServiceInfo>
#include <QBluetoothSocket>
#include <QBluetoothUuid>
#include <QByteArray>
#include <QFutureWatcher>
#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QtGlobal>

struct BluetoothDeviceEntry
{
    QString name;
    QString address;
    int rssi = 0;
    bool paired = false;
};

class BluetoothDeviceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum DeviceRoles {
        NameRole = Qt::UserRole + 1,
        AddressRole,
        RssiRole,
        PairedRole
    };

    explicit BluetoothDeviceModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void clear();
    void upsertDevice(const BluetoothDeviceEntry &entry);
    void sortDevices();
    QList<BluetoothDeviceEntry> devices() const;

private:
    int indexOfDevice(const BluetoothDeviceEntry &entry) const;

    QList<BluetoothDeviceEntry> m_devices;
};

class BluetoothManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStateChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryChanged)
    Q_PROPERTY(bool isCharging READ isCharging NOTIFY chargingChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(QString primaryDeviceName READ primaryDeviceName NOTIFY devicesUpdated)
    Q_PROPERTY(QString primaryDeviceAddress READ primaryDeviceAddress NOTIFY devicesUpdated)
    Q_PROPERTY(int primaryDeviceRssi READ primaryDeviceRssi NOTIFY devicesUpdated)
    Q_PROPERTY(int devicesCount READ devicesCount NOTIFY devicesUpdated)
    Q_PROPERTY(QAbstractListModel *devicesModel READ devicesModel CONSTANT)

public:
    explicit BluetoothManager(QObject *parent = nullptr);

    bool isConnected() const;
    QString connectionStatus() const;
    QString deviceName() const;
    int batteryLevel() const;
    bool isCharging() const;
    bool scanning() const;

    QString primaryDeviceName() const;
    QString primaryDeviceAddress() const;
    int primaryDeviceRssi() const;
    int devicesCount() const;
    QAbstractListModel *devicesModel();

public slots:
    void startDiscovery();
    void connectToDevice(const QString &address);
    void disconnect();
    void sendCommand(const QString &command);
    void sendJoystickCommand(const QString &command);
    void sendGaugeCommand(const QString &command, int value);
    void sendNormalSpeed(int value);
    void sendTurnSpeed(int value);
    void requestBatteryStatus();

signals:
    void connectionStateChanged();
    void connectionStatusChanged();
    void deviceChanged();
    void batteryChanged();
    void chargingChanged();
    void devicesDiscovered(const QStringList &deviceNames);
    void devicesUpdated();
    void scanningChanged();

private:
    void handleDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void handleDiscoveryFinished();
    void handleDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error);
    void handleServiceDiscovered(const QBluetoothServiceInfo &service);
    void handleServiceDiscoveryFinished();
    void handleServiceDiscoveryError(QBluetoothServiceDiscoveryAgent::Error error);
    void handleSocketConnected();
    void handleSocketDisconnected();
    void handleSocketReadyRead();
    void handleSocketError(QBluetoothSocket::SocketError error);
    void beginSocketConnection(const QBluetoothAddress &targetAddress, const QBluetoothUuid &serviceUuid);
    void finalizeServiceDiscovery();
    void resetSocket();
    void parseIncomingData(const QString &receivedData);
    void parseBatteryData(const QString &receivedData);
    QString resolveDeviceNameByAddress(const QString &address) const;
    void updatePrimaryDevice();

#ifdef Q_OS_WIN
    void startDiscoveryNativeWindows();
    void handleNativeDiscoveryFinished();
    void connectToDeviceNativeWindows(const QString &address);
    void disconnectNativeWindows();
    void sendCommandNativeWindows(const QString &command);
    void pollNativeSocket();
    bool ensureWinsock();
    void closeNativeSocket();
    static QString formatWindowsBluetoothAddress(quint64 rawAddress);
    static bool parseWindowsBluetoothAddress(const QString &addressText, quint64 *outRawAddress);
#endif

    BluetoothDeviceModel m_deviceModel;
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QBluetoothServiceDiscoveryAgent *m_serviceDiscoveryAgent;
    QBluetoothSocket *m_socket;
    QBluetoothLocalDevice m_localDevice;
    QBluetoothAddress m_pendingAddress;
    bool m_isConnected;
    bool m_isConnecting;
    bool m_serviceFound;
    bool m_scanning;
    QString m_connectionStatus;
    QString m_deviceName;
    int m_batteryLevel;
    bool m_isCharging;
    QString m_primaryDeviceName;
    QString m_primaryDeviceAddress;
    int m_primaryDeviceRssi;

#ifdef Q_OS_WIN
    qintptr m_nativeSocket;
    QByteArray m_nativeReceiveBuffer;
    QTimer *m_nativeReadTimer;
    QFutureWatcher<QVariantList> *m_nativeDiscoveryWatcher;
    bool m_winsockReady;
#endif
};

#endif // BLUETOOTHMANAGER_H
