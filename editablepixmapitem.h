#ifndef EDITABLEPIXMAPITEM_H
#define EDITABLEPIXMAPITEM_H

#define PI 3.14159265358979323846

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

class EditablePixmapItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    explicit EditablePixmapItem(const QPixmap &pixmap, QGraphicsItem *parent = nullptr);

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    void setSelected(bool selected);
    bool isSelected() const { return selected; }

    void setEditable(bool editable);
    bool isEditable() const { return editable; }

    // 变换操作
    void rotate(qreal angle);
    void scale(qreal factor);
    void crop(const QRect &rect);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

signals:
    void itemSelected(EditablePixmapItem *item);
    void itemDoubleClicked(EditablePixmapItem *item);
    void itemDeleted(EditablePixmapItem *item);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    bool selected;
    bool editable;
    bool dragging;
    bool resizing;
    QPointF dragStartPos;
    QPointF itemStartPos;
    qreal itemStartRotation;
    QRectF selectionRect;

    // 控制点
    enum ControlPoint { None, TopLeft, TopRight, BottomLeft, BottomRight, Rotate };
    ControlPoint activeControlPoint;

    QRectF getControlPointRect(ControlPoint point) const;
    ControlPoint getControlPointAt(const QPointF &pos) const;
    void updateCursor(const QPointF &pos);
    //atan2(qreal, qreal);
};

#endif // EDITABLEPIXMAPITEM_H
