#include "battery.h"
#include <QPaintEvent>
#include <QLinearGradient>
#include <QFontMetrics>
#include <QTimer>

Battery::Battery(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(50, 30); // Height 25 olarak ayarlandı
    setBatteryLevel(75);
    
    // Animasyon timer'ı oluştur
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &Battery::updateAnimation);
    m_animationTimer->start(100); // 100ms'de bir güncelle
}

int Battery::batteryLevel() const
{
    return m_batteryLevel;
}

void Battery::setBatteryLevel(int level)
{
    if (level < 0) level = 0;
    if (level > 100) level = 100;
    
    if (m_batteryLevel != level) {
        m_batteryLevel = level;
        emit batteryLevelChanged(level);
        
        // Android için güncelleme sıklığını azalt
        #ifdef Q_OS_ANDROID
        static int lastUpdateLevel = -1;
        if (abs(level - lastUpdateLevel) >= 5) { // Sadece %5 değişimde güncelle
            update();
            lastUpdateLevel = level;
        }
        #else
        update(); // Widget'ı yeniden çiz
        #endif
    }
}

bool Battery::isCharging() const
{
    return m_isCharging;
}

void Battery::setCharging(bool charging)
{
    if (m_isCharging != charging) {
        m_isCharging = charging;
        emit chargingChanged(charging);
        
        // Android için güncelleme sıklığını azalt
        #ifdef Q_OS_ANDROID
        static bool lastChargingState = false;
        if (charging != lastChargingState) {
            update();
            lastChargingState = charging;
        }
        #else
        update(); // Widget'ı yeniden çiz
        #endif
    }
}

void Battery::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    // Android için performans optimizasyonu
    #ifdef Q_OS_ANDROID
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    #else
    painter.setRenderHint(QPainter::Antialiasing);
    #endif
    
    // Batarya gövdesini çiz
    drawBatteryOutline(painter);
    
    // Batarya dolgusunu çiz
    drawBatteryFill(painter);
    
    // Batarya ucunu (terminal) çiz
    drawBatteryTerminal(painter);
    
    // Yüzde yazısını her durumda çiz
    drawChargingAnimation(painter);
}

QColor Battery::getBatteryColor() const
{
    // Referans görüntüdeki mavi renk
    return QColor(64, 156, 255); // Tam mavi renk
}

void Battery::drawBatteryOutline(QPainter &painter)
{
    painter.setPen(QPen(QColor(64, 156, 255), 2)); // Mavi çerçeve
    painter.setBrush(QColor(26, 26, 26));
    
    // Ana batarya gövdesi - mini ve toplu
    QRect batteryRect(2, 5, 38, 20);
    painter.drawRoundedRect(batteryRect, 6, 6);
}

void Battery::drawBatteryFill(QPainter &painter)
{
    if (m_batteryLevel <= 0) return;
    
    // Dolgu alanını hesapla - yeni boyutlara göre
    int fillWidth = (38 - 8) * m_batteryLevel / 100; // 8px kenar boşluğu
    
    if (fillWidth > 0) {
        QRect fillRect(4, 7, fillWidth, 16);
        
        // Gradient oluştur - daha sade
        QLinearGradient gradient(fillRect.left(), 0, fillRect.right(), 0);
        QColor batteryColor = getBatteryColor();
        
        gradient.setColorAt(0.0, batteryColor);
        gradient.setColorAt(1.0, batteryColor);
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(gradient);
        painter.drawRoundedRect(fillRect, 4, 4);
    }
}

void Battery::drawBatteryTerminal(QPainter &painter)
{
    // Batarya ucunun pozisyonu - mini ve toplu
    QRect terminalRect(40, 10, 4, 10);
    
    painter.setPen(QPen(QColor(64, 156, 255), 1)); // Mavi çerçeve
    painter.setBrush(QColor(64, 156, 255)); // Mavi dolgu
    painter.drawRoundedRect(terminalRect, 2, 2);
}

void Battery::drawBatteryPercentage(QPainter &painter)
{
    // Yüzde yazısını gösterme - referans tasarımda yok
    Q_UNUSED(painter)
}

void Battery::updateAnimation()
{
    if (m_isCharging) {
        m_animationOffset = (m_animationOffset + 2) % 20; // 0-19 arası döngü
        update(); // Widget'ı yeniden çiz
    }
}

void Battery::drawChargingAnimation(QPainter &painter)
{
    // Her durumda sayı gösterimi (şarj olmasa da)
    if (m_batteryLevel > 0 && m_batteryLevel < 100) {
        // Yüzde metni
        QFont percentFont("Arial", 7, QFont::Normal); // Küçük font
        painter.setFont(percentFont);
        
        // Özel renk: #E0E0E0
        QColor textColor = QColor(224, 224, 224); // #E0E0E0
        painter.setPen(textColor);
        
        QString percentageText = QString::number(m_batteryLevel);
        QRect percentRect(4, 5, 34, 20);
        painter.drawText(percentRect, Qt::AlignCenter, percentageText);
        
        // Şarj olunca yıldırım çiz
        if (m_isCharging) {
            QFont iconFont("Arial", 6);
            painter.setFont(iconFont);
            painter.setPen(QColor(255, 224, 0)); // Sarı yıldırım
            
            // Batarya kenarına küçük yıldırım (biraz daha sağda)
            QRect iconRect(41, 8, 12, 12);
            painter.drawText(iconRect, Qt::AlignCenter, "⚡");
        }
    }
}
