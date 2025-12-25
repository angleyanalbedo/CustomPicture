#include "postertemplate.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>
#include <QDebug>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRandomGenerator>

PosterTemplate::PosterTemplate(QObject *parent)
    : QObject(parent)
    , m_templateType(TEMPLATE_4_GRID)
    , m_posterSize(1080, 1920)
    , m_spacing(10.0)
    , m_margin(20.0)
    , m_randomRotation(false)
    , m_randomScale(false)
{
    setupDefaultSlots();
}

PosterTemplate::~PosterTemplate()
{
}

void PosterTemplate::setTemplateType(TemplateType type)
{
    if (m_templateType != type) {
        m_templateType = type;
        setupDefaultSlots();
        emit templateChanged();
    }
}

QString PosterTemplate::getTemplateName() const
{
    return getTemplateName(m_templateType);
}

QString PosterTemplate::getTemplateName(TemplateType type)
{
    static QMap<TemplateType, QString> templateNames = {
        {TEMPLATE_4_GRID, "四宫格"},
        {TEMPLATE_9_GRID, "九宫格"},
        {TEMPLATE_3_HORIZONTAL, "三图横向"},
        {TEMPLATE_3_VERTICAL, "三图竖向"},
        {TEMPLATE_2_HORIZONTAL, "两图横向"},
        {TEMPLATE_2_VERTICAL, "两图竖向"},
        {TEMPLATE_HEART, "心形布局"},
        {TEMPLATE_CIRCLE, "圆形布局"},
        {TEMPLATE_SPIRAL, "螺旋布局"},
        {TEMPLATE_COLLAGE, "拼贴风"},
        {TEMPLATE_FREE, "自由布局"},
        {TEMPLATE_CUSTOM, "自定义"}
    };

    return templateNames.value(type, "未知模板");
}

QStringList PosterTemplate::getAvailableTemplates()
{
    return {
        "四宫格",
        "九宫格",
        "三图横向",
        "三图竖向",
        "两图横向",
        "两图竖向",
        "心形布局",
        "圆形布局",
        "螺旋布局",
        "拼贴风",
        "自由布局",
        "自定义"
    };
}

void PosterTemplate::setupDefaultSlots()
{
    clearPhotoSlots();

    switch (m_templateType) {
    case TEMPLATE_4_GRID:
        setup4GridTemplate();
        break;
    case TEMPLATE_9_GRID:
        setup9GridTemplate();
        break;
    case TEMPLATE_3_HORIZONTAL:
        setup3HorizontalTemplate();
        break;
    case TEMPLATE_3_VERTICAL:
        setup3VerticalTemplate();
        break;
    case TEMPLATE_2_HORIZONTAL:
        addPhotoSlot(PhotoSlot(QRectF(0.0, 0.25, 0.5, 0.5)));
        addPhotoSlot(PhotoSlot(QRectF(0.5, 0.25, 0.5, 0.5)));
        break;
    case TEMPLATE_2_VERTICAL:
        addPhotoSlot(PhotoSlot(QRectF(0.25, 0.0, 0.5, 0.5)));
        addPhotoSlot(PhotoSlot(QRectF(0.25, 0.5, 0.5, 0.5)));
        break;
    case TEMPLATE_HEART:
        setupHeartTemplate();
        break;
    case TEMPLATE_CIRCLE:
        setupCircleTemplate();
        break;
    case TEMPLATE_SPIRAL:
        setupSpiralTemplate(9);
        break;
    case TEMPLATE_COLLAGE:
        setupCollageTemplate(6);
        break;
    case TEMPLATE_FREE:
        // 自由布局开始时为空
        break;
    case TEMPLATE_CUSTOM:
        // 自定义布局保持原有槽位
        break;
    }
}

void PosterTemplate::setup4GridTemplate()
{
    clearPhotoSlots();

    // 四宫格布局
    addPhotoSlot(PhotoSlot(QRectF(0.0, 0.0, 0.5, 0.5)));
    addPhotoSlot(PhotoSlot(QRectF(0.5, 0.0, 0.5, 0.5)));
    addPhotoSlot(PhotoSlot(QRectF(0.0, 0.5, 0.5, 0.5)));
    addPhotoSlot(PhotoSlot(QRectF(0.5, 0.5, 0.5, 0.5)));
}

void PosterTemplate::setup9GridTemplate()
{
    clearPhotoSlots();

    // 九宫格布局
    qreal cellSize = 1.0 / 3.0;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            QRectF rect(col * cellSize, row * cellSize, cellSize, cellSize);
            addPhotoSlot(PhotoSlot(rect));
        }
    }
}

void PosterTemplate::setup3HorizontalTemplate()
{
    clearPhotoSlots();

    // 三图横向布局
    qreal width = 1.0 / 3.0;
    for (int i = 0; i < 3; ++i) {
        QRectF rect(i * width, 0.25, width, 0.5);
        addPhotoSlot(PhotoSlot(rect));
    }
}

void PosterTemplate::setup3VerticalTemplate()
{
    clearPhotoSlots();

    // 三图竖向布局
    qreal height = 1.0 / 3.0;
    for (int i = 0; i < 3; ++i) {
        QRectF rect(0.25, i * height, 0.5, height);
        addPhotoSlot(PhotoSlot(rect));
    }
}

void PosterTemplate::setupHeartTemplate()
{
    clearPhotoSlots();

    // 心形布局参数
    int photoCount = 9;
    QSizeF heartSize(0.8, 0.8);
    QPointF heartCenter(0.5, 0.5);

    // 生成心形坐标
    QVector<QPointF> heartPoints;
    for (int i = 0; i < photoCount; ++i) {
        qreal t = static_cast<qreal>(i) / photoCount * 2 * M_PI;
        qreal x = 16 * qPow(qSin(t), 3);
        qreal y = 13 * qCos(t) - 5 * qCos(2*t) - 2 * qCos(3*t) - qCos(4*t);

        // 归一化到0-1
        x = (x + 16) / 32;  // x范围大约-16到16
        y = (y + 18) / 36;  // y范围大约-18到18

        // 调整到心形区域内
        x = heartCenter.x() + (x - 0.5) * heartSize.width();
        y = heartCenter.y() + (y - 0.5) * heartSize.height();

        QRectF rect(x - 0.05, y - 0.05, 0.1, 0.1);
        PhotoSlot slot(rect);
        slot.maskType = "circle";
        slot.borderColor = QColor(255, 255, 255, 200);
        slot.borderWidth = 2.0;

        addPhotoSlot(slot);
    }
}

void PosterTemplate::setupCircleTemplate()
{
    clearPhotoSlots();

    int photoCount = 8;
    qreal radius = 0.35;
    QPointF center(0.5, 0.5);

    for (int i = 0; i < photoCount; ++i) {
        qreal angle = static_cast<qreal>(i) / photoCount * 2 * M_PI;
        qreal x = center.x() + radius * qCos(angle) - 0.05;
        qreal y = center.y() + radius * qSin(angle) - 0.05;

        QRectF rect(x, y, 0.1, 0.1);
        PhotoSlot slot(rect);
        slot.maskType = "circle";
        slot.rotation = angle * 180 / M_PI;

        addPhotoSlot(slot);
    }
}

void PosterTemplate::setupSpiralTemplate(int count)
{
    clearPhotoSlots();

    QPointF center(0.5, 0.5);
    qreal angleStep = 2 * M_PI / 7;  // 每次旋转角度
    qreal radiusStep = 0.05;         // 半径增量
    qreal startRadius = 0.1;

    for (int i = 0; i < count; ++i) {
        qreal radius = startRadius + i * radiusStep;
        qreal angle = i * angleStep;

        qreal x = center.x() + radius * qCos(angle) - 0.04;
        qreal y = center.y() + radius * qSin(angle) - 0.04;

        QRectF rect(x, y, 0.08, 0.08);
        PhotoSlot slot(rect);
        slot.maskType = "circle";
        slot.rotation = angle * 180 / M_PI;

        if (m_randomScale) {
            slot.scale = 0.8 + QRandomGenerator::global()->bounded(0.4);
        }

        addPhotoSlot(slot);
    }
}

void PosterTemplate::setupCollageTemplate(int count)
{
    clearPhotoSlots();

    // 随机拼贴布局
    for (int i = 0; i < count; ++i) {
        qreal width = 0.2 + QRandomGenerator::global()->bounded(0.2);
        qreal height = 0.2 + QRandomGenerator::global()->bounded(0.2);
        qreal x = QRandomGenerator::global()->bounded(1.0 - width);
        qreal y = QRandomGenerator::global()->bounded(1.0 - height);

        PhotoSlot slot(QRectF(x, y, width, height));

        // 随机旋转
        if (m_randomRotation) {
            slot.rotation = QRandomGenerator::global()->bounded(360.0) - 180.0;
        }

        // 随机遮罩
        int maskType = QRandomGenerator::global()->bounded(3);
        switch (maskType) {
        case 0:
            slot.maskType = "rectangle";
            slot.cornerRadius = QRandomGenerator::global()->bounded(20.0);
            break;
        case 1:
            slot.maskType = "circle";
            break;
        case 2:
            slot.maskType = "rounded";
            slot.cornerRadius = 15.0;
            break;
        }

        // 随机边框
        if (QRandomGenerator::global()->bounded(100) < 30) {  // 30%概率有边框
            slot.borderWidth = 1.0 + QRandomGenerator::global()->bounded(3.0);
            int colorIndex = QRandomGenerator::global()->bounded(6);
            QColor colors[] = {
                QColor(255, 255, 255),
                QColor(0, 0, 0),
                QColor(255, 200, 200),
                QColor(200, 255, 200),
                QColor(200, 200, 255),
                QColor(255, 255, 200)
            };
            slot.borderColor = colors[colorIndex];
        }

        addPhotoSlot(slot);
    }
}

void PosterTemplate::addPhotoSlot(const PhotoSlot &slot)
{
    m_photoSlots.append(slot);
}

void PosterTemplate::removePhotoSlot(int index)
{
    if (index >= 0 && index < m_photoSlots.size()) {
        m_photoSlots.remove(index);
    }
}

void PosterTemplate::clearPhotoSlots()
{
    m_photoSlots.clear();
}

void PosterTemplate::updatePhotoSlot(int index, const PhotoSlot &slot)
{
    if (index >= 0 && index < m_photoSlots.size()) {
        m_photoSlots[index] = slot;
    }
}

QPixmap PosterTemplate::generatePoster(const QVector<QPixmap> &photos,
                                       const QSize &outputSize,
                                       const QColor &backgroundColor,
                                       const QPixmap &backgroundImage)
{
    if (photos.isEmpty()) {
        return QPixmap();
    }

    // 创建画布
    QPixmap poster(outputSize);
    poster.fill(Qt::transparent);

    QPainter painter(&poster);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 绘制背景
    drawBackground(painter, backgroundColor, backgroundImage);

    // 绘制照片
    int slotCount = qMin(m_photoSlots.size(), photos.size());
    for (int i = 0; i < slotCount; ++i) {
        const PhotoSlot &slot = m_photoSlots[i];
        const QPixmap &photo = photos[i];

        // 计算实际位置
        QRectF targetRect = calculateAbsoluteRect(slot.position, outputSize);

        // 处理照片
        QPixmap processedPhoto = processPhoto(photo, slot, targetRect.size().toSize());

        // 绘制照片
        drawPhoto(painter, processedPhoto, slot, targetRect);

        // 发送进度信号
        emit generationProgress((i + 1) * 100 / slotCount);
    }

    painter.end();

    return poster;
}

QPixmap PosterTemplate::generatePreview(const QVector<QPixmap> &photos, const QSize &previewSize)
{
    return generatePoster(photos, previewSize, Qt::white);
}

void PosterTemplate::drawBackground(QPainter &painter, const QColor &bgColor, const QPixmap &bgImage)
{
    if (!bgImage.isNull()) {
        // 绘制背景图片
        painter.drawPixmap(0, 0, painter.device()->width(), painter.device()->height(), bgImage);
    } else if (bgColor.isValid()) {
        // 绘制纯色背景
        painter.fillRect(0, 0, painter.device()->width(), painter.device()->height(), bgColor);
    }
}

QPixmap PosterTemplate::processPhoto(const QPixmap &original, const PhotoSlot &slot, const QSize &targetSize)
{
    if (original.isNull() || targetSize.isEmpty()) {
        return QPixmap();
    }

    QPixmap photo = original;

    // 1. 裁剪源区域
    if (slot.sourceRect.isValid() &&
        (slot.sourceRect != QRectF(0, 0, 1, 1))) {
        QRect sourceRect = calculateSourceRect(slot.sourceRect, original.size()).toRect();
        photo = original.copy(sourceRect);
    }

    // 2. 缩放
    QSize scaledSize = targetSize * slot.scale;
    if (slot.keepAspectRatio) {
        photo = photo.scaled(scaledSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    } else {
        photo = photo.scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // 3. 旋转
    if (qAbs(slot.rotation) > 0.01) {
        QTransform transform;
        transform.rotate(slot.rotation);
        photo = photo.transformed(transform, Qt::SmoothTransformation);
    }

    return photo;
}

void PosterTemplate::drawPhoto(QPainter &painter, const QPixmap &photo, const PhotoSlot &slot, const QRectF &targetRect)
{
    if (photo.isNull()) return;

    // 保存 painter 状态
    painter.save();

    // 应用遮罩
    applyMask(painter, slot.maskType, targetRect, slot.cornerRadius);

    // 计算绘制位置（考虑锚点）
    QPointF drawPos = targetRect.topLeft();
    if (slot.anchorPoint != QPointF(0.5, 0.5)) {
        QSizeF offset = QSizeF(targetRect.width()  * (0.5 - slot.anchorPoint.x()),
                               targetRect.height() * (0.5 - slot.anchorPoint.y()));
        drawPos += QPointF(offset.width(), offset.height());
    }

    // 绘制照片
    painter.drawPixmap(drawPos, photo);

    // 恢复 painter 状态
    painter.restore();

    // 绘制边框
    drawBorder(painter, slot, targetRect);
}

void PosterTemplate::drawBorder(QPainter &painter, const PhotoSlot &slot, const QRectF &rect)
{
    if (slot.borderWidth > 0 && slot.borderColor.alpha() > 0) {
        painter.save();

        QPen pen(slot.borderColor);
        pen.setWidthF(slot.borderWidth);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        if (slot.maskType == "circle") {
            painter.drawEllipse(rect);
        } else {
            if (slot.cornerRadius > 0) {
                painter.drawRoundedRect(rect, slot.cornerRadius, slot.cornerRadius);
            } else {
                painter.drawRect(rect);
            }
        }

        painter.restore();
    }
}

void PosterTemplate::applyMask(QPainter &painter, const QString &maskType, const QRectF &rect, qreal cornerRadius)
{
    QPainterPath clipPath;

    if (maskType == "circle") {
        clipPath.addEllipse(rect);
    } else if (maskType == "heart") {
        clipPath = createHeartPath(rect);
    } else if (maskType == "star") {
        clipPath = createStarPath(rect, 5);
    } else if (maskType == "rounded") {
        clipPath.addRoundedRect(rect, cornerRadius, cornerRadius);
    } else {
        // 矩形或默认
        clipPath.addRect(rect);
    }

    painter.setClipPath(clipPath);
}

QRectF PosterTemplate::calculateAbsoluteRect(const QRectF &relativeRect, const QSize &canvasSize)
{
    qreal x = relativeRect.x() * canvasSize.width();
    qreal y = relativeRect.y() * canvasSize.height();
    qreal width = relativeRect.width() * canvasSize.width();
    qreal height = relativeRect.height() * canvasSize.height();

    return QRectF(x, y, width, height);
}

QRectF PosterTemplate::calculateSourceRect(const QRectF &relativeRect, const QSize &photoSize)
{
    qreal x = relativeRect.x() * photoSize.width();
    qreal y = relativeRect.y() * photoSize.height();
    qreal width = relativeRect.width() * photoSize.width();
    qreal height = relativeRect.height() * photoSize.height();

    return QRectF(x, y, width, height);
}

qreal PosterTemplate::getRandomRotationAngle() const
{
    if (m_randomRotation) {
        return QRandomGenerator::global()->bounded(360.0) - 180.0;
    }
    return 0.0;
}

qreal PosterTemplate::getRandomScaleFactor() const
{
    if (m_randomScale) {
        return 0.7 + QRandomGenerator::global()->bounded(0.6); // 0.7-1.3
    }
    return 1.0;
}

QPainterPath PosterTemplate::createRectanglePath(const QRectF &rect, qreal cornerRadius)
{
    QPainterPath path;
    if (cornerRadius > 0) {
        path.addRoundedRect(rect, cornerRadius, cornerRadius);
    } else {
        path.addRect(rect);
    }
    return path;
}

QPainterPath PosterTemplate::createCirclePath(const QRectF &rect)
{
    QPainterPath path;
    path.addEllipse(rect);
    return path;
}

QPainterPath PosterTemplate::createHeartPath(const QRectF &rect)
{
    QPainterPath path;

    qreal width = rect.width();
    qreal height = rect.height();
    qreal centerX = rect.center().x();
    qreal centerY = rect.center().y();

    // 绘制心形
    QPointF topPoint(centerX, rect.top() + height * 0.25);
    QPointF leftControl(rect.left(), rect.top());
    QPointF rightControl(rect.right(), rect.top());
    QPointF bottomPoint(centerX, rect.bottom());

    path.moveTo(topPoint);
    path.cubicTo(leftControl, QPointF(rect.left(), centerY), bottomPoint);
    path.cubicTo(QPointF(rect.right(), centerY), rightControl, topPoint);

    return path;
}

QPainterPath PosterTemplate::createStarPath(const QRectF &rect, int points)
{
    QPainterPath path;

    qreal centerX = rect.center().x();
    qreal centerY = rect.center().y();
    qreal outerRadius = qMin(rect.width(), rect.height()) / 2;
    qreal innerRadius = outerRadius * 0.5;

    for (int i = 0; i <= points * 2; ++i) {
        qreal angle = M_PI / points * i;
        qreal radius = (i % 2 == 0) ? outerRadius : innerRadius;

        qreal x = centerX + radius * qCos(angle - M_PI_2);
        qreal y = centerY + radius * qSin(angle - M_PI_2);

        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }

    path.closeSubpath();
    return path;
}

bool PosterTemplate::saveTemplate(const QString &filePath)
{
    QJsonObject root;

    // 保存模板类型
    root["templateType"] = static_cast<int>(m_templateType);

    // 保存模板参数
    QJsonObject params;
    params["posterWidth"] = m_posterSize.width();
    params["posterHeight"] = m_posterSize.height();
    params["spacing"] = m_spacing;
    params["margin"] = m_margin;
    params["randomRotation"] = m_randomRotation;
    params["randomScale"] = m_randomScale;
    root["parameters"] = params;

    // 保存槽位信息
    QJsonArray slotsArray;
    for (const PhotoSlot &slot : m_photoSlots) {
        QJsonObject slotObj;

        QJsonObject posObj;
        posObj["x"] = slot.position.x();
        posObj["y"] = slot.position.y();
        posObj["width"] = slot.position.width();
        posObj["height"] = slot.position.height();
        slotObj["position"] = posObj;

        QJsonObject srcObj;
        srcObj["x"] = slot.sourceRect.x();
        srcObj["y"] = slot.sourceRect.y();
        srcObj["width"] = slot.sourceRect.width();
        srcObj["height"] = slot.sourceRect.height();
        slotObj["sourceRect"] = srcObj;

        slotObj["rotation"] = slot.rotation;
        slotObj["scale"] = slot.scale;
        slotObj["maskType"] = slot.maskType;
        slotObj["borderWidth"] = slot.borderWidth;
        slotObj["cornerRadius"] = slot.cornerRadius;
        slotObj["keepAspectRatio"] = slot.keepAspectRatio;

        QJsonObject borderColor;
        borderColor["r"] = slot.borderColor.red();
        borderColor["g"] = slot.borderColor.green();
        borderColor["b"] = slot.borderColor.blue();
        borderColor["a"] = slot.borderColor.alpha();
        slotObj["borderColor"] = borderColor;

        slotsArray.append(slotObj);
    }
    root["slots"] = slotsArray;

    // 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();

    return true;
}

bool PosterTemplate::loadTemplate(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        return false;
    }

    QJsonObject root = doc.object();

    // 加载模板类型
    m_templateType = static_cast<TemplateType>(root["templateType"].toInt());

    // 加载模板参数
    QJsonObject params = root["parameters"].toObject();
    m_posterSize.setWidth(params["posterWidth"].toInt());
    m_posterSize.setHeight(params["posterHeight"].toInt());
    m_spacing = params["spacing"].toDouble();
    m_margin = params["margin"].toDouble();
    m_randomRotation = params["randomRotation"].toBool();
    m_randomScale = params["randomScale"].toBool();

    // 加载槽位信息
    clearPhotoSlots();
    QJsonArray slotsArray = root["slots"].toArray();
    for (const QJsonValue &slotValue : slotsArray) {
        QJsonObject slotObj = slotValue.toObject();

        PhotoSlot slot;

        QJsonObject posObj = slotObj["position"].toObject();
        slot.position = QRectF(
            posObj["x"].toDouble(),
            posObj["y"].toDouble(),
            posObj["width"].toDouble(),
            posObj["height"].toDouble()
            );

        QJsonObject srcObj = slotObj["sourceRect"].toObject();
        slot.sourceRect = QRectF(
            srcObj["x"].toDouble(),
            srcObj["y"].toDouble(),
            srcObj["width"].toDouble(),
            srcObj["height"].toDouble()
            );

        slot.rotation = slotObj["rotation"].toDouble();
        slot.scale = slotObj["scale"].toDouble();
        slot.maskType = slotObj["maskType"].toString();
        slot.borderWidth = slotObj["borderWidth"].toDouble();
        slot.cornerRadius = slotObj["cornerRadius"].toDouble();
        slot.keepAspectRatio = slotObj["keepAspectRatio"].toBool();

        QJsonObject borderColor = slotObj["borderColor"].toObject();
        slot.borderColor = QColor(
            borderColor["r"].toInt(),
            borderColor["g"].toInt(),
            borderColor["b"].toInt(),
            borderColor["a"].toInt()
            );

        addPhotoSlot(slot);
    }

    emit templateChanged();
    return true;
}

PosterTemplate* PosterTemplate::createTemplate(TemplateType type, QObject *parent)
{
    PosterTemplate* templateObj = new PosterTemplate(parent);
    templateObj->setTemplateType(type);
    return templateObj;
}
