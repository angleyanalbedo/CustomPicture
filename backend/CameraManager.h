#pragma once
#include <QObject>
#include <opencv4/opencv2/opencv.hpp>

class CameraManager : public QObject {
    Q_OBJECT
public:
    bool start();
    cv::Mat capture();

private:
    cv::VideoCapture cap;
};
