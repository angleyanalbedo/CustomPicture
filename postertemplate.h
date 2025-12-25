#ifndef POSTERTEMPLATE_H
#define POSTERTEMPLATE_H

#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QVector>
#include <QRectF>
#include <QSize>
#include <QMap>
#include <QString>
#include <QRandomGenerator>

// 照片槽位结构体
struct PhotoSlot {
    QRectF position;     // 在画布中的位置（相对坐标，0-1）
    QRectF sourceRect;   // 源图片裁剪区域（相对坐标，0-1）
    qreal rotation;      // 旋转角度
    qreal scale;         // 缩放比例
    QPointF anchorPoint; // 锚点位置（相对坐标，0-1）
    QString maskType;    // 遮罩类型：矩形、圆形、圆角矩形、自定义形状
    QColor borderColor;  // 边框颜色
    qreal borderWidth;   // 边框宽度
    qreal cornerRadius;  // 圆角半径（仅对圆角矩形有效）
    bool keepAspectRatio; // 保持宽高比

    PhotoSlot()
        : rotation(0.0)
        , scale(1.0)
        , anchorPoint(0.5, 0.5)
        , maskType("rectangle")
        , borderColor(Qt::transparent)
        , borderWidth(0.0)
        , cornerRadius(0.0)
        , keepAspectRatio(true)
    {}

    PhotoSlot(const QRectF& pos, const QRectF& src = QRectF(0, 0, 1, 1),
              qreal rot = 0.0, qreal scl = 1.0)
        : position(pos)
        , sourceRect(src)
        , rotation(rot)
        , scale(scl)
        , anchorPoint(0.5, 0.5)
        , maskType("rectangle")
        , borderColor(Qt::transparent)
        , borderWidth(0.0)
        , cornerRadius(0.0)
        , keepAspectRatio(true)
    {}
};

// 模板类型枚举
enum TemplateType {
    TEMPLATE_4_GRID = 0,      // 四宫格
    TEMPLATE_9_GRID = 1,      // 九宫格
    TEMPLATE_3_HORIZONTAL = 2, // 三图横向
    TEMPLATE_3_VERTICAL = 3,   // 三图竖向
    TEMPLATE_2_HORIZONTAL = 4, // 两图横向
    TEMPLATE_2_VERTICAL = 5,   // 两图竖向
    TEMPLATE_HEART = 6,        // 心形布局
    TEMPLATE_CIRCLE = 7,       // 圆形布局
    TEMPLATE_SPIRAL = 8,       // 螺旋布局
    TEMPLATE_COLLAGE = 9,      // 拼贴风
    TEMPLATE_FREE = 10,        // 自由布局
    TEMPLATE_CUSTOM = 11       // 自定义
};

class PosterTemplate : public QObject
{
    Q_OBJECT

public:
    explicit PosterTemplate(QObject *parent = nullptr);
    ~PosterTemplate();

    // 模板管理
    void setTemplateType(TemplateType type);
    TemplateType getTemplateType() const { return m_templateType; }

    QString getTemplateName() const;
    static QString getTemplateName(TemplateType type);
    static QStringList getAvailableTemplates();

    // 槽位管理
    void addPhotoSlot(const PhotoSlot &slot);
    void removePhotoSlot(int index);
    void clearPhotoSlots();
    void updatePhotoSlot(int index, const PhotoSlot &slot);

    QVector<PhotoSlot> getPhotoSlots() const { return m_photoSlots; }
    int getSlotCount() const { return m_photoSlots.size(); }

    // 海报生成
    QPixmap generatePoster(const QVector<QPixmap> &photos,
                           const QSize &outputSize = QSize(1080, 1920),
                           const QColor &backgroundColor = Qt::white,
                           const QPixmap &backgroundImage = QPixmap());

    QPixmap generatePreview(const QVector<QPixmap> &photos,
                            const QSize &previewSize = QSize(400, 600));

    // 模板参数设置
    void setPosterSize(const QSize &size) { m_posterSize = size; }
    QSize getPosterSize() const { return m_posterSize; }

    void setSpacing(qreal spacing) { m_spacing = spacing; }
    qreal getSpacing() const { return m_spacing; }

    void setMargin(qreal margin) { m_margin = margin; }
    qreal getMargin() const { return m_margin; }

    void setRandomRotation(bool enabled) { m_randomRotation = enabled; }
    bool getRandomRotation() const { return m_randomRotation; }

    void setRandomScale(bool enabled) { m_randomScale = enabled; }
    bool getRandomScale() const { return m_randomScale; }

    // 模板保存/加载
    bool saveTemplate(const QString &filePath);
    bool loadTemplate(const QString &filePath);

    // 预设模板
    void setup4GridTemplate();
    void setup9GridTemplate();
    void setup3HorizontalTemplate();
    void setup3VerticalTemplate();
    void setupHeartTemplate();
    void setupCircleTemplate();
    void setupSpiralTemplate(int count);
    void setupCollageTemplate(int count);

    // 静态方法：创建预设模板
    static PosterTemplate* createTemplate(TemplateType type, QObject *parent = nullptr);

signals:
    void templateChanged();
    void generationProgress(int percent);

private:
    TemplateType m_templateType;
    QVector<PhotoSlot> m_photoSlots;
    QSize m_posterSize;

    // 布局参数
    qreal m_spacing;
    qreal m_margin;
    bool m_randomRotation;
    bool m_randomScale;

    // 私有方法
    void setupDefaultSlots();

    // 绘制方法
    void drawBackground(QPainter &painter, const QColor &bgColor, const QPixmap &bgImage);
    QPixmap processPhoto(const QPixmap &original, const PhotoSlot &slot, const QSize &targetSize);
    void drawPhoto(QPainter &painter, const QPixmap &photo, const PhotoSlot &slot, const QRectF &targetRect);
    void drawBorder(QPainter &painter, const PhotoSlot &slot, const QRectF &rect);
    void applyMask(QPainter &painter, const QString &maskType, const QRectF &rect, qreal cornerRadius);

    // 工具方法
    QRectF calculateAbsoluteRect(const QRectF &relativeRect, const QSize &canvasSize);
    QRectF calculateSourceRect(const QRectF &relativeRect, const QSize &photoSize);
    qreal getRandomRotationAngle() const;
    qreal getRandomScaleFactor() const;

    // 布局算法
    QVector<QRectF> calculateGridLayout(int rows, int cols, const QSize &canvasSize);
    QVector<QRectF> calculateCircleLayout(int count, const QSize &canvasSize);
    QVector<QRectF> calculateHeartLayout(int count, const QSize &canvasSize);
    QVector<QRectF> calculateSpiralLayout(int count, const QSize &canvasSize);

    // 遮罩路径
    QPainterPath createRectanglePath(const QRectF &rect, qreal cornerRadius = 0);
    QPainterPath createCirclePath(const QRectF &rect);
    QPainterPath createHeartPath(const QRectF &rect);
    QPainterPath createStarPath(const QRectF &rect, int points = 5);
};

#endif // POSTERTEMPLATE_H
