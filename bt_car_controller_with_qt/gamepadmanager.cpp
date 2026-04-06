#include "gamepadmanager.h"
#include <QDebug>
#include <cmath>

#ifdef Q_OS_WIN
#include <windows.h>

typedef DWORD (WINAPI *XInputGetStateFunc)(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD (WINAPI *XInputSetStateFunc)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

static HMODULE xinputDll = nullptr;
static XInputGetStateFunc pXInputGetState = nullptr;
static XInputSetStateFunc pXInputSetState = nullptr;

static bool loadXInput()
{
    if (xinputDll) return true;
    
    // XInput 1.4 önce dene (Windows 8+)
    xinputDll = LoadLibraryW(L"XInput1_4.dll");
    if (!xinputDll) {
        // Yedek olarak 1.3 dene (Windows 7)
        xinputDll = LoadLibraryW(L"XInput1_3.dll");
    }
    
    if (xinputDll) {
        pXInputGetState = (XInputGetStateFunc)GetProcAddress(xinputDll, "XInputGetState");
        pXInputSetState = (XInputSetStateFunc)GetProcAddress(xinputDll, "XInputSetState");
        return pXInputGetState != nullptr;
    }
    return false;
}

static void unloadXInput()
{
    if (xinputDll) {
        FreeLibrary(xinputDll);
        xinputDll = nullptr;
        pXInputGetState = nullptr;
        pXInputSetState = nullptr;
    }
}
#endif

GamepadManager::GamepadManager(QObject *parent)
    : QObject(parent)
{
    // İlk gamepad'i otomatik bul
    scanForGamepads();
    
    // Periyodik polling için timer
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(50); // 50ms = 20Hz
    connect(m_pollTimer, &QTimer::timeout, this, &GamepadManager::pollGamepad);
    m_pollTimer->start();
}

GamepadManager::~GamepadManager()
{
#ifdef Q_OS_WIN
    unloadXInput();
#endif
}

bool GamepadManager::gamepadConnected() const
{
    return m_connected;
}

QString GamepadManager::gamepadName() const
{
    return m_gamepadName;
}

QString GamepadManager::getControllerName(int id)
{
    switch (id) {
        case 0: return QStringLiteral("Controller 1");
        case 1: return QStringLiteral("Controller 2");
        case 2: return QStringLiteral("Controller 3");
        case 3: return QStringLiteral("Controller 4");
        default: return QStringLiteral("Unknown");
    }
}

void GamepadManager::scanForGamepads()
{
#ifdef Q_OS_WIN
    if (!loadXInput()) {
        qDebug() << "[GAMEPAD] XInput yüklenemedi";
        return;
    }
    
    for (int i = 0; i < 4; i++) {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));
        
        if (pXInputGetState(i, &state) == ERROR_SUCCESS) {
            if (!m_connected || m_controllerId != i) {
                m_connected = true;
                m_controllerId = i;
                m_gamepadName = getControllerName(i);
                emit gamepadConnectedChanged();
                emit gamepadNameChanged();
                qDebug() << "[GAMEPAD] Controller bağlandı:" << m_gamepadName << "ID:" << i;
            }
            return;
        }
    }
#endif
    
    // No controller found
    if (m_connected) {
        m_connected = false;
        m_controllerId = -1;
        m_gamepadName = QStringLiteral("Bağlı değil");
        emit gamepadConnectedChanged();
        emit gamepadNameChanged();
    }
}

void GamepadManager::pollGamepad()
{
    scanForGamepads();
    
    if (!m_connected) {
        if (m_currentDirection != "S") {
            m_currentDirection = "S";
            emit directionChanged("S");
        }
        return;
    }
    
#ifdef Q_OS_WIN
    if (!pXInputGetState) return;
    
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));
    
    if (pXInputGetState(m_controllerId, &state) != ERROR_SUCCESS) {
        m_connected = false;
        emit gamepadConnectedChanged();
        return;
    }
    
    // Sol analog stick değerlerini al (-32768 ile 32767 arası)
    // -1.0 ile 1.0 arasına normalize et
    double x = state.Gamepad.sThumbLX / 32767.0;
    double y = state.Gamepad.sThumbLY / 32767.0;
    
    // Deadzone uygula (XInput deadzone: 7849)
    const double DEADZONE = 0.15;
    if (std::abs(x) < DEADZONE) x = 0;
    if (std::abs(y) < DEADZONE) y = 0;
    
    m_leftX = x;
    m_leftY = -y; // Y eksenini ters çevir (yukarı = -1, aşağı = +1)
    
    emit axisValuesChanged(m_leftX, m_leftY);
    updateDirection();
#ifdef Q_OS_WIN
    checkButtons(state);
#else
    checkButtons(0);  // Android stub
#endif
#endif
}

void GamepadManager::updateDirection()
{
    double x = m_leftX; // Yatay: -1 = sol, +1 = sağ
    double y = m_leftY; // Dikey: -1 = yukarı(ileri), +1 = aşağı(geri)
    
    QString newDirection = "S";
    
    // WASD mantığı ile aynı
    if (std::abs(x) > std::abs(y)) {
        // Yatay hareket baskın
        if (x > 0.3) {
            newDirection = "R"; // Sağ
        } else if (x < -0.3) {
            newDirection = "L"; // Sol
        }
    } else {
        // Dikey hareket baskın
        if (y > 0.3) {
            newDirection = "B"; // Geri
        } else if (y < -0.3) {
            newDirection = "F"; // İleri
        }
    }
    
    // Sadece yön değiştiğinde sinyal gönder
    if (newDirection != m_currentDirection) {
        m_currentDirection = newDirection;
        emit directionChanged(newDirection);
        qDebug() << "[GAMEPAD] Yön değişti:" << newDirection << "x:" << x << "y:" << y;
        
        // Hareket yön değiştiğinde titreşim (S hariç)
        if (newDirection != "S") {
            vibrate(2000, 2000); // Kısa titreşim
        }
    }
}

void GamepadManager::vibrate(int leftMotor, int rightMotor)
{
    Q_UNUSED(leftMotor)
    Q_UNUSED(rightMotor)
#ifdef Q_OS_WIN
    if (!pXInputSetState || m_controllerId < 0) return;

    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = leftMotor;   // 0-65535
    vibration.wRightMotorSpeed = rightMotor; // 0-65535
    pXInputSetState(m_controllerId, &vibration);
#endif
}

#ifdef Q_OS_WIN
void GamepadManager::checkButtons(const XINPUT_STATE &state)
{
    WORD buttons = state.Gamepad.wButtons;
    WORD changes = buttons ^ m_lastButtons;
    WORD pressed = changes & buttons;  // Yeni basılan butonlar

    // RB (Right Bumper) - Normal speed -50
    if (pressed & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
        emit normalSpeedChangeRequested(-50);
        qDebug() << "[GAMEPAD] RB basıldı - Normal Speed -50";
        vibrate(0, 10000); // Sağ motor titreşim
    }

    // LB (Left Bumper) - Turn speed -50
    if (pressed & XINPUT_GAMEPAD_LEFT_SHOULDER) {
        emit turnSpeedChangeRequested(-50);
        qDebug() << "[GAMEPAD] LB basıldı - Turn Speed -50";
        vibrate(10000, 0); // Sol motor titreşim
    }

    // RT (Right Trigger) - Normal speed +50
    // Trigger analog değer, 255'ten büyük bir threshold kullan
    BYTE rt = state.Gamepad.bRightTrigger;
    static bool rtWasPressed = false;
    bool rtPressed = rt > 200; // 200/255 threshold
    if (rtPressed && !rtWasPressed) {
        emit normalSpeedChangeRequested(50);
        qDebug() << "[GAMEPAD] RT basıldı - Normal Speed +50";
        vibrate(0, 5000); // Hafif sağ titreşim
    }
    rtWasPressed = rtPressed;

    // LT (Left Trigger) - Turn speed +50
    BYTE lt = state.Gamepad.bLeftTrigger;
    static bool ltWasPressed = false;
    bool ltPressed = lt > 200; // 200/255 threshold
    if (ltPressed && !ltWasPressed) {
        emit turnSpeedChangeRequested(50);
        qDebug() << "[GAMEPAD] LT basıldı - Turn Speed +50";
        vibrate(5000, 0); // Hafif sol titreşim
    }
    ltWasPressed = ltPressed;

    m_lastButtons = buttons;
}
#else
// Android stub
void GamepadManager::checkButtons(quint32 buttons)
{
    Q_UNUSED(buttons)
    // Android'de gamepad implementasyonu ileride eklenecek
}
#endif
