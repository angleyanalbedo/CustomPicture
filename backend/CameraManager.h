#pragma once
#define RK3568 1
#include <QObject>
#include <opencv2/opencv.hpp>
class CameraManager : public QObject {
    Q_OBJECT
public:
    bool start();
    cv::Mat capture();

private:
    cv::VideoCapture cap;
};
