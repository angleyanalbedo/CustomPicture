// Backend_disk.cpp
#include "ImageComposer.h"
#include "TemplateManager.h"
#include "backenddisk.h"
#include <QFile>

BackendDisk::BackendDisk(QObject *parent) : QObject(parent)
{
    cam.start();
    connect(&timer, &QTimer::timeout, this, &BackendDisk::composeOneFrame);
    timer.start(30);                     // 30 ms 约 33 FPS
}

void BackendDisk::composeOneFrame()
{
    auto frame  = cam.capture();
    auto layout = TemplateManager::load("qrc:/assets/templates/paper_01");
    ImageComposer::compose(frame, layout, "live.jpg");   // 写盘
    emit liveChanged();
}

void BackendDisk::capture()
{
    QFile::remove("final.jpg");
    QFile::copy("live.jpg", "final.jpg");                // 拍照
}
