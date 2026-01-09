// #include <QGuiApplication>
// #include <QQmlApplicationEngine>
// #include <QQmlContext>
// #include <QTimer>

// #include <backend/LiveImageProvider.h>
// #include <backend/backendmem.h>

// #include "backend/backenddisk.h"
// #include "bigheadpicturewindow.h"

// int main(int argc, char *argv[]) {
//     QGuiApplication app(argc, argv);
//     // QQmlApplicationEngine engine;

//     // BackendMem backend(&engine);
//     // engine.rootContext()->setContextProperty("backend", &backend);

//     // engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
//     BigHeadPictureWindow bigwindows;
//     bigwindows.show();
//     return app.exec();
// }


#include "bigheadpicturewindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QStyleFactory>
#include <QFile>
#include <QDebug>
#include <mainwindow2.h>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    // 1. 设置High DPI缩放（必须在QApplication之前）
    // 设置高DPI支持
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // 2. 设置OpenCV和GStreamer环境变量
    qputenv("QT_LOGGING_RULES", "*.debug=false");
    qputenv("GST_DEBUG", "0");  // 禁用GStreamer调试
    qputenv("OPENCV_LOG_LEVEL", "ERROR");

    // 3. 优先使用V4L2
    qputenv("OPENCV_VIDEOIO_PRIORITY_V4L2", "1000");
    qputenv("OPENCV_VIDEOIO_PRIORITY_MSMF", "0");
    qputenv("OPENCV_VIDEOIO_PRIORITY_GSTREAMER", "0");

    // 4. 修复runtime目录警告
    // QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    // if (runtimeDir.isEmpty()) {
    //     qputenv("XDG_RUNTIME_DIR", "/tmp/runtime-" + QByteArray::number(getuid()));
    // }
    QApplication app(argc, argv);

    // 设置应用程序信息
    QApplication::setApplicationName("BigHeadPicture");
    QApplication::setOrganizationName("MyCompany");
    QApplication::setApplicationVersion("1.0.0");

    // 设置应用程序图标（如果有的话）
    // QApplication::setWindowIcon(QIcon(":/icons/app.ico"));

    // 设置全局样式
    app.setStyle(QStyleFactory::create("Fusion"));

    // 加载样式表（可选）
    QFile styleFile(":/styles/default.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        styleFile.close();
    }

    // 加载翻译文件（可选）
    QTranslator translator;
    QString locale = QLocale::system().name();
    if (translator.load(":/translations/bigheadpicture_" + locale + ".qm")) {
        app.installTranslator(&translator);
    }



    // 创建并显示主窗口
    MainWindow2 window;



    // 居中显示（如果是桌面应用）
    window.show();

    // 设置窗口初始大小（适合移动端）
    window.showFullScreen();

    return app.exec();
}


