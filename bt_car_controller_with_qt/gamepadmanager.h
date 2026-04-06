#ifndef GAMEPADMANAGER_H
#define GAMEPADMANAGER_H

#include <QObject>
#include <QTimer>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#include <xinput.h>
#endif

class GamepadManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool gamepadConnected READ gamepadConnected NOTIFY gamepadConnectedChanged)
    Q_PROPERTY(QString gamepadName READ gamepadName NOTIFY gamepadNameChanged)

public:
    explicit GamepadManager(QObject *parent = nullptr);
    ~GamepadManager();

    bool gamepadConnected() const;
    QString gamepadName() const;

    Q_INVOKABLE void scanForGamepads();

signals:
    void gamepadConnectedChanged();
    void gamepadNameChanged();
    void directionChanged(const QString &direction);
    void axisValuesChanged(double xAxis, double yAxis);
    void normalSpeedChangeRequested(int delta);  // RB/RT için (+/-50)
    void turnSpeedChangeRequested(int delta);    // LB/LT için (+/-50)

private slots:
    void pollGamepad();

private:
    bool m_connected = false;
    int m_controllerId = -1;
    QString m_gamepadName;
    QString m_currentDirection = "S";
    QTimer *m_pollTimer = nullptr;
    
    double m_leftX = 0.0;
    double m_leftY = 0.0;
    
#ifdef Q_OS_WIN
    WORD m_lastButtons = 0;  // Önceki buton durumu
#else
    quint32 m_lastButtons = 0;  // Android için
#endif
    
    void updateDirection();
#ifdef Q_OS_WIN
    void checkButtons(const XINPUT_STATE &state);  // Butonları kontrol et
#else
    void checkButtons(quint32 buttons);  // Android stub
#endif
    QString getControllerName(int id);
    void vibrate(int leftMotor = 0, int rightMotor = 0);  // Titreşim
};

#endif // GAMEPADMANAGER_H
