#ifndef BATTERY_H
#define BATTERY_H

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>

class Battery : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int batteryLevel READ batteryLevel WRITE setBatteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(bool isCharging READ isCharging WRITE setCharging NOTIFY chargingChanged)

public:
    explicit Battery(QWidget *parent = nullptr);

    int batteryLevel() const;
    void setBatteryLevel(int level);
    
    bool isCharging() const;
    void setCharging(bool charging);

signals:
    void batteryLevelChanged(int newLevel);
    void chargingChanged(bool charging);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateAnimation();

private:
    int m_batteryLevel = 75;
    bool m_isCharging = false;
    int m_animationOffset = 0;
    QTimer *m_animationTimer;
    
    QColor getBatteryColor() const;
    void drawBatteryOutline(QPainter &painter);
    void drawBatteryFill(QPainter &painter);
    void drawBatteryTerminal(QPainter &painter);
    void drawBatteryPercentage(QPainter &painter);
    void drawChargingAnimation(QPainter &painter);
};

#endif // BATTERY_H
