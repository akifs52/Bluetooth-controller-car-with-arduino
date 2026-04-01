#include "joypad.h"
#include "qpainter.h"
#include "QtMath"
#include <QTimer>
#include <QTouchEvent>

joypad::joypad(QWidget *parent)
    : QWidget{parent}
{
    setMinimumSize(160, 160);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setMouseTracking(true);
}

void joypad::paintEvent(QPaintEvent *)
{
    QPainter p(this);
#ifdef Q_OS_ANDROID
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setRenderHint(QPainter::SmoothPixmapTransform, false);
#else
    p.setRenderHint(QPainter::Antialiasing);
#endif

    int w = width();
    int h = height();
    QPoint center(w / 2, h / 2);
    int radius = qMin(w, h) / 2 - 10;

    p.setBrush(QColor(40, 40, 40));
    p.drawEllipse(center, radius, radius);

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

    p.setBrush(QColor(80, 80, 80));
    for (int i = 0; i < 4; ++i) {
        double angle = i * M_PI_2;
        int boltX = center.x() + qCos(angle) * radius * 0.6;
        int boltY = center.y() + qSin(angle) * radius * 0.6;
        p.drawEllipse(QPoint(boltX, boltY), 5, 5);
    }

    QPointF redDotPos = center + redDotOffset;
    p.setBrush(Qt::red);
    p.drawEllipse(redDotPos, 12, 12);
}

void joypad::mousePressEvent(QMouseEvent *event)
{
    handlePress(event->pos());
}

void joypad::mouseMoveEvent(QMouseEvent *event)
{
    handleMove(event->pos());
}

void joypad::mouseReleaseEvent(QMouseEvent *)
{
    handleRelease();
}

bool joypad::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate: {
            auto *touch = static_cast<QTouchEvent *>(event);
            if (touch->points().isEmpty())
                break;
            const QPointF pos = touch->points().first().position();
            if (event->type() == QEvent::TouchBegin)
                handlePress(pos);
            else
                handleMove(pos);
            event->accept();
            return true;
        }
        case QEvent::TouchEnd:
        case QEvent::TouchCancel:
            handleRelease();
            event->accept();
            return true;
        default:
            break;
    }

    return QWidget::event(event);
}

void joypad::handlePress(const QPointF &pos)
{
    mouseHeld = true;

    QPointF center(width() / 2.0, height() / 2.0);
    QPointF dir = pos - center;

    qreal maxOffset = qMin(width(), height()) / 2.0 * 0.6;
    if (QLineF(QPointF(0, 0), dir).length() > maxOffset) {
        dir = dir / std::sqrt(dir.x() * dir.x() + dir.y() * dir.y()) * maxOffset;
    }

    redDotOffset = dir;

    QLineF vector(center, pos);
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

        QTimer::singleShot(200, this, [this]() {
            pressedDirection.clear();
            update();
        });
    }

    update();
}

void joypad::handleMove(const QPointF &pos)
{
    if (!mouseHeld)
        return;

    QPointF center(width() / 2.0, height() / 2.0);
    QPointF delta = pos - center;

    qreal maxOffset = qMin(width(), height()) / 2.0 * 0.6;
    if (QLineF(QPointF(0, 0), delta).length() > maxOffset) {
        delta = delta / std::sqrt(delta.x() * delta.x() + delta.y() * delta.y()) * maxOffset;
    }

    redDotOffset = delta;

    QString direction;
    if (qAbs(delta.x()) > qAbs(delta.y())) {
        direction = (delta.x() > 0) ? "R" : "L";
    } else {
        direction = (delta.y() > 0) ? "D" : "U";
    }

    emit directionPressed(direction);
    update();
}

void joypad::handleRelease()
{
    mouseHeld = false;
    redDotOffset = QPointF(0, 0);
    emit directionPressed("S");
    update();
}
