#ifndef MAINWINDOW2_H
#define MAINWINDOW2_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include "opencv2/opencv.hpp"

namespace Ui {
class MainWindow2;
}

class MainWindow2 : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow2(QWidget *parent = nullptr);
    ~MainWindow2();

private slots:
    void updateFrame();          // 更新帧
    void takePhoto();            // 拍照
    void savePhoto();            // 保存照片

private:
    cv::VideoCapture cap;        // OpenCV视频捕获对象
    QTimer *timer;               // 定时器用于更新画面
    QLabel *videoLabel;          // 显示视频的标签
    QPushButton *captureBtn;     // 拍照按钮
    QPushButton *saveBtn;        // 保存按钮

    cv::Mat currentFrame;        // 当前帧
    cv::Mat capturedImage;       // 捕获的图像
    bool isCaptured;             // 是否已拍照

    void setupUI();              // 初始化UI
    QImage cvMatToQImage(const cv::Mat &mat); // 转换函数
    void initializeCamera();
private:
    Ui::MainWindow2 *ui;
};

#endif // MAINWINDOW2_H
