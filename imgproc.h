#ifndef IMGPROC_H
#define IMGPROC_H

#include <QImage>

#include <opencv2/opencv.hpp>

class ImgProc {
public:
    /**
     * @brief 将图片嵌入到背景图的指定位置
     * @param background 报纸背景图
     * @param foreground 要插入的图片
     * @param targetRect 目标区域 (x, y, 宽度, 高度)
     * @return 合成后的图像
     */
    static cv::Mat embedImage(const cv::Mat& background, const cv::Mat& foreground, const cv::Rect& targetRect) {
        if (background.empty() || foreground.empty()) {
            return background;
        }

        cv::Mat result = background.clone();

        // 1. 安全检查：确保目标区域没有超出报纸边界
        // 利用 & 运算符取交集，防止程序崩溃
        cv::Rect safeRect = targetRect & cv::Rect(0, 0, background.cols, background.rows);

        if (safeRect.width <= 0 || safeRect.height <= 0) {
            return result;
        }

        // 2. 缩放插入图以匹配目标区域的大小
        cv::Mat resizedForeground;
        cv::resize(foreground, resizedForeground, cv::Size(safeRect.width, safeRect.height), 0, 0, cv::INTER_AREA);

        // 3. 覆盖到目标区域
        resizedForeground.copyTo(result(safeRect));

        return result;
    }
    /**
     * @brief 从文件读取并保存结果到文件
     * @param bgPath 背景图路径
     * @param fgPath 要插入的图片路径
     * @param outPath 输出保存路径
     * @param targetRect 目标位置
     * @return 是否处理成功
     */
    static bool processAndSave(const std::string& bgPath,
                               const std::string& fgPath,
                               const std::string& outPath,
                               const cv::Rect& targetRect) {

        // 1. 读取文件 (强制使用彩色模式，防止单通道背景报错)
        cv::Mat bg = cv::imread(bgPath, cv::IMREAD_COLOR);
        cv::Mat fg = cv::imread(fgPath, cv::IMREAD_COLOR);

        if (bg.empty()) {
            std::cerr << "Error: Could not read background at " << bgPath << std::endl;
            return false;
        }
        if (fg.empty()) {
            std::cerr << "Error: Could not read foreground at " << fgPath << std::endl;
            return false;
        }

        // 2. 调用之前的核心逻辑
        cv::Mat result = embedImage(bg, fg, targetRect);

        // 3. 写入文件
        bool success = cv::imwrite(outPath, result);
        if (!success) {
            std::cerr << "Error: Could not write result to " << outPath << std::endl;
        }

        return success;
    }

    // 在 ImgProc 类中增加
    static cv::Mat qImageToMat(const QImage& src) {
        switch (src.format()) {
        case QImage::Format_RGB888: {
            cv::Mat mat(src.height(), src.width(), CV_8UC3, (void*)src.bits(), src.bytesPerLine());
            cv::Mat result;
            cv::cvtColor(mat, result, cv::COLOR_RGB2BGR);
            return result;
        }
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied: {
            cv::Mat mat(src.height(), src.width(), CV_8UC4, (void*)src.bits(), src.bytesPerLine());
            cv::Mat result;
            cv::cvtColor(mat, result, cv::COLOR_BGRA2BGR);
            return result;
        }
        default:
            // 如果是其他格式，先强制转换
            return qImageToMat(src.convertToFormat(QImage::Format_RGB888));
        }
    }
};

#endif // IMGPROC_H
