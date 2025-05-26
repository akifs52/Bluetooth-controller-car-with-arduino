#include "circularslider.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>

CircularSlider::CircularSlider(QWidget *parent)
    : QWidget{parent}
{

   setMinimumSize(150, 150);
}


int CircularSlider::value() const
{
    return m_value;
}

void CircularSlider::setValue(int newValue)
{
    if (newValue < 0) newValue = 0;
    if (newValue > 255) newValue = 255;
    if (newValue != m_value) {
        m_value = newValue;
        emit valueChanged(m_value);
        update();
    }
}

void CircularSlider::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int size = qMin(width(), height());
    QPoint center = rect().center();
    float radius = size / 2.5;

    QRectF circleRect(center.x() - radius, center.y() - radius, 2*radius, 2*radius);

    float angle = (m_value / 255.0f) * 360.0f;
    float startAngle = 90;

    // Background circle
    p.setPen(QPen(QColor(80, 80, 80), 6));
    p.drawArc(circleRect, 0, 360 * 16);

    // Progress arc
    p.setPen(QPen(progressColor, 10, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(circleRect, (startAngle - angle) * 16, angle * 16);

    // Knob
    QPointF knobCenter = pointOnCircle(90 - angle, radius);
    p.setBrush(Qt::black);
    p.setPen(Qt::NoPen);
    p.drawEllipse(knobCenter, 6, 6);

    // Value text
    p.setPen(Qt::lightGray);
    QFont f = p.font();
    f.setPointSize(16);
    p.setFont(f);
    p.drawText(rect(), Qt::AlignCenter, QString::number(m_value));
}

QPointF CircularSlider::pointOnCircle(float angleDeg, float radius)
{
    float rad = qDegreesToRadians(angleDeg);
    float x = width() / 2 + radius * qCos(rad);
    float y = height() / 2 - radius * qSin(rad);
    return QPointF(x, y);
}

void CircularSlider::mousePressEvent(QMouseEvent *event)
{
    mouseMoveEvent(event);
}

void CircularSlider::mouseMoveEvent(QMouseEvent *event)
{
    QPoint center = rect().center();
    QPointF delta = event->pos() - center;
    float angle = std::atan2(-delta.y(), delta.x()) * 180.0f / M_PI;

    angle = 90 - angle;
    if (angle < 0) angle += 360;
    int newValue = qRound(angle / 360.0f * 255);
    setValue(newValue);
}

void CircularSlider::setProgressColor(const QColor &color)
{
    progressColor = color;
    update(); // yeniden çiz
}
