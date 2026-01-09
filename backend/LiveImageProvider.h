#pragma once
#include <QQuickImageProvider>
#include <QImage>

class LiveImageProvider : public QQuickImageProvider {
public:
    LiveImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Image) {}

    /* 每次 QML 需要刷新时 Qt 会调用它 */
    QImage requestImage(const QString &/*id*/, QSize *size,
                        const QSize &requestedSize) override {
        if (m_img.isNull()) return QImage(1,1,QImage::Format_RGB888);
        QImage img = m_img;
        if (size) *size = img.size();
        if (!requestedSize.isEmpty())
            img = img.scaled(requestedSize, Qt::KeepAspectRatio);
        return img;
    }

    /* 我们在 Backend 里每合成完一帧就调这个接口更新内存图 */
    void updateImage(const QImage &newImg) { m_img = newImg; }

private:
    QImage m_img;
};
