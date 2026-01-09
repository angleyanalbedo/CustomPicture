#include "ImageComposer.h"
#include "qfileinfo.h"

bool ImageComposer::compose(const cv::Mat& cameraFrame,
                            const TemplateLayout& layout,
                            const std::string& outPath)
{

    QFile file(layout.paperPath);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "open failed: "
                  << file.errorString().toLocal8Bit().data()   // 或 .toStdString()
                  << std::endl;
        return {};
    }
    if (!file.open(QIODevice::ReadOnly)) return {};
    QByteArray ba = file.readAll();
    std::vector<uchar> buf(ba.begin(), ba.end());
    cv::Mat paper = cv::imdecode(buf, cv::IMREAD_COLOR);
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
bool ImageComposer::compose(const cv::Mat& cameraFrame,
                            const TemplateLayout& layout,
                            cv::Mat& outMat)
{
    QFile file(layout.paperPath);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "open failed: "
                  << file.errorString().toLocal8Bit().data()   // 或 .toStdString()
                  << std::endl;
        return {};
    }
    if (!file.open(QIODevice::ReadOnly)) return {};
    QByteArray ba = file.readAll();
    std::vector<uchar> buf(ba.begin(), ba.end());
    cv::Mat paper = cv::imdecode(buf, cv::IMREAD_COLOR);
    if (paper.empty()) return false;

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
                       portrait.rows)));

    outMat = paper;          // 直接返回 mat
    return true;
}
