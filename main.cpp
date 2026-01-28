#include "mainwindow.h"

#include <QApplication>
#include <QtGlobal>
#include <QScreen>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt5：关闭 DPI 自动缩放（关键）
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#else
    // Qt6：禁止 DPI 自动缩放
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication a(argc, argv);

    MainWindow w;

    // 固定逻辑尺寸 1080 x 1920
    w.setFixedSize(1080, 1920);

    // 全屏显示（覆盖物理屏幕）
    w.showFullScreen();

    return a.exec();
}
