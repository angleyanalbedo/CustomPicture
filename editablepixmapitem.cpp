#include "editablepixmapitem.h"
#include "qgraphicsscene.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <cmath>

EditablePixmapItem::EditablePixmapItem(const QPixmap &pixmap, QGraphicsItem *parent)
    : QGraphicsPixmapItem(pixmap, parent)
    , selected(false)
    , editable(true)
    , dragging(false)
    , resizing(false)
    , activeControlPoint(None)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setAcceptHoverEvents(true);
}

void EditablePixmapItem::setSelected(bool selected)
{
    if (this->selected != selected) {
        this->selected = selected;
        update();
    }
}

void EditablePixmapItem::setEditable(bool editable)
{
    this->editable = editable;
    setFlag(QGraphicsItem::ItemIsMovable, editable);
    setFlag(QGraphicsItem::ItemIsSelectable, editable);
}

void EditablePixmapItem::rotate(qreal angle)
{
    setRotation(rotation() + angle);
}

void EditablePixmapItem::scale(qreal factor)
{
    setScale(QGraphicsItem::scale() * factor);
}

void EditablePixmapItem::crop(const QRect &rect)
{
    QPixmap original = pixmap();
    QPixmap cropped = original.copy(rect);
    setPixmap(cropped);
}

QRectF EditablePixmapItem::boundingRect() const
{
    QRectF rect = QGraphicsPixmapItem::boundingRect();
    if (selected) {
        // 扩大边界以包含控制点
        qreal adjust = 10.0;
        rect.adjust(-adjust, -adjust, adjust, adjust);
    }
    return rect;
}

void EditablePixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsPixmapItem::paint(painter, option, widget);

    if (selected && editable) {
        // 绘制选择边框
        painter->save();
        QPen pen(Qt::blue, 2, Qt::DashLine);
        painter->setPen(pen);
        painter->drawRect(boundingRect().adjusted(2, 2, -2, -2));

        // 绘制控制点
        painter->setBrush(Qt::white);
        painter->setPen(QPen(Qt::black, 1));

        QRectF rect = boundingRect();
        qreal size = 8.0;

        // 四个角
        painter->drawRect(getControlPointRect(TopLeft));
        painter->drawRect(getControlPointRect(TopRight));
        painter->drawRect(getControlPointRect(BottomLeft));
        painter->drawRect(getControlPointRect(BottomRight));

        // 旋转控制点（上方中间）
        QPointF topCenter = rect.topLeft() + QPointF(rect.width() / 2, 0);
        painter->drawEllipse(QRectF(topCenter.x() - size/2, topCenter.y() - size/2 - 20, size, size));

        painter->restore();
    }
}

QRectF EditablePixmapItem::getControlPointRect(ControlPoint point) const
{
    QRectF rect = boundingRect();
    qreal size = 8.0;

    switch (point) {
    case TopLeft:
        return QRectF(rect.topLeft() - QPointF(size/2, size/2), QSizeF(size, size));
    case TopRight:
        return QRectF(rect.topRight() - QPointF(size/2, size/2), QSizeF(size, size));
    case BottomLeft:
        return QRectF(rect.bottomLeft() - QPointF(size/2, size/2), QSizeF(size, size));
    case BottomRight:
        return QRectF(rect.bottomRight() - QPointF(size/2, size/2), QSizeF(size, size));
    default:
        return QRectF();
    }
}

EditablePixmapItem::ControlPoint EditablePixmapItem::getControlPointAt(const QPointF &pos) const
{
    if (getControlPointRect(TopLeft).contains(pos))
        return TopLeft;
    if (getControlPointRect(TopRight).contains(pos))
        return TopRight;
    if (getControlPointRect(BottomLeft).contains(pos))
        return BottomLeft;
    if (getControlPointRect(BottomRight).contains(pos))
        return BottomRight;

    // 检查旋转控制点
    QRectF rect = boundingRect();
    QPointF topCenter = rect.topLeft() + QPointF(rect.width() / 2, 0);
    QRectF rotateRect(topCenter.x() - 5, topCenter.y() - 25, 10, 10);
    if (rotateRect.contains(pos))
        return Rotate;

    return None;
}

void EditablePixmapItem::updateCursor(const QPointF &pos)
{
    ControlPoint point = getControlPointAt(pos);

    switch (point) {
    case TopLeft:
    case BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case TopRight:
    case BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case Rotate:
        setCursor(Qt::OpenHandCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

void EditablePixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!editable) {
        event->ignore();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        QPointF pos = event->pos();
        activeControlPoint = getControlPointAt(pos);

        if (activeControlPoint != None) {
            // 开始调整大小或旋转
            dragStartPos = event->scenePos();
            itemStartPos = scenePos();
            itemStartRotation = rotation();

            if (activeControlPoint == Rotate) {
                // 旋转模式
                setCursor(Qt::ClosedHandCursor);
            } else {
                // 调整大小模式
                resizing = true;
            }
            event->accept();
            return;
        } else {
            // 开始移动
            dragging = true;
            dragStartPos = event->scenePos();
            itemStartPos = scenePos();
        }
    }

    // 发出选择信号
    emit itemSelected(this);

    QGraphicsPixmapItem::mousePressEvent(event);
}

void EditablePixmapItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!editable) {
        event->ignore();
        return;
    }

    if (dragging) {
        // 移动项目
        QPointF delta = event->scenePos() - dragStartPos;
        setPos(itemStartPos + delta);
        scene()->update();
        event->accept();
    } else if (resizing) {
        // 调整大小
        QPointF delta = event->scenePos() - dragStartPos;
        qreal scaleFactor = 1.0;

        // 根据控制点计算缩放
        switch (activeControlPoint) {
        case TopLeft:
            scaleFactor = 1.0 - delta.x() / boundingRect().width();
            break;
        case TopRight:
            scaleFactor = 1.0 + delta.x() / boundingRect().width();
            break;
        case BottomLeft:
            scaleFactor = 1.0 - delta.x() / boundingRect().width();
            break;
        case BottomRight:
            scaleFactor = 1.0 + delta.x() / boundingRect().width();
            break;
        default:
            break;
        }

        // 应用缩放
        setScale(QGraphicsItem::scale() * scaleFactor);

        // 更新起始位置，以便连续调整
        dragStartPos = event->scenePos();
        event->accept();
    } else if (activeControlPoint == Rotate) {
        // 旋转
        QPointF center = scenePos();
        QPointF startVec = dragStartPos - center;
        QPointF currentVec = event->scenePos() - center;

        qreal angle = atan2(currentVec.y(), currentVec.x()) - atan2(startVec.y(), startVec.x());
        angle = angle * 180.0 / PI;

        setRotation(itemStartRotation + angle);
        scene()->update();
        event->accept();
    } else {
        // 更新光标
        updateCursor(event->pos());
    }
}

void EditablePixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!editable) {
        event->ignore();
        return;
    }

    dragging = false;
    resizing = false;
    activeControlPoint = None;
    setCursor(Qt::ArrowCursor);

    QGraphicsPixmapItem::mouseReleaseEvent(event);
}

void EditablePixmapItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    emit itemDoubleClicked(this);
    QGraphicsPixmapItem::mouseDoubleClickEvent(event);
}

void EditablePixmapItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // 可以在这里添加上下文菜单
    event->ignore();
}
