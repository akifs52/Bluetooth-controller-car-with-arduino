#ifndef JOYPAD_H
#define JOYPAD_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QEvent>

class joypad : public QWidget
{
    Q_OBJECT
public:
    explicit joypad(QWidget *parent = nullptr);

signals:
    void directionPressed(const QString &direction);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;

private:
    QString pressedDirection;
    QPointF redDotOffset;
    bool mouseHeld = false;

    void handlePress(const QPointF &pos);
    void handleMove(const QPointF &pos);
    void handleRelease();
};

#endif // JOYPAD_H
