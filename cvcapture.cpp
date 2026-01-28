#include "cvcapture.h"
#include <QDebug>

CvCapture::CvCapture(int camIdx, QObject *parent)
    : QObject(parent), m_camIdx(camIdx)
{
    moveToThread(new QThread(this));
    thread()->start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CvCapture::grabOnce);
}

CvCapture::~CvCapture()
{
    stop();
    thread()->quit();
    thread()->wait();
}

void CvCapture::start()
{
    if (m_cap.isOpened()) return;
    // 优先用 V4L2
    m_cap.open(m_camIdx, cv::CAP_V4L2);
    if (!m_cap.isOpened())
        m_cap.open(m_camIdx);          //  fallback
    if (!m_cap.isOpened()) {
        emit errorString("无法打开摄像头 /dev/video" + QString::number(m_camIdx));
        return;
    }
    m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    m_cap.set(cv::CAP_PROP_FPS, 30);
    QMetaObject::invokeMethod(m_timer, "start", Qt::QueuedConnection, Q_ARG(int, 30));
}

void CvCapture::stop()
{
    QMetaObject::invokeMethod(m_timer, "stop", Qt::QueuedConnection);
    if (m_cap.isOpened()) m_cap.release();
}

void CvCapture::grabOnce()
{
    cv::Mat mat;
    if (!m_cap.read(mat) || mat.empty()) return;
    cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
    QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
    m_current = img.copy();              // 线程安全地留一份
    emit frameReady(img);
}
