#include "ImageComposer.h"

bool ImageComposer::compose(const cv::Mat& cameraFrame,
                            const TemplateLayout& layout,
                            const std::string& outPath)
{
    cv::Mat paper = cv::imread(layout.paperPath.toStdString());
    if (paper.empty()) return false;

    // 简单裁剪人像（居中）
    int w = cameraFrame.cols * 0.6;
    int h = cameraFrame.rows * 0.7;
    int x = (cameraFrame.cols - w) / 2;
    int y = (cameraFrame.rows - h) / 4;

    cv::Mat portrait = cameraFrame(cv::Rect(x, y, w, h)).clone();

    cv::resize(portrait, portrait,
               cv::Size(layout.photoRect.width(),
                        layout.photoRect.height()));

    portrait.copyTo(
        paper(cv::Rect(layout.photoRect.x(),
                       layout.photoRect.y(),
                       portrait.cols,
                       portrait.rows))
        );

    return cv::imwrite(outPath, paper,
                       {cv::IMWRITE_JPEG_QUALITY, 95});
}
