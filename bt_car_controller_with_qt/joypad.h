#ifndef JOYPAD_H
#define JOYPAD_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>


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

private:
    QString pressedDirection;

    QPointF redDotOffset; // <-- kırmızı noktanın merkezden sapması
    bool mouseHeld = false; // <-- mouse basılı mı kontrolü


};





#endif // JOYPAD_H
