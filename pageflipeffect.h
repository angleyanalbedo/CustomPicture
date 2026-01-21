#ifndef PAGEFLIPEFFECT_H
#define PAGEFLIPEFFECT_H

#include <QGraphicsEffect>
#include <QPainter>
#include <QTransform>
#include <QtMath>

class PageFlipEffect : public QGraphicsEffect
{
    Q_OBJECT
    Q_PROPERTY(qreal angle READ angle WRITE setAngle NOTIFY angleChanged)
    Q_PROPERTY(bool flipping READ isFlipping WRITE setFlipping NOTIFY flippingChanged)

public:
    explicit PageFlipEffect(QObject *parent = nullptr);

    // 角度属性（0-90度）
    qreal angle() const;
    void setAngle(qreal angle);

    // 是否正在翻页
    bool isFlipping() const;
    void setFlipping(bool flipping);

    // 翻页方向（true=向右翻，false=向左翻）
    void setDirection(bool toRight);

signals:
    void angleChanged();
    void flippingChanged();

protected:
    void draw(QPainter *painter) override;

private:
    qreal m_angle;      // 当前角度（0-90度）
    bool m_flipping;    // 是否正在翻页
    bool m_direction;   // 翻页方向
};

#endif // PAGEFLIPEFFECT_H
