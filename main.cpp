#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

#include <backend/LiveImageProvider.h>
#include <backend/backendmem.h>

#include "backend/backenddisk.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    BackendMem backend(&engine);
    engine.rootContext()->setContextProperty("backend", &backend);

    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    return app.exec();
}



