#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "backend/CameraManager.h"
#include "backend/TemplateManager.h"
#include "backend/ImageComposer.h"

class Backend : public QObject {
    Q_OBJECT
public:
    Backend() { cam.start(); }

    Q_INVOKABLE void capture() {
        auto frame = cam.capture();
        auto layout = TemplateManager::load(
            "/usr/share/app/assets/templates/paper_01");

        ImageComposer::compose(
            frame,
            layout,
            "/data/output/final.jpg"
            );
    }

private:
    CameraManager cam;
};

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    Backend backend;
    engine.rootContext()->setContextProperty("backend", &backend);

    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    return app.exec();
}

#include "main.moc"

