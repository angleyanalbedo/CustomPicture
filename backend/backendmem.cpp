#include "backendmem.h"
#include <opencv2/opencv.hpp>

BackendMem::BackendMem(QQmlApplicationEngine *engine, QObject *parent)
    : QObject(parent)
{
    cam.start();
    provider = new LiveImageProvider;
    engine->addImageProvider("live", provider);          // 注册 provider

    connect(&timer, &QTimer::timeout, this, &BackendMem::showCam);
    timer.start(30);
}

void BackendMem::composeOneFrame()
{
    cv::Mat frame = cam.capture();
    if (frame.empty()) {
        fprintf(stderr, "[composeOneFrame] camera frame empty!\n");
        return;
    }
    fprintf(stderr, "[composeOneFrame] camera ok  %dx%d  channels=%d\n",
            frame.cols, frame.rows, frame.channels());

    auto layout = TemplateManager::load(":/assets/templates/paper_01");

    cv::Mat composed;
    bool ok = ImageComposer::compose(frame, layout, composed);
    fprintf(stderr, "[composeOneFrame] ImageComposer::compose ret=%d  composed empty=%d\n",
            ok, composed.empty());
    if (composed.empty()) return;

    cv::cvtColor(composed, composed, cv::COLOR_BGR2RGB);
    QImage qimg(composed.data,
                composed.cols,
                composed.rows,
                static_cast<int>(composed.step),
                QImage::Format_RGB888);
    provider->updateImage(qimg.copy());
    emit liveChanged();
    fprintf(stderr, "[composeOneFrame] live image updated\n");
}

void BackendMem::showCam()
{
    cv::Mat frame = cam.capture();
    if (frame.empty()) {
        fprintf(stderr, "[composeOneFrame] camera frame empty!\n");
        return;
    }
    fprintf(stderr, "[composeOneFrame] camera ok  %dx%d  channels=%d\n",
            frame.cols, frame.rows, frame.channels());

    /* ---- 纯预览：直接原图 ---- */
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage qimg = QImage(frame.data, frame.cols, frame.rows,
                  static_cast<int>(frame.step),
                  QImage::Format_RGB888).copy();

    provider->updateImage(qimg.copy());
    emit liveChanged();
    fprintf(stderr, "[composeOneFrame] live image updated\n");
}

void BackendMem::capture()
{
    QImage img = provider->requestImage("0", nullptr, QSize());
    img.save("final.jpg", nullptr, 95);                  // 一次性写盘
}
