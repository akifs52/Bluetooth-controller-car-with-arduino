#include "joypad.h"
#include "qpainter.h"
#include "QtMath"
#include <QTimer>
QRect upRect, downRect, leftRect, rightRect;

joypad::joypad(QWidget *parent)
    : QWidget{parent}
{
    setMinimumSize(200, 200);
}

void joypad::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    QPoint center(w / 2, h / 2);
    int radius = qMin(w, h) / 2 - 10;

    // Dış daire (joystick tabanı)
    p.setBrush(QColor(40, 40, 40));
    p.drawEllipse(center, radius, radius);

    // Yön vurgusu
    auto drawDirectionHighlight = [&](const QString &dir, double angleDeg) {
        if (pressedDirection == dir) {
            QPointF dirVec(qCos(qDegreesToRadians(angleDeg)), -qSin(qDegreesToRadians(angleDeg)));
            QPointF pos = center + dirVec * (radius * 0.6);
            p.setBrush(QColor(100, 100, 100));
            p.drawEllipse(pos, 15, 15);
        }
    };

    drawDirectionHighlight("U", 90);
    drawDirectionHighlight("R", 0);
    drawDirectionHighlight("D", 270);
    drawDirectionHighlight("L", 180);

    // Vidalama yerleri
    p.setBrush(QColor(80, 80, 80));
    for (int i = 0; i < 4; ++i) {
        double angle = i * M_PI_2;
        int boltX = center.x() + qCos(angle) * radius * 0.6;
        int boltY = center.y() + qSin(angle) * radius * 0.6;
        p.drawEllipse(QPoint(boltX, boltY), 5, 5);
    }

    // Kırmızı top (joystick ucu)
    QPointF redDotPos = center + redDotOffset;
    p.setBrush(Qt::red);
    p.drawEllipse(redDotPos, 6, 6);
}

void joypad::mousePressEvent(QMouseEvent *event)
{
    mouseHeld = true;
    QPoint center(width() / 2, height() / 2);
    QPointF dir = event->pos() - center;

    qreal maxOffset = qMin(width(), height()) / 2 * 0.5;
    if (dir.manhattanLength() > maxOffset) {
        dir = dir / std::sqrt(dir.x()*dir.x() + dir.y()*dir.y()) * maxOffset;
    }

    redDotOffset = dir;

    // Yön belirleme
    QLineF vector(center, event->pos());
    if (vector.length() >= 20) {
        qreal angle = vector.angle();
        QString dirStr;
        if (angle >= 45 && angle < 135)
            dirStr = "U";
        else if (angle >= 135 && angle < 225)
            dirStr = "L";
        else if (angle >= 225 && angle < 315)
            dirStr = "D";
        else
            dirStr = "R";

        pressedDirection = dirStr;
        emit directionPressed(dirStr);

        // Highlight'ı 200ms sonra temizle
        QTimer::singleShot(200, this, [this]() {
            pressedDirection.clear();
            update();
        });
    }

    update();
}

void joypad::mouseMoveEvent(QMouseEvent *event)
{
    if (!mouseHeld) return;

    QPointF center(width() / 2, height() / 2);
    QPointF delta = event->pos() - center;
    redDotOffset = delta;

    QString direction;

    if (qAbs(delta.x()) > qAbs(delta.y())) {
        direction = (delta.x() > 0) ? "R" : "L";
    } else {
        direction = (delta.y() > 0) ? "D" : "U";
    }

    emit directionPressed(direction);

    update(); // joystick’i yeniden çiz
}

void joypad::mouseReleaseEvent(QMouseEvent *)
{
    mouseHeld = false;
    redDotOffset = QPointF(0, 0); // merkeze dön
    emit directionPressed("S"); // stop sinyali gönder
    update();
}
