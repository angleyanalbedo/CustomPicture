#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

#include <QObject>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QColor>
#include <QVector>
#include <QRect>
#include <QRgb>
#include <QtMath>
#include <QTransform>
#include <QConicalGradient>
#include <QRadialGradient>
#include <QLinearGradient>

// 滤镜类型枚举
enum FilterType {
    FILTER_NONE = 0,
    FILTER_GRAYSCALE,      // 灰度
    FILTER_SEPIA,          // 怀旧
    FILTER_VINTAGE,        // 复古
    FILTER_BRIGHTNESS,     // 亮度调整
    FILTER_CONTRAST,       // 对比度
    FILTER_SATURATION,     // 饱和度
    FILTER_TEMPERATURE,    // 色温
    FILTER_INVERT,         // 反色
    FILTER_BLUR,           // 模糊
    FILTER_SHARPEN,        // 锐化
    FILTER_EMBOSS,         // 浮雕
    FILTER_EDGE_DETECT,    // 边缘检测
    FILTER_OIL_PAINT,      // 油画
    FILTER_PENCIL_SKETCH,  // 铅笔素描
    FILTER_CARTOON,        // 卡通
    FILTER_HDR,            // HDR效果
    FILTER_VIGNETTE,       // 暗角
    FILTER_TILT_SHIFT,     // 移轴
    FILTER_CROSS_PROCESS,  // 交叉冲印
    FILTER_LOMO,           // LOMO风格
    FILTER_CINEMATIC,      // 电影感
    FILTER_PORTRAIT        // 人像美化
};

// 混合模式枚举
enum BlendMode {
    Normal,
    Multiply,
    Screen,
    Overlay,
    SoftLight,
    HardLight,
    ColorDodge,
    ColorBurn,
    Darken,
    Lighten,
    Difference,
    Exclusion
};

// 遮罩类型枚举
enum MaskType {
    Circle,
    Rectangle,
    RoundedRect,
    Heart,
    Star,
    Ellipse,
    Polygon
};

// 调整参数结构体
struct AdjustParams {
    qreal brightness;     // 亮度 (-1.0 到 1.0)
    qreal contrast;       // 对比度 (-1.0 到 1.0)
    qreal saturation;     // 饱和度 (-1.0 到 1.0)
    qreal temperature;    // 色温 (-1.0 到 1.0)
    qreal exposure;       // 曝光 (-1.0 到 1.0)
    qreal highlights;     // 高光 (-1.0 到 1.0)
    qreal shadows;        // 阴影 (-1.0 到 1.0)
    qreal vibrance;       // 自然饱和度 (-1.0 到 1.0)

    AdjustParams()
        : brightness(0.0)
        , contrast(0.0)
        , saturation(0.0)
        , temperature(0.0)
        , exposure(0.0)
        , highlights(0.0)
        , shadows(0.0)
        , vibrance(0.0)
    {}
    AdjustParams(qreal b, qreal c, qreal s, qreal t,
                 qreal e, qreal hi, qreal sh, qreal v)
        : brightness(b), contrast(c), saturation(s), temperature(t),
        exposure(e), highlights(hi), shadows(sh), vibrance(v) {}
};

// 滤镜参数结构体
struct FilterParams {
    FilterType type;
    qreal intensity;      // 滤镜强度 (0.0 到 1.0)
    AdjustParams adjust;  // 调整参数

    FilterParams(FilterType t = FILTER_NONE, qreal i = 1.0)
        : type(t), intensity(i)
    {}
};

class ImageEditor : public QObject
{
    Q_OBJECT

public:
    explicit ImageEditor(QObject *parent = nullptr);

    // 基本图像操作
    static QPixmap cropImage(const QPixmap &original, const QRect &rect);
    static QPixmap resizeImage(const QPixmap &original, const QSize &size,
                               bool keepAspectRatio = true);
    static QPixmap rotateImage(const QPixmap &original, qreal degrees);
    static QPixmap flipHorizontal(const QPixmap &original);
    static QPixmap flipVertical(const QPixmap &original);

    // 滤镜应用
    static QPixmap applyFilter(const QPixmap &original, FilterType filter,
                               qreal intensity = 1.0);
    static QPixmap applyAdjustments(const QPixmap &original, const AdjustParams &params);
    static QPixmap applyFilterWithParams(const QPixmap &original, const FilterParams &params);

    // 批量操作
    static QVector<QPixmap> applyFilterToAll(const QVector<QPixmap> &images,
                                             FilterType filter, qreal intensity = 1.0);

    // 高级编辑功能
    static QPixmap addBorder(const QPixmap &original, int borderWidth,
                             const QColor &color = Qt::white);
    static QPixmap addShadow(const QPixmap &original, int shadowRadius = 10,
                             qreal opacity = 0.5, const QColor &color = Qt::black);
    static QPixmap addReflection(const QPixmap &original, qreal reflectionHeight = 0.3,
                                 qreal opacity = 0.3);
    static QPixmap addVignette(const QPixmap &original, qreal intensity = 0.7,
                               const QColor &color = Qt::black);
    static QPixmap addTiltShift(const QPixmap &original, const QPoint &focusCenter,
                                int focusSize = 100, qreal blurIntensity = 0.7);

    // 特效
    static QPixmap applyBlur(const QPixmap &original, int radius);
    static QPixmap applyMotionBlur(const QPixmap &original, int angle, int distance);
    static QPixmap applyRadialBlur(const QPixmap &original, const QPoint &center,
                                   int strength = 10);

    // 艺术效果
    static QPixmap applyWatercolor(const QPixmap &original, int brushSize = 8);
    static QPixmap applyOilPaint(const QPixmap &original, int radius = 4);
    static QPixmap applyPencilSketch(const QPixmap &original,
                                     qreal pencilIntensity = 0.5,
                                     qreal paperIntensity = 0.3);
    static QPixmap applyCharcoal(const QPixmap &original, int charcoalSize = 3);
    static QPixmap applyCartoon(const QPixmap &original, int edgeThreshold = 20,
                                int colorLevels = 8);

    // 颜色调整
    static QPixmap adjustBrightness(const QPixmap &original, qreal value);
    static QPixmap adjustContrast(const QPixmap &original, qreal value);
    static QPixmap adjustSaturation(const QPixmap &original, qreal value);
    static QPixmap adjustHue(const QPixmap &original, qreal value);
    static QPixmap adjustTemperature(const QPixmap &original, qreal value);
    static QPixmap adjustExposure(const QPixmap &original, qreal value);
    static QPixmap adjustGamma(const QPixmap &original, qreal value);

    // 工具函数
    static QPixmap blendImages(const QPixmap &base, const QPixmap &overlay,
                               qreal opacity = 0.5, BlendMode mode = BlendMode::Normal);
    static QPixmap createMask(const QSize &size, MaskType type = MaskType::Circle);
    static QColor getDominantColor(const QPixmap &image, int sampleSize = 32);

    // 人脸美化（简化版）
    static QPixmap smoothSkin(const QPixmap &original, qreal intensity = 0.5);
    static QPixmap enhanceEyes(const QPixmap &original, qreal intensity = 0.3);
    static QPixmap whitenTeeth(const QPixmap &original, qreal intensity = 0.4);

    // 获取滤镜名称
    static QString getFilterName(FilterType filter);
    static QStringList getAvailableFilters();




private:
    // 滤镜实现
    static QImage applyGrayscaleFilter(const QImage &image);
    static QImage applySepiaFilter(const QImage &image, qreal intensity = 1.0);
    static QImage applyVintageFilter(const QImage &image);
    static QImage applyBlurFilter(const QImage &image, int radius);
    static QImage applySharpenFilter(const QImage &image, qreal intensity = 1.0);
    static QImage applyEmbossFilter(const QImage &image);
    static QImage applyEdgeDetectFilter(const QImage &image);
    static QImage applyInvertFilter(const QImage &image);
    static QImage applyLomoFilter(const QImage &image, qreal intensity = 1.0);
    static QImage applyCinematicFilter(const QImage &image);
    static QImage applyCrossProcessFilter(const QImage &image);

    // 卷积滤波
    static QImage applyConvolution(const QImage &image, const QVector<QVector<qreal>> &kernel);
    static QVector<QVector<qreal>> getGaussianKernel(int size, qreal sigma);
    static QVector<QVector<qreal>> getSharpenKernel(qreal intensity);
    static QVector<QVector<qreal>> getEmbossKernel();
    static QVector<QVector<qreal>> getEdgeDetectKernel();

    // 颜色处理
    static QRgb adjustPixelBrightness(QRgb pixel, qreal value);
    static QRgb adjustPixelContrast(QRgb pixel, qreal value, qreal average);
    static QRgb adjustPixelSaturation(QRgb pixel, qreal value);
    static QRgb adjustPixelTemperature(QRgb pixel, qreal value);
    static QRgb blendPixels(QRgb base, QRgb overlay, qreal opacity, BlendMode mode);

    // 工具函数
    static qreal clamp(qreal value, qreal min = 0.0, qreal max = 1.0);
    static int clamp(int value, int min = 0, int max = 255);
    static qreal lerp(qreal a, qreal b, qreal t);
    static QRgb lerpColor(QRgb a, QRgb b, qreal t);
    static QColor rgbToHsv(const QColor &color);
    static QColor hsvToRgb(const QColor &hsv);

    // 图像分析
    static QColor calculateAverageColor(const QImage &image);
    static qreal calculateLuminance(QRgb pixel);
    static QVector<int> calculateHistogram(const QImage &image, int channel = 0); // 0:亮度, 1:R, 2:G, 3:B

    // 人脸检测占位（实际使用时可以集成OpenCV等库）
    static QRect detectFace(const QImage &image);
    static QVector<QRect> detectFaces(const QImage &image);
    static QPoint detectEyes(const QImage &faceImage);
};

#endif // IMAGEEDITOR_H
