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
    // 如果没有翻页效果，直接绘制原图
    if (m_angle == 0 || !m_flipping) {
        drawSource(painter);
        return;
    }

    QPoint offset;
    // 使用正确的坐标系统
    QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset);

    if (pixmap.isNull()) {
        drawSource(painter);
        return;
    }

    painter->save();

    // 简化：使用简单的2D滑动效果，避免3D计算的复杂性
    QTransform transform;
    int width = pixmap.width();
    int height = pixmap.height();

    // 计算滑动距离（根据角度）
    qreal slideDistance = width * (m_angle / 90.0);

    if (m_direction) {
        // 向右翻页：从右侧滑入
        // 设置新页面从右侧进入
        transform.translate(width - slideDistance, 0);
        painter->setTransform(transform);
        painter->drawPixmap(offset, pixmap);

        // 绘制当前页面向左滑出
        painter->setTransform(QTransform()); // 重置变换
        QTransform currentTransform;
        currentTransform.translate(-slideDistance, 0);
        painter->setTransform(currentTransform);
        // 这里需要绘制当前页面，但需要从其他地方获取
    } else {
        // 向左翻页：从左侧滑入
        // 设置新页面从左侧进入
        transform.translate(-width + slideDistance, 0);
        painter->setTransform(transform);
        painter->drawPixmap(offset, pixmap);

        // 绘制当前页面向右滑出
        painter->setTransform(QTransform()); // 重置变换
        QTransform currentTransform;
        currentTransform.translate(slideDistance, 0);
        painter->setTransform(currentTransform);
    }

    painter->restore();
}
