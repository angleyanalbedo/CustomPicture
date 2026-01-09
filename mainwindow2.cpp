// mainwindow.cpp
#include "mainwindow2.h"
#include "ui_mainwindow2.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QHBoxLayout>

MainWindow2::MainWindow2(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow2)
{
    ui->setupUi(this);

    // 初始化变量
    isCaptured = false;

    // 设置UI
    setupUI();

    // 打开摄像头（
    initializeCamera();


}

void MainWindow2::initializeCamera()
{
    // 尝试不同的摄像头打开方法
    int cameraIndex = 9;  // 默认摄像头

    // 方法1: 使用设备路径
    QString devicePath = QString("/dev/video%1").arg(cameraIndex);
    cap.open(devicePath.toStdString(), cv::CAP_V4L2);

    if (!cap.isOpened()) {
        // 方法2: 使用索引和V4L2后端
        cap.open(cameraIndex, cv::CAP_V4L2);
    }

    if (!cap.isOpened()) {
        // 方法3: 尝试自动检测
        cap.open(cameraIndex);
    }

    if (cap.isOpened()) {
       std::cout << "摄像头打开成功\n";

        // 设置摄像头参数
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        cap.set(cv::CAP_PROP_FPS, 30);
        cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

        // 验证设置
        double width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        double height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        double fps = cap.get(cv::CAP_PROP_FPS);
        std::cout<< "摄像头参数: " << width << "x" << height << " FPS:" << fps<<std::endl;

        // 启动定时器
        // 设置定时器，每30ms更新一帧（约33fps）
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MainWindow2::updateFrame);
        timer->start(10);  // 33fps

    } else {
        printf("错误无法打开摄像头！\n"
                             "可能的原因：\n"
                             "1. 摄像头未连接\n"
                             "2. 权限不足（尝试sudo）\n"
                             "3. 摄像头被其他程序占用\n"
                             "4. 设备路径不正确");
    }
}


MainWindow2::~MainWindow2()
{
    cap.release();
    delete ui;
}

void MainWindow2::setupUI()
{
    // 创建视频显示标签
    videoLabel = new QLabel(this);
    videoLabel->setFixedSize(640, 480);
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setStyleSheet("border: 2px solid #555; background-color: black;");

    // 创建按钮
    captureBtn = new QPushButton("拍 照", this);
    saveBtn = new QPushButton("保 存", this);
    saveBtn->setEnabled(false);

    // 按钮布局
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(captureBtn);
    btnLayout->addWidget(saveBtn);
    btnLayout->addStretch();

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(videoLabel, 0, Qt::AlignCenter);
    mainLayout->addLayout(btnLayout);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // 连接信号槽
    connect(captureBtn, &QPushButton::clicked, this, &MainWindow2::takePhoto);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow2::savePhoto);
}

void MainWindow2::updateFrame()
{
    // if (!cap.isOpened()) return;

    // cap >> currentFrame;
    // if (currentFrame.empty()) return;

    // // 如果未拍照，显示实时视频
    // if (!isCaptured) {
    //     cv::Mat displayFrame;
    //     cv::cvtColor(currentFrame, displayFrame, cv::COLOR_BGR2RGB);
    //     QImage img = cvMatToQImage(displayFrame);
    //     videoLabel->setPixmap(QPixmap::fromImage(img).scaled(
    //         videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    // }
    if (!cap.isOpened()) return;

    cv::Mat frame;
    bool success = cap.read(frame);

    if (!success || frame.empty()) {
        std::cout << "读取帧失败"<<std::endl;
        return;
    }

    // 转换颜色空间
    cv::Mat rgbFrame;
    cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);

    // 显示图像
    QImage img(rgbFrame.data, rgbFrame.cols, rgbFrame.rows,
               rgbFrame.step, QImage::Format_RGB888);

    QPixmap pixmap = QPixmap::fromImage(img);
    videoLabel->setPixmap(pixmap.scaled(videoLabel->size(),
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation));
}

void MainWindow2::takePhoto()
{
    if (currentFrame.empty()) return;

    capturedImage = currentFrame.clone();
    isCaptured = true;

    // 显示捕获的图像
    cv::Mat displayFrame;
    cv::cvtColor(capturedImage, displayFrame, cv::COLOR_BGR2RGB);
    QImage img = cvMatToQImage(displayFrame);
    videoLabel->setPixmap(QPixmap::fromImage(img).scaled(
        videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    saveBtn->setEnabled(true);
    QMessageBox::information(this, "提示", "拍照成功!");
}

void MainWindow2::savePhoto()
{
    if (capturedImage.empty()) return;

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "保存图片",
                                                    QString("photo_%1.jpg").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
                                                    "图像文件 (*.jpg *.png *.bmp)");

    if (!fileName.isEmpty()) {
        cv::imwrite(fileName.toStdString(), capturedImage);
        QMessageBox::information(this, "成功", "图片保存成功!");
    }
}

QImage MainWindow2::cvMatToQImage(const cv::Mat &mat)
{
    switch(mat.type()) {
    case CV_8UC3: {
        QImage image(mat.data, mat.cols, mat.rows,
                     static_cast<int>(mat.step), QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    case CV_8UC1: {
        QImage image(mat.data, mat.cols, mat.rows,
                     static_cast<int>(mat.step), QImage::Format_Grayscale8);
        return image;
    }
    default:
        return QImage();
    }
}
