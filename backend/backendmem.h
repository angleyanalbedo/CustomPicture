// Backend_memory.h
#pragma once
#include <QObject>
#include <QTimer>
#include <QImage>
#include <QQmlApplicationEngine>
#include "CameraManager.h"
#include "TemplateManager.h"
#include "LiveImageProvider.h"
#include "ImageComposer.h"

class BackendMem : public QObject
{
    Q_OBJECT
public:
    BackendMem(QQmlApplicationEngine *engine, QObject *parent = nullptr);

    Q_INVOKABLE void capture();          // 拍照按钮调用

signals:
    void liveChanged();

private slots:
    void composeOneFrame();
    void showCam();

private:
    CameraManager cam;
    QTimer timer;
    LiveImageProvider *provider = nullptr;

};
