#pragma once
#include <QObject>
#include <QImage>
#include <QTimer>
#include <QThread>
#include <opencv2/opencv.hpp>

class CvCapture : public QObject
{
    Q_OBJECT
public:
    explicit CvCapture(int camIdx = 9, QObject *parent = nullptr);
    ~CvCapture();

    void start();          // 开始采集
    void stop();           // 停止采集
    QImage getCurrent() const { return m_current; }

signals:
    void frameReady(const QImage &img);   // 每帧图像
    void errorString(const QString &err);

private:
    void grabOnce();

    cv::VideoCapture m_cap;
    QTimer *m_timer = nullptr;
    mutable QImage m_current;
    int m_camIdx;
};
