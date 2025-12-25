#include "imageeditor.h"
#include <QPainter>
#include <QPainterPath>
#include <QBrush>
#include <QPen>
#include <QDebug>
#include <QtMath>
#include <QVector>
#include <algorithm>
#include <random>

ImageEditor::ImageEditor(QObject *parent)
    : QObject(parent)
{
}

// 基本图像操作
QPixmap ImageEditor::cropImage(const QPixmap &original, const QRect &rect)
{
    if (original.isNull() || !rect.isValid()) {
        return original;
    }

    // 确保裁剪区域在图片范围内
    QRect validRect = rect.intersected(original.rect());
    if (validRect.isEmpty()) {
        return original;
    }

    return original.copy(validRect);
}

QPixmap ImageEditor::resizeImage(const QPixmap &original, const QSize &size, bool keepAspectRatio)
{
    if (original.isNull() || size.isEmpty()) {
        return original;
    }

    Qt::TransformationMode mode = Qt::SmoothTransformation;

    if (keepAspectRatio) {
        return original.scaled(size, Qt::KeepAspectRatio, mode);
    } else {
        return original.scaled(size, Qt::IgnoreAspectRatio, mode);
    }
}

QPixmap ImageEditor::rotateImage(const QPixmap &original, qreal degrees)
{
    if (original.isNull()) {
        return original;
    }

    QTransform transform;
    transform.rotate(degrees);
    return original.transformed(transform, Qt::SmoothTransformation);
}

QPixmap ImageEditor::flipHorizontal(const QPixmap &original)
{
    if (original.isNull()) {
        return original;
    }

    return original.transformed(QTransform().scale(-1, 1));
}

QPixmap ImageEditor::flipVertical(const QPixmap &original)
{
    if (original.isNull()) {
        return original;
    }

    return original.transformed(QTransform().scale(1, -1));
}

// 滤镜应用
QPixmap ImageEditor::applyFilter(const QPixmap &original, FilterType filter, qreal intensity)
{
    if (original.isNull()) {
        return original;
    }

    QImage image = original.toImage();
    QImage result;

    switch (filter) {
    case FILTER_GRAYSCALE:
        result = applyGrayscaleFilter(image);
        break;
    case FILTER_SEPIA:
        result = applySepiaFilter(image, intensity);
        break;
    case FILTER_VINTAGE:
        result = applyVintageFilter(image);
        break;
    case FILTER_BLUR:
        result = applyBlurFilter(image, static_cast<int>(intensity * 10));
        break;
    case FILTER_SHARPEN:
        result = applySharpenFilter(image, intensity);
        break;
    case FILTER_EMBOSS:
        result = applyEmbossFilter(image);
        break;
    case FILTER_EDGE_DETECT:
        result = applyEdgeDetectFilter(image);
        break;
    case FILTER_INVERT:
        result = applyInvertFilter(image);
        break;
    case FILTER_LOMO:
        result = applyLomoFilter(image, intensity);
        break;
    case FILTER_CINEMATIC:
        result = applyCinematicFilter(image);
        break;
    case FILTER_CROSS_PROCESS:
        result = applyCrossProcessFilter(image);
        break;
    default:
        result = image;
        break;
    }

    return QPixmap::fromImage(result);
}

QPixmap ImageEditor::applyAdjustments(const QPixmap &original, const AdjustParams &params)
{
    if (original.isNull()) {
        return original;
    }

    QImage image = original.toImage();
    QImage result = image.copy();

    // 计算平均亮度用于对比度调整
    qreal averageLuminance = 0;
    int pixelCount = image.width() * image.height();

    if (qAbs(params.contrast) > 0.01) {
        for (int y = 0; y < image.height(); ++y) {
            const QRgb *line = reinterpret_cast<const QRgb*>(image.scanLine(y));
            for (int x = 0; x < image.width(); ++x) {
                averageLuminance += calculateLuminance(line[x]);
            }
        }
        averageLuminance /= pixelCount;
    }

    // 应用调整
    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = line[x];

            // 亮度调整
            if (qAbs(params.brightness) > 0.01) {
                pixel = adjustPixelBrightness(pixel, params.brightness);
            }

            // 对比度调整
            if (qAbs(params.contrast) > 0.01) {
                pixel = adjustPixelContrast(pixel, params.contrast, averageLuminance);
            }

            // 饱和度调整
            if (qAbs(params.saturation) > 0.01) {
                pixel = adjustPixelSaturation(pixel, params.saturation);
            }

            // 色温调整
            if (qAbs(params.temperature) > 0.01) {
                pixel = adjustPixelTemperature(pixel, params.temperature);
            }

            // 曝光调整（简化版）
            if (qAbs(params.exposure) > 0.01) {
                qreal exposure = 1.0 + params.exposure * 2.0;
                int r = clamp(qRed(pixel) * exposure);
                int g = clamp(qGreen(pixel) * exposure);
                int b = clamp(qBlue(pixel) * exposure);
                pixel = qRgb(r, g, b);
            }

            line[x] = pixel;
        }
    }

    return QPixmap::fromImage(result);
}

QPixmap ImageEditor::applyFilterWithParams(const QPixmap &original, const FilterParams &params)
{
    QPixmap result = original;

    // 首先应用调整
    if (params.adjust.brightness != 0.0 || params.adjust.contrast != 0.0 ||
        params.adjust.saturation != 0.0 || params.adjust.temperature != 0.0) {
        result = applyAdjustments(result, params.adjust);
    }

    // 然后应用滤镜
    if (params.type != FILTER_NONE) {
        result = applyFilter(result, params.type, params.intensity);
    }

    return result;
}

// 灰度滤镜
QImage ImageEditor::applyGrayscaleFilter(const QImage &image)
{
    QImage result = image;

    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = line[x];
            int gray = qGray(pixel);
            line[x] = qRgb(gray, gray, gray);
        }
    }

    return result;
}

// 怀旧滤镜
QImage ImageEditor::applySepiaFilter(const QImage &image, qreal intensity)
{
    QImage result = image;

    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = line[x];
            int gray = qGray(pixel);

            // 怀旧色公式
            int r = clamp(gray * 1.2 * intensity + gray * (1 - intensity));
            int g = clamp(gray * 0.9 * intensity + gray * (1 - intensity));
            int b = clamp(gray * 0.7 * intensity + gray * (1 - intensity));

            line[x] = qRgb(r, g, b);
        }
    }

    return result;
}

// 复古滤镜
QImage ImageEditor::applyVintageFilter(const QImage &image)
{
    QImage result = image;

    // 添加褐色调
    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = line[x];

            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);

            // 复古色调调整
            r = clamp(r * 1.08);
            g = clamp(g * 0.95);
            b = clamp(b * 0.82);

            // 添加轻微褪色效果
            r = clamp(r + 20);
            g = clamp(g + 15);
            b = clamp(b + 10);

            line[x] = qRgb(r, g, b);
        }
    }

    QPixmap px = QPixmap::fromImage(result);   // 1. 先拿到 QPixmap
    px = addVignette(px, 0.6, QColor(60,40,20)); // 2. 加暗角
    result = px.toImage();                  // 3. 需要 QImage 再转回来

    return result;
}

// LOMO滤镜
QImage ImageEditor::applyLomoFilter(const QImage &image, qreal intensity)
{
    QImage result = image;

    // 增加饱和度
    result = applyAdjustments(QPixmap::fromImage(result),
                              AdjustParams{0.0, 0.1, 0.3, 0.0, 0.0, 0.0, 0.0, 0.0}).toImage();

    // 添加暗角
    QPixmap pixmap = QPixmap::fromImage(result);
    pixmap = addVignette(pixmap, 0.8 * intensity, QColor(0, 0, 0));

    // 添加轻微晕影
    for (int y = 0; y < pixmap.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(pixmap.toImage().scanLine(y));
        for (int x = 0; x < pixmap.width(); ++x) {
            QRgb pixel = line[x];

            // 计算到中心的距离
            qreal dx = (x - pixmap.width() / 2.0) / (pixmap.width() / 2.0);
            qreal dy = (y - pixmap.height() / 2.0) / (pixmap.height() / 2.0);
            qreal distance = qSqrt(dx * dx + dy * dy);

            // 边缘变暗
            qreal darken = 1.0 - distance * 0.3 * intensity;
            int r = clamp(qRed(pixel) * darken);
            int g = clamp(qGreen(pixel) * darken);
            int b = clamp(qBlue(pixel) * darken);

            line[x] = qRgb(r, g, b);
        }
    }

    return pixmap.toImage();
}

// 电影感滤镜
QImage ImageEditor::applyCinematicFilter(const QImage &image)
{
    QImage result = image;

    // 降低饱和度，增加对比度
    result = applyAdjustments(QPixmap::fromImage(result),
                              AdjustParams{0.0, 0.2, -0.2, 0.0, 0.1, 0.0, 0.0, 0.0}).toImage();

    // 添加电影调色（蓝绿色调）
    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = line[x];

            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);

            // 电影感色调（偏蓝青）
            r = clamp(r * 0.95);
            g = clamp(g * 1.05);
            b = clamp(b * 1.1);

            line[x] = qRgb(r, g, b);
        }
    }

    // 添加上下黑边（电影宽银幕效果）
    QPixmap pixmap = QPixmap::fromImage(result);
    QPixmap cinematic(pixmap.width(), pixmap.height() * 1.2);
    cinematic.fill(Qt::black);

    QPainter painter(&cinematic);
    painter.drawPixmap(0, pixmap.height() * 0.1, pixmap);
    painter.end();

    return cinematic.toImage();
}

// 交叉冲印滤镜
QImage ImageEditor::applyCrossProcessFilter(const QImage &image)
{
    QImage result = image;

    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = line[x];

            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);

            // 交叉冲印典型效果：高对比度，偏青色
            r = clamp(r * 1.1);
            g = clamp(g * 1.2);
            b = clamp(b * 0.9);

            // 提高对比度
            int avg = (r + g + b) / 3;
            r = clamp(r + (r - avg) * 0.3);
            g = clamp(g + (g - avg) * 0.3);
            b = clamp(b + (b - avg) * 0.3);

            line[x] = qRgb(r, g, b);
        }
    }

    return result;
}

// 模糊滤镜
QImage ImageEditor::applyBlurFilter(const QImage &image, int radius)
{
    if (radius <= 0) {
        return image;
    }

    int kernelSize = radius * 2 + 1;
    auto kernel = getGaussianKernel(kernelSize, radius / 2.0);
    return applyConvolution(image, kernel);
}

// 锐化滤镜
QImage ImageEditor::applySharpenFilter(const QImage &image, qreal intensity)
{
    auto kernel = getSharpenKernel(intensity);
    return applyConvolution(image, kernel);
}

// 浮雕滤镜
QImage ImageEditor::applyEmbossFilter(const QImage &image)
{
    auto kernel = getEmbossKernel();
    QImage result = applyConvolution(image, kernel);

    // 将结果偏移到可见范围
    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = line[x];
            int gray = qGray(pixel);
            // 偏移并限制范围
            gray = clamp(gray + 128);
            line[x] = qRgb(gray, gray, gray);
        }
    }

    return result;
}

// 边缘检测滤镜
QImage ImageEditor::applyEdgeDetectFilter(const QImage &image)
{
    auto kernel = getEdgeDetectKernel();
    QImage result = applyConvolution(image, kernel);

    // 取绝对值并反转
    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = line[x];
            int gray = 255 - qAbs(qGray(pixel) - 128);
            line[x] = qRgb(gray, gray, gray);
        }
    }

    return result;
}

// 反色滤镜
QImage ImageEditor::applyInvertFilter(const QImage &image)
{
    QImage result = image;

    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = line[x];
            line[x] = qRgb(255 - qRed(pixel),
                           255 - qGreen(pixel),
                           255 - qBlue(pixel));
        }
    }

    return result;
}

// 卷积滤波
QImage ImageEditor::applyConvolution(const QImage &image, const QVector<QVector<qreal>> &kernel)
{
    int kernelSize = kernel.size();
    int radius = kernelSize / 2;

    QImage result(image.size(), image.format());
    result.fill(Qt::black);

    for (int y = radius; y < image.height() - radius; ++y) {
        QRgb *destLine = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = radius; x < image.width() - radius; ++x) {
            qreal rSum = 0, gSum = 0, bSum = 0;

            // 应用卷积核
            for (int ky = -radius; ky <= radius; ++ky) {
                const QRgb *srcLine = reinterpret_cast<const QRgb*>(
                    image.scanLine(y + ky));

                for (int kx = -radius; kx <= radius; ++kx) {
                    qreal weight = kernel[ky + radius][kx + radius];
                    QRgb pixel = srcLine[x + kx];

                    rSum += qRed(pixel) * weight;
                    gSum += qGreen(pixel) * weight;
                    bSum += qBlue(pixel) * weight;
                }
            }

            int r = clamp(static_cast<int>(rSum));
            int g = clamp(static_cast<int>(gSum));
            int b = clamp(static_cast<int>(bSum));

            destLine[x] = qRgb(r, g, b);
        }
    }

    return result;
}

// 高斯核生成
QVector<QVector<qreal>> ImageEditor::getGaussianKernel(int size, qreal sigma)
{
    QVector<QVector<qreal>> kernel(size, QVector<qreal>(size, 0.0));
    int center = size / 2;
    qreal sum = 0.0;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            qreal dx = x - center;
            qreal dy = y - center;
            qreal value = qExp(-(dx * dx + dy * dy) / (2 * sigma * sigma))
                          / (2 * M_PI * sigma * sigma);
            kernel[y][x] = value;
            sum += value;
        }
    }

    // 归一化
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            kernel[y][x] /= sum;
        }
    }

    return kernel;
}

// 锐化核
QVector<QVector<qreal>> ImageEditor::getSharpenKernel(qreal intensity)
{
    QVector<QVector<qreal>> kernel = {
        {0, -intensity, 0},
        {-intensity, 1 + 4 * intensity, -intensity},
        {0, -intensity, 0}
    };

    return kernel;
}

// 浮雕核
QVector<QVector<qreal>> ImageEditor::getEmbossKernel()
{
    QVector<QVector<qreal>> kernel = {
        {-2, -1, 0},
        {-1, 1, 1},
        {0, 1, 2}
    };

    return kernel;
}

// 边缘检测核
QVector<QVector<qreal>> ImageEditor::getEdgeDetectKernel()
{
    QVector<QVector<qreal>> kernel = {
        {-1, -1, -1},
        {-1, 8, -1},
        {-1, -1, -1}
    };

    return kernel;
}

// 像素亮度调整
QRgb ImageEditor::adjustPixelBrightness(QRgb pixel, qreal value)
{
    int r = clamp(qRed(pixel) + value * 255);
    int g = clamp(qGreen(pixel) + value * 255);
    int b = clamp(qBlue(pixel) + value * 255);

    return qRgb(r, g, b);
}

// 像素对比度调整
QRgb ImageEditor::adjustPixelContrast(QRgb pixel, qreal value, qreal average)
{
    qreal factor = (1.0 + value);
    int r = clamp((qRed(pixel) - average) * factor + average);
    int g = clamp((qGreen(pixel) - average) * factor + average);
    int b = clamp((qBlue(pixel) - average) * factor + average);

    return qRgb(r, g, b);
}

// 像素饱和度调整
QRgb ImageEditor::adjustPixelSaturation(QRgb pixel, qreal value)
{
    QColor color(pixel);
    QColor hsv = rgbToHsv(color);

    qreal s = hsv.hsvSaturationF();
    s = clamp(s * (1.0 + value));

    hsv.setHsvF(hsv.hsvHueF(), s, hsv.valueF());
    return hsvToRgb(hsv).rgb();
}

// 像素色温调整
QRgb ImageEditor::adjustPixelTemperature(QRgb pixel, qreal value)
{
    int r = qRed(pixel);
    int g = qGreen(pixel);
    int b = qBlue(pixel);

    // 暖色调增加红色，冷色调增加蓝色
    if (value > 0) { // 暖色
        r = clamp(r + value * 30);
        b = clamp(b - value * 15);
    } else { // 冷色
        r = clamp(r + value * 30);
        b = clamp(b - value * 15);
    }

    return qRgb(r, g, b);
}

// 添加边框
QPixmap ImageEditor::addBorder(const QPixmap &original, int borderWidth, const QColor &color)
{
    if (original.isNull() || borderWidth <= 0) {
        return original;
    }

    QPixmap result(original.width() + borderWidth * 2,
                   original.height() + borderWidth * 2);
    result.fill(color);

    QPainter painter(&result);
    painter.drawPixmap(borderWidth, borderWidth, original);
    painter.end();

    return result;
}

// 添加阴影
QPixmap ImageEditor::addShadow(const QPixmap &original, int shadowRadius, qreal opacity, const QColor &color)
{
    if (original.isNull()) {
        return original;
    }

    // 创建带阴影的图片
    int padding = shadowRadius * 2;
    QPixmap shadow(original.width() + padding, original.height() + padding);
    shadow.fill(Qt::transparent);

    QPainter painter(&shadow);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制阴影
    QPainterPath path;
    path.addRoundedRect(shadowRadius, shadowRadius,
                        original.width(), original.height(), 5, 5);

    QColor shadowColor = color;
    shadowColor.setAlphaF(opacity);

    painter.setBrush(shadowColor);
    painter.setPen(Qt::NoPen);

    // 应用模糊效果
    painter.drawPath(path);

    // 绘制原图
    painter.drawPixmap(shadowRadius, shadowRadius, original);
    painter.end();

    return shadow;
}

// 添加倒影
QPixmap ImageEditor::addReflection(const QPixmap &original, qreal reflectionHeight, qreal opacity)
{
    if (original.isNull() || reflectionHeight <= 0) {
        return original;
    }

    int reflectionSize = original.height() * reflectionHeight;
    QPixmap result(original.width(), original.height() + reflectionSize);
    result.fill(Qt::transparent);

    QPainter painter(&result);

    // 绘制原图
    painter.drawPixmap(0, 0, original);

    // 创建倒影
    QPixmap reflection = original.copy(0, original.height() - reflectionSize,
                                       original.width(), reflectionSize);

    // 垂直翻转
    reflection = reflection.transformed(QTransform().scale(1, -1));

    // 创建渐变蒙版
    QLinearGradient gradient(0, 0, 0, reflectionSize);
    gradient.setColorAt(0, QColor(0, 0, 0, static_cast<int>(opacity * 255)));
    gradient.setColorAt(1, Qt::transparent);

    // 应用渐变蒙版
    QPixmap maskedReflection(reflection.size());
    maskedReflection.fill(Qt::transparent);

    QPainter reflectionPainter(&maskedReflection);
    reflectionPainter.setCompositionMode(QPainter::CompositionMode_Source);
    reflectionPainter.drawPixmap(0, 0, reflection);
    reflectionPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    reflectionPainter.fillRect(maskedReflection.rect(), gradient);
    reflectionPainter.end();

    // 绘制倒影
    painter.drawPixmap(0, original.height(), maskedReflection);
    painter.end();

    return result;
}

// 添加暗角
QPixmap ImageEditor::addVignette(const QPixmap &original, qreal intensity, const QColor &color)
{
    if (original.isNull()) {
        return original;
    }

    QPixmap result = original;
    QImage image = result.toImage();

    qreal centerX = image.width() / 2.0;
    qreal centerY = image.height() / 2.0;
    qreal maxDist = qSqrt(centerX * centerX + centerY * centerY);

    for (int y = 0; y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            qreal dx = x - centerX;
            qreal dy = y - centerY;
            qreal distance = qSqrt(dx * dx + dy * dy) / maxDist;

            // 计算暗角强度
            qreal vignette = 1.0 - distance * intensity;
            vignette = clamp(vignette);

            QRgb pixel = line[x];
            int r = clamp(qRed(pixel) * vignette + color.red() * (1 - vignette));
            int g = clamp(qGreen(pixel) * vignette + color.green() * (1 - vignette));
            int b = clamp(qBlue(pixel) * vignette + color.blue() * (1 - vignette));

            line[x] = qRgb(r, g, b);
        }
    }

    return QPixmap::fromImage(image);
}

// 移轴模糊
QPixmap ImageEditor::addTiltShift(const QPixmap &original, const QPoint &focusCenter,
                                  int focusSize, qreal blurIntensity)
{
    if (original.isNull()) {
        return original;
    }

    QImage image = original.toImage();
    QImage result = image.copy();

    // 应用渐变模糊
    for (int y = 0; y < image.height(); ++y) {
        // 计算该行的模糊强度
        qreal distance = qAbs(y - focusCenter.y());
        qreal blurFactor = 0;

        if (distance > focusSize / 2) {
            blurFactor = (distance - focusSize / 2) / (image.height() / 2.0);
            blurFactor = clamp(blurFactor) * blurIntensity;
        }

        if (blurFactor > 0.01) {
            int blurRadius = static_cast<int>(blurFactor * 20);
            if (blurRadius > 0) {
                // 简单水平模糊
                for (int x = 0; x < image.width(); ++x) {
                    int rSum = 0, gSum = 0, bSum = 0;
                    int count = 0;

                    for (int dx = -blurRadius; dx <= blurRadius; ++dx) {
                        int sampleX = clamp(x + dx, 0, image.width() - 1);
                        QRgb pixel = image.pixel(sampleX, y);

                        rSum += qRed(pixel);
                        gSum += qGreen(pixel);
                        bSum += qBlue(pixel);
                        count++;
                    }

                    QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
                    line[x] = qRgb(rSum / count, gSum / count, bSum / count);
                }
            }
        }
    }

    return QPixmap::fromImage(result);
}

// 油画效果
QPixmap ImageEditor::applyOilPaint(const QPixmap &original, int radius)
{
    if (original.isNull() || radius <= 0) {
        return original;
    }

    QImage image = original.toImage();
    QImage result(image.size(), image.format());

    for (int y = radius; y < image.height() - radius; ++y) {
        QRgb *destLine = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = radius; x < image.width() - radius; ++x) {
            // 统计区域内的颜色直方图
            QMap<int, int> intensityCount;
            QMap<int, QRgb> intensityColor;

            for (int dy = -radius; dy <= radius; ++dy) {
                const QRgb *srcLine = reinterpret_cast<const QRgb*>(
                    image.scanLine(y + dy));

                for (int dx = -radius; dx <= radius; ++dx) {
                    QRgb pixel = srcLine[x + dx];
                    int intensity = qGray(pixel);

                    intensityCount[intensity] = intensityCount.value(intensity, 0) + 1;
                    intensityColor[intensity] = pixel;
                }
            }

            // 找到出现次数最多的强度值
            int maxCount = 0;
            int dominantIntensity = 0;

            for (auto it = intensityCount.begin(); it != intensityCount.end(); ++it) {
                if (it.value() > maxCount) {
                    maxCount = it.value();
                    dominantIntensity = it.key();
                }
            }

            // 使用该强度对应的颜色
            destLine[x] = intensityColor[dominantIntensity];
        }
    }

    return QPixmap::fromImage(result);
}

// 铅笔素描效果
QPixmap ImageEditor::applyPencilSketch(const QPixmap &original,
                                       qreal pencilIntensity,
                                       qreal paperIntensity)
{
    if (original.isNull()) {
        return original;
    }

    // 创建灰度图
    QPixmap grayscale = applyFilter(original, FILTER_GRAYSCALE);
    QImage grayImage = grayscale.toImage();

    // 创建边缘图
    QPixmap edges = applyFilter(original, FILTER_EDGE_DETECT);
    QImage edgeImage = edges.toImage();

    // 合并效果
    QImage result(grayImage.size(), grayImage.format());

    for (int y = 0; y < result.height(); ++y) {
        QRgb *destLine = reinterpret_cast<QRgb*>(result.scanLine(y));
        const QRgb *grayLine = reinterpret_cast<const QRgb*>(grayImage.scanLine(y));
        const QRgb *edgeLine = reinterpret_cast<const QRgb*>(edgeImage.scanLine(y));

        for (int x = 0; x < result.width(); ++x) {
            int gray = qGray(grayLine[x]);
            int edge = qGray(edgeLine[x]);

            // 铅笔效果：边缘部分变暗
            int value = gray - edge * pencilIntensity;
            value = clamp(value);

            // 添加纸张纹理效果
            if (paperIntensity > 0) {
                // 简单随机噪声模拟纸张纹理
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(-20, 20);

                int noise = dis(gen) * paperIntensity;
                value = clamp(value + noise);
            }

            destLine[x] = qRgb(value, value, value);
        }
    }

    return QPixmap::fromImage(result);
}

// 卡通效果
QPixmap ImageEditor::applyCartoon(const QPixmap &original, int edgeThreshold, int colorLevels)
{
    if (original.isNull()) {
        return original;
    }

    // 1. 边缘检测
    QPixmap edges = applyFilter(original, FILTER_EDGE_DETECT);
    QImage edgeImage = edges.toImage();

    // 2. 颜色量化（减少颜色数量）
    QImage quantized = original.toImage();
    for (int y = 0; y < quantized.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(quantized.scanLine(y));
        for (int x = 0; x < quantized.width(); ++x) {
            QRgb pixel = line[x];

            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);

            // 颜色量化
            r = (r * colorLevels) / 256 * (256 / colorLevels);
            g = (g * colorLevels) / 256 * (256 / colorLevels);
            b = (b * colorLevels) / 256 * (256 / colorLevels);

            line[x] = qRgb(r, g, b);
        }
    }

    // 3. 合并：用边缘图作为蒙版
    QImage result = quantized;
    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        const QRgb *edgeLine = reinterpret_cast<const QRgb*>(edgeImage.scanLine(y));

        for (int x = 0; x < result.width(); ++x) {
            int edgeValue = qGray(edgeLine[x]);

            if (edgeValue < edgeThreshold) {
                // 边缘部分变黑
                line[x] = qRgb(0, 0, 0);
            }
        }
    }

    // 4. 轻微模糊平滑
    result = applyBlurFilter(result, 1);

    return QPixmap::fromImage(result);
}

// 批量应用滤镜
QVector<QPixmap> ImageEditor::applyFilterToAll(const QVector<QPixmap> &images,
                                               FilterType filter, qreal intensity)
{
    QVector<QPixmap> result;
    result.reserve(images.size());

    for (const QPixmap &image : images) {
        result.append(applyFilter(image, filter, intensity));
    }

    return result;
}

// 获取滤镜名称
QString ImageEditor::getFilterName(FilterType filter)
{
    static QMap<FilterType, QString> filterNames = {
        {FILTER_NONE, "无滤镜"},
        {FILTER_GRAYSCALE, "灰度"},
        {FILTER_SEPIA, "怀旧"},
        {FILTER_VINTAGE, "复古"},
        {FILTER_BRIGHTNESS, "亮度"},
        {FILTER_CONTRAST, "对比度"},
        {FILTER_SATURATION, "饱和度"},
        {FILTER_TEMPERATURE, "色温"},
        {FILTER_INVERT, "反色"},
        {FILTER_BLUR, "模糊"},
        {FILTER_SHARPEN, "锐化"},
        {FILTER_EMBOSS, "浮雕"},
        {FILTER_EDGE_DETECT, "边缘检测"},
        {FILTER_OIL_PAINT, "油画"},
        {FILTER_PENCIL_SKETCH, "铅笔素描"},
        {FILTER_CARTOON, "卡通"},
        {FILTER_HDR, "HDR"},
        {FILTER_VIGNETTE, "暗角"},
        {FILTER_TILT_SHIFT, "移轴"},
        {FILTER_CROSS_PROCESS, "交叉冲印"},
        {FILTER_LOMO, "LOMO"},
        {FILTER_CINEMATIC, "电影感"},
        {FILTER_PORTRAIT, "人像美化"}
    };

    return filterNames.value(filter, "未知滤镜");
}

QStringList ImageEditor::getAvailableFilters()
{
    return {
        "无滤镜",
        "灰度",
        "怀旧",
        "复古",
        "亮度",
        "对比度",
        "饱和度",
        "色温",
        "反色",
        "模糊",
        "锐化",
        "浮雕",
        "边缘检测",
        "油画",
        "铅笔素描",
        "卡通",
        "HDR",
        "暗角",
        "移轴",
        "交叉冲印",
        "LOMO",
        "电影感",
        "人像美化"
    };
}

// 工具函数
qreal ImageEditor::clamp(qreal value, qreal min, qreal max)
{
    return qBound(min, value, max);
}

int ImageEditor::clamp(int value, int min, int max)
{
    return qBound(min, value, max);
}

qreal ImageEditor::lerp(qreal a, qreal b, qreal t)
{
    return a + (b - a) * t;
}

QRgb ImageEditor::lerpColor(QRgb a, QRgb b, qreal t)
{
    int r = lerp(qRed(a), qRed(b), t);
    int g = lerp(qGreen(a), qGreen(b), t);
    int b_ = lerp(qBlue(a), qBlue(b), t);

    return qRgb(r, g, b_);
}

QColor ImageEditor::rgbToHsv(const QColor &color)
{
    return color.toHsv();
}

QColor ImageEditor::hsvToRgb(const QColor &hsv)
{
    return hsv.toRgb();
}

qreal ImageEditor::calculateLuminance(QRgb pixel)
{
    return 0.299 * qRed(pixel) + 0.587 * qGreen(pixel) + 0.114 * qBlue(pixel);
}

// 添加更多滤镜实现...

// 运动模糊
QPixmap ImageEditor::applyMotionBlur(const QPixmap &original, int angle, int distance)
{
    if (original.isNull() || distance <= 0) {
        return original;
    }

    QImage image = original.toImage();
    QImage result(image.size(), image.format());
    result.fill(Qt::transparent);

    // 计算运动方向
    qreal rad = qDegreesToRadians(static_cast<qreal>(angle));
    qreal dx = qCos(rad);
    qreal dy = qSin(rad);

    for (int y = 0; y < image.height(); ++y) {
        QRgb *destLine = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < image.width(); ++x) {
            qreal rSum = 0, gSum = 0, bSum = 0;
            int count = 0;

            // 沿着运动方向采样
            for (int i = -distance; i <= distance; ++i) {
                int sampleX = x + static_cast<int>(dx * i);
                int sampleY = y + static_cast<int>(dy * i);

                if (sampleX >= 0 && sampleX < image.width() &&
                    sampleY >= 0 && sampleY < image.height()) {
                    QRgb pixel = image.pixel(sampleX, sampleY);

                    rSum += qRed(pixel);
                    gSum += qGreen(pixel);
                    bSum += qBlue(pixel);
                    count++;
                }
            }

            if (count > 0) {
                destLine[x] = qRgb(static_cast<int>(rSum / count),
                                   static_cast<int>(gSum / count),
                                   static_cast<int>(bSum / count));
            }
        }
    }

    return QPixmap::fromImage(result);
}

// 径向模糊
QPixmap ImageEditor::applyRadialBlur(const QPixmap &original, const QPoint &center, int strength)
{
    if (original.isNull() || strength <= 0) {
        return original;
    }

    QImage image = original.toImage();
    QImage result(image.size(), image.format());

    for (int y = 0; y < image.height(); ++y) {
        QRgb *destLine = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < image.width(); ++x) {
            // 计算到中心的距离和角度
            qreal dx = x - center.x();
            qreal dy = y - center.y();
            qreal distance = qSqrt(dx * dx + dy * dy);
            qreal angle = qAtan2(dy, dx);

            qreal rSum = 0, gSum = 0, bSum = 0;
            int count = 0;

            // 沿着径向采样
            for (int i = -strength; i <= strength; ++i) {
                int sampleDistance = static_cast<int>(distance + i);
                if (sampleDistance < 0) continue;

                int sampleX = center.x() + static_cast<int>(qCos(angle) * sampleDistance);
                int sampleY = center.y() + static_cast<int>(qSin(angle) * sampleDistance);

                if (sampleX >= 0 && sampleX < image.width() &&
                    sampleY >= 0 && sampleY < image.height()) {
                    QRgb pixel = image.pixel(sampleX, sampleY);

                    rSum += qRed(pixel);
                    gSum += qGreen(pixel);
                    bSum += qBlue(pixel);
                    count++;
                }
            }

            if (count > 0) {
                destLine[x] = qRgb(static_cast<int>(rSum / count),
                                   static_cast<int>(gSum / count),
                                   static_cast<int>(bSum / count));
            } else {
                destLine[x] = image.pixel(x, y);
            }
        }
    }

    return QPixmap::fromImage(result);
}

// 水彩画效果
QPixmap ImageEditor::applyWatercolor(const QPixmap &original, int brushSize)
{
    if (original.isNull() || brushSize <= 0) {
        return original;
    }

    // 首先应用油画效果
    QPixmap oilPaint = applyOilPaint(original, brushSize / 2);

    // 然后添加纸张纹理
    QImage result = oilPaint.toImage();

    // 添加轻微噪点模拟水彩纸纹理
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(-10, 10);

    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            if ((x + y) % 3 == 0) { // 随机间隔添加纹理
                int noise = dis(gen);
                QRgb pixel = line[x];

                int r = clamp(qRed(pixel) + noise);
                int g = clamp(qGreen(pixel) + noise);
                int b = clamp(qBlue(pixel) + noise);

                line[x] = qRgb(r, g, b);
            }
        }
    }

    // 轻微模糊让颜色融合
    result = applyBlurFilter(result, 1);

    return QPixmap::fromImage(result);
}

// 木炭画效果
QPixmap ImageEditor::applyCharcoal(const QPixmap &original, int charcoalSize)
{
    if (original.isNull() || charcoalSize <= 0) {
        return original;
    }

    // 创建高对比度灰度图
    QPixmap grayscale = applyFilter(original, FILTER_GRAYSCALE);
    QPixmap highContrast = applyAdjustments(grayscale, AdjustParams{0.0, 0.3, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0});

    // 应用运动模糊模拟木炭笔触
    QPixmap charcoal = applyMotionBlur(highContrast, 45, charcoalSize);

    // 反转颜色
    QImage result = applyInvertFilter(charcoal.toImage());

    // 添加颗粒感
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 30);

    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            if ((x + y) % 4 == 0) { // 添加随机颗粒
                int grain = dis(gen);
                int gray = qGray(line[x]);
                gray = clamp(gray - grain);
                line[x] = qRgb(gray, gray, gray);
            }
        }
    }

    return QPixmap::fromImage(result);
}

// 亮度调整
QPixmap ImageEditor::adjustBrightness(const QPixmap &original, qreal value)
{
    AdjustParams params;
    params.brightness = value;
    return applyAdjustments(original, params);
}

// 对比度调整
QPixmap ImageEditor::adjustContrast(const QPixmap &original, qreal value)
{
    AdjustParams params;
    params.contrast = value;
    return applyAdjustments(original, params);
}

// 饱和度调整
QPixmap ImageEditor::adjustSaturation(const QPixmap &original, qreal value)
{
    AdjustParams params;
    params.saturation = value;
    return applyAdjustments(original, params);
}

// 色相调整
QPixmap ImageEditor::adjustHue(const QPixmap &original, qreal value)
{
    if (original.isNull()) {
        return original;
    }

    QImage image = original.toImage();
    QImage result = image.copy();

    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QColor color(line[x]);
            QColor hsv = color.toHsv();

            qreal hue = hsv.hsvHueF() + value;
            while (hue < 0) hue += 1.0;
            while (hue > 1) hue -= 1.0;

            hsv.setHsvF(hue, hsv.hsvSaturationF(), hsv.valueF());
            line[x] = hsv.toRgb().rgb();
        }
    }

    return QPixmap::fromImage(result);
}

// 色温调整
QPixmap ImageEditor::adjustTemperature(const QPixmap &original, qreal value)
{
    AdjustParams params;
    params.temperature = value;
    return applyAdjustments(original, params);
}

// 曝光调整
QPixmap ImageEditor::adjustExposure(const QPixmap &original, qreal value)
{
    AdjustParams params;
    params.exposure = value;
    return applyAdjustments(original, params);
}

// Gamma调整
QPixmap ImageEditor::adjustGamma(const QPixmap &original, qreal value)
{
    if (original.isNull() || qAbs(value - 1.0) < 0.01) {
        return original;
    }

    QImage image = original.toImage();
    QImage result = image.copy();

    // 预计算Gamma表
    QVector<int> gammaTable(256);
    qreal invGamma = 1.0 / value;

    for (int i = 0; i < 256; ++i) {
        gammaTable[i] = clamp(static_cast<int>(qPow(i / 255.0, invGamma) * 255.0));
    }

    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = line[x];

            int r = gammaTable[qRed(pixel)];
            int g = gammaTable[qGreen(pixel)];
            int b = gammaTable[qBlue(pixel)];

            line[x] = qRgb(r, g, b);
        }
    }

    return QPixmap::fromImage(result);
}

// 图像混合


// 创建遮罩
QPixmap ImageEditor::createMask(const QSize &size, MaskType type)
{
    QPixmap mask(size);
    mask.fill(Qt::transparent);

    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);

    QRectF rect(0, 0, size.width(), size.height());

    switch (type) {
    case Circle:
        painter.drawEllipse(rect);
        break;
    case Rectangle:
        painter.drawRect(rect);
        break;
    case RoundedRect:
        painter.drawRoundedRect(rect, 20, 20);
        break;
    case Heart: {
        QPainterPath path;
        qreal width = rect.width();
        qreal height = rect.height();

        path.moveTo(width / 2, height / 4);
        path.cubicTo(width * 0.1, height * 0.1,
                     0, height * 0.3,
                     width / 2, height * 0.9);
        path.cubicTo(width, height * 0.3,
                     width * 0.9, height * 0.1,
                     width / 2, height / 4);
        painter.drawPath(path);
        break;
    }
    case Star: {
        QPainterPath path;
        QPointF center = rect.center();
        qreal radius = qMin(rect.width(), rect.height()) / 2;

        for (int i = 0; i < 5; ++i) {
            qreal angle = 2 * M_PI * i / 5 - M_PI_2;
            QPointF outer(center.x() + radius * qCos(angle),
                          center.y() + radius * qSin(angle));

            if (i == 0) path.moveTo(outer);
            else path.lineTo(outer);

            qreal innerAngle = angle + M_PI / 5;
            QPointF inner(center.x() + radius * 0.5 * qCos(innerAngle),
                          center.y() + radius * 0.5 * qSin(innerAngle));
            path.lineTo(inner);
        }
        path.closeSubpath();
        painter.drawPath(path);
        break;
    }
    case Ellipse:
        painter.drawEllipse(rect.adjusted(10, 20, -10, -20));
        break;
    case Polygon: {
        QPainterPath path;
        int sides = 6;
        QPointF center = rect.center();
        qreal radius = qMin(rect.width(), rect.height()) / 2;

        for (int i = 0; i <= sides; ++i) {
            qreal angle = 2 * M_PI * i / sides - M_PI_2;
            QPointF point(center.x() + radius * qCos(angle),
                          center.y() + radius * qSin(angle));

            if (i == 0) path.moveTo(point);
            else path.lineTo(point);
        }
        painter.drawPath(path);
        break;
    }
    }

    painter.end();
    return mask;
}

// 获取主色调
QColor ImageEditor::getDominantColor(const QPixmap &image, int sampleSize)
{
    if (image.isNull()) {
        return Qt::black;
    }

    QImage sampled = image.scaled(sampleSize, sampleSize,
                                  Qt::KeepAspectRatioByExpanding,
                                  Qt::SmoothTransformation)
                         .toImage();

    QMap<QRgb, int> colorCount;

    // 统计颜色出现频率
    for (int y = 0; y < sampled.height(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb*>(sampled.scanLine(y));
        for (int x = 0; x < sampled.width(); ++x) {
            // 量化颜色以减少颜色数量
            QRgb pixel = line[x];
            int r = (qRed(pixel) >> 4) << 4;    // 16级量化
            int g = (qGreen(pixel) >> 4) << 4;
            int b = (qBlue(pixel) >> 4) << 4;
            QRgb quantized = qRgb(r, g, b);

            colorCount[quantized] = colorCount.value(quantized, 0) + 1;
        }
    }

    // 找到出现最多的颜色
    QRgb dominant = 0;
    int maxCount = 0;

    for (auto it = colorCount.begin(); it != colorCount.end(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            dominant = it.key();
        }
    }

    return QColor(dominant);
}

// 人脸美化（简化版）
QPixmap ImageEditor::smoothSkin(const QPixmap &original, qreal intensity)
{
    if (original.isNull()) {
        return original;
    }

    // 应用高斯模糊来平滑皮肤
    int blurRadius = static_cast<int>(intensity * 10);
    QPixmap blurred = applyBlur(original, blurRadius);

    // 将模糊后的图像与原图混合
    return blendImages(original, blurred, intensity * 0.5, Normal);
}

// 增强眼睛（简化版）
QPixmap ImageEditor::enhanceEyes(const QPixmap &original, qreal intensity)
{
    if (original.isNull()) {
        return original;
    }

    // 这里应该是检测眼睛位置，然后局部增强
    // 简化版：整体增加对比度和饱和度

    AdjustParams params;
    params.contrast = intensity * 0.2;
    params.saturation = intensity * 0.1;

    return applyAdjustments(original, params);
}

// 美白牙齿（简化版）
QPixmap ImageEditor::whitenTeeth(const QPixmap &original, qreal intensity)
{
    if (original.isNull()) {
        return original;
    }

    QImage image = original.toImage();
    QImage result = image.copy();

    // 简化版：整体增加蓝色通道，减少红色通道
    for (int y = 0; y < result.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = line[x];

            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);

            // 向蓝色偏移（牙齿美白效果）
            r = clamp(r - intensity * 20);
            g = clamp(g + intensity * 10);
            b = clamp(b + intensity * 30);

            line[x] = qRgb(r, g, b);
        }
    }

    return QPixmap::fromImage(result);
}
