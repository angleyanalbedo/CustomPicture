#pragma once
#include <opencv2/opencv.hpp>
#include "TemplateManager.h"

class ImageComposer {
public:
    static bool compose(const cv::Mat& cameraFrame,
                        const TemplateLayout& layout,
                        const std::string& outPath);

    static bool compose(const cv::Mat& cameraFrame,
                        const TemplateLayout& layout,
                        cv::Mat& outMat);
};
