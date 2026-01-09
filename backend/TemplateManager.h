#pragma once
#include <QString>
#include <QRect>

struct TemplateLayout {
    QString paperPath;
    QRect photoRect;
};

class TemplateManager {
public:
    static TemplateLayout load(const QString& dir);
};

