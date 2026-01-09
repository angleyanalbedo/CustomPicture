#include "TemplateManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

TemplateLayout TemplateManager::load(const QString& dir) {
    TemplateLayout layout;
    layout.paperPath = dir + "/paper.png";

    QFile f(dir + "/layout.json");
    f.open(QIODevice::ReadOnly);

    auto obj = QJsonDocument::fromJson(f.readAll()).object();
    auto r = obj["photo_rect"].toObject();

    layout.photoRect = QRect(
        r["x"].toInt(),
        r["y"].toInt(),
        r["width"].toInt(),
        r["height"].toInt()
        );
    return layout;
}
