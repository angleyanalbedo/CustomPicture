#include "pageflipeffect.h"
PageFlipEffect::PageFlipEffect(QObject *parent)
    : QGraphicsEffect(parent),
    m_angle(0),
    m_flipping(false),
    m_direction(true)
{
}

qreal PageFlipEffect::angle() const
{
    return m_angle;
}

void PageFlipEffect::setAngle(qreal angle)
{
    m_angle = angle;
    update();
    emit angleChanged();
}

bool PageFlipEffect::isFlipping() const
{
    return m_flipping;
}

void PageFlipEffect::setFlipping(bool flipping)
{
    m_flipping = flipping;
    emit flippingChanged();
}

void PageFlipEffect::setDirection(bool toRight)
{
    m_direction = toRight;
}

void PageFlipEffect::draw(QPainter *painter)
{
    QPoint offset;
    QPixmap pix = sourcePixmap(Qt::DeviceCoordinates, &offset);

    if (pix.isNull() || !m_flipping) {
        drawSource(painter);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    int w = pix.width();
    int h = pix.height();

    // 角度 0 ~ 90
    qreal rad = qDegreesToRadians(m_angle);
    qreal scaleX = qCos(rad);               // 关键：宽度压缩
    scaleX = qMax(scaleX, 0.02);             // 防止变成 0

    int visibleW = int(w * scaleX);

    QTransform t;

    if (m_direction) {
        // 👉 从右往左翻（右边固定）
        t.translate(w, 0);
        t.scale(-scaleX, 1.0);
    } else {
        // 👉 从左往右翻（左边固定）
        t.scale(scaleX, 1.0);
    }

    painter->setTransform(t, true);

    // 裁剪只画“还能看到的部分”
    painter->setClipRect(0, 0, visibleW, h);
    painter->drawPixmap(offset, pix);

    // ===== 简单阴影（非常重要）=====
    QLinearGradient shadowGrad(
        m_direction ? w - visibleW : 0, 0,
        m_direction ? w : visibleW, 0
        );
    shadowGrad.setColorAt(0.0, QColor(0, 0, 0, 80));
    shadowGrad.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter->fillRect(0, 0, w, h, shadowGrad);

    painter->restore();
}
