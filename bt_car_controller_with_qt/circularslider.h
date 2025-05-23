#ifndef CIRCULARSLIDER_H
#define CIRCULARSLIDER_H

#include <QWidget>

class CircularSlider : public QWidget
{
    Q_OBJECT
public:
    explicit CircularSlider(QWidget *parent = nullptr);
    int value() const;

signals:
    void valueChanged(int newValue);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;

private:
    int m_value = 0;
    void setValue(int newValue);
    QPointF pointOnCircle(float angle, float radius);
};

#endif // CIRCULARSLIDER_H
