#include "CameraManager.h"

bool CameraManager::start() {
    cap.open(9, cv::CAP_V4L2);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    return cap.isOpened();
}

cv::Mat CameraManager::capture() {
    cv::Mat frame;
    cap >> frame;
    return frame;
}
