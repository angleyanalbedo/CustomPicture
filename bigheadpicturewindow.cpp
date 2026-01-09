#include "bigheadpicturewindow.h"
#include "ui_bigheadpicturewindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QDateTime>
#include <QPainter>
#include <QDir>
#include <QDebug>
#include <QResizeEvent>
#include <QElapsedTimer>
#include <QThread>

BigHeadPictureWindow::BigHeadPictureWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::BigHeadPictureWindow)
    , camera(nullptr)
    , viewfinder(nullptr)
    , imageCapture(nullptr)
    , cameraActive(false)
    , currentBgIndex(0)
{
    ui->setupUi(this);

    // 初始化UI状态
    initUI();

    // 初始化相机
    initCamera();

    // 初始化背景图片
    initBackgrounds();

    // 连接信号槽
    initConnections();

    // 更新UI状态
    updateUI();
}

void BigHeadPictureWindow::initUI()
{
    // 设置窗口属性
    setWindowTitle("大头照相机");

    // 初始状态
    ui->btnCapture->setEnabled(false);
    ui->btnPrevBg->setEnabled(false);
    ui->btnNextBg->setEnabled(true);

    // 创建相机视图容器
    viewfinder = new QCameraViewfinder(this);
    viewfinder->setMinimumSize(180, 180);
    viewfinder->setMaximumSize(180, 180);

    // 设置圆形遮罩
    viewfinder->setStyleSheet(
        "border-radius: 90px; "
        "background-color: transparent;"
        );

    // 添加到相机容器
    ui->cameraContainerLayout->addWidget(viewfinder);
    viewfinder->setVisible(false);
}

void BigHeadPictureWindow::initCamera()
{
    // 检查可用摄像头
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (cameras.isEmpty()) {
        qDebug() << "未检测到摄像头";
        ui->btnToggleCamera->setEnabled(false);
        ui->btnToggleCamera->setText("无摄像头");
        ui->labelStatus->setText("未检测到摄像头");
        return;
    }

    qDebug() << "找到摄像头:" << cameras.size();

    // 选择第一个摄像头
    QCameraInfo selectedCamera = chooseCamera();

    // 创建相机对象
    camera = new QCamera(selectedCamera, this);
    imageCapture = new QCameraImageCapture(camera, this);

    // 设置相机参数（降低分辨率减少内存）
    QCameraViewfinderSettings settings;
    settings.setResolution(320, 240);  // 低分辨率
    settings.setPixelFormat(QVideoFrame::Format_YUYV);
    settings.setMinimumFrameRate(10.0);
    settings.setMaximumFrameRate(20.0);

    camera->setViewfinderSettings(settings);
    camera->setViewfinder(viewfinder);

    // 连接信号
    connect(imageCapture, &QCameraImageCapture::imageCaptured,
            this, &BigHeadPictureWindow::onImageCaptured);
}
QCameraInfo BigHeadPictureWindow::chooseCamera(){
    const auto all = QCameraInfo::availableCameras();

    /* 1. 优先：描述里带 "USB" / "UVC" / "Web" 的摄像头 */
    for (const QCameraInfo &i : all) {
        const QString desc = i.description().toLower();
        if (desc.contains("usb") || desc.contains("uvc") || desc.contains("web"))
            return i;
    }

    /* 2. 其次：设备名里带 video9 / video8 ... 等 USB 口 */
    for (const QCameraInfo &i : all) {
        if (i.deviceName().contains("video9") ||
            i.deviceName().contains("video8") ||
            i.deviceName().contains("video7"))
            return i;
    }

    /* 3. 最后：随便一个能用的 */
    return all.isEmpty() ? QCameraInfo() : all.first();
}
void BigHeadPictureWindow::initBackgrounds()
{
    // 清空背景列表
    backgrounds.clear();
    bgNames.clear();

    // 添加简单背景（纯色）
    // 背景1：白色
    backgrounds.append(QPixmap(400, 400));
    backgrounds.last().fill(Qt::white);
    bgNames.append("纯白背景");

    // 背景2：浅灰色
    backgrounds.append(QPixmap(400, 400));
    backgrounds.last().fill(Qt::lightGray);
    bgNames.append("浅灰背景");

    // 背景3：浅蓝色
    backgrounds.append(QPixmap(400, 400));
    backgrounds.last().fill(QColor(173, 216, 230));
    bgNames.append("浅蓝背景");

    // 背景4：浅粉色
    backgrounds.append(QPixmap(400, 400));
    backgrounds.last().fill(QColor(255, 182, 193));
    bgNames.append("浅粉背景");

    // 背景5：浅绿色
    backgrounds.append(QPixmap(400, 400));
    backgrounds.last().fill(QColor(152, 251, 152));
    bgNames.append("浅绿背景");

    // 背景6：渐变背景
    QPixmap gradientBg(400, 400);
    QPainter painter(&gradientBg);
    QLinearGradient gradient(0, 0, 400, 400);
    gradient.setColorAt(0, Qt::cyan);
    gradient.setColorAt(1, Qt::blue);
    painter.fillRect(gradientBg.rect(), gradient);
    painter.end();
    backgrounds.append(gradientBg);
    bgNames.append("渐变背景");

    // 更新背景显示
    // updateBackgroundDisplay();
}

void BigHeadPictureWindow::initConnections()
{
    // 相机控制
    connect(ui->btnToggleCamera, &QPushButton::clicked,
            this, &BigHeadPictureWindow::onBtnToggleCameraClicked);
    connect(ui->btnCapture, &QPushButton::clicked,
            this, &BigHeadPictureWindow::onBtnCaptureClicked);

    // 背景控制
    connect(ui->btnPrevBg, &QPushButton::clicked,
            this, &BigHeadPictureWindow::onBtnPrevBgClicked);
    connect(ui->btnNextBg, &QPushButton::clicked,
            this, &BigHeadPictureWindow::onBtnNextBgClicked);
}

void BigHeadPictureWindow::updateUI()
{
    // 更新背景名称标签
    if (currentBgIndex >= 0 && currentBgIndex < bgNames.size()) {
        ui->labelBgName->setText(bgNames[currentBgIndex]);
    }

    // 更新按钮状态
    ui->btnPrevBg->setEnabled(currentBgIndex > 0);
    ui->btnNextBg->setEnabled(currentBgIndex < backgrounds.size() - 1);

    // 更新状态栏
    ui->labelStatus->setText(QString("当前背景: %1/%2")
                                 .arg(currentBgIndex + 1)
                                 .arg(backgrounds.size()));
}

void BigHeadPictureWindow::updateBackgroundDisplay()
{
    if (currentBgIndex >= 0 && currentBgIndex < backgrounds.size()) {
        QPixmap bg = backgrounds[currentBgIndex];

        // 缩放背景以适应显示区域
        QSize labelSize = ui->labelBackground->size();
        QPixmap scaledBg = bg.scaled(labelSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        ui->labelBackground->setPixmap(scaledBg);
    }
}

void BigHeadPictureWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateBackgroundDisplay();
}

void BigHeadPictureWindow::onBtnToggleCameraClicked()
{
    if (!camera) {
        QMessageBox::warning(this, "错误", "摄像头初始化失败");
        return;
    }

    if (!cameraActive) {
        // 开启相机
        try {
            camera->start();

            // 等待相机启动
            QElapsedTimer timer;
            timer.start();
            while (camera->state() != QCamera::ActiveState && timer.elapsed() < 3000) {
                QCoreApplication::processEvents();
                QThread::msleep(50);
            }

            if (camera->state() == QCamera::ActiveState) {
                viewfinder->setVisible(true);
                ui->btnToggleCamera->setText("📷 关闭相机");
                ui->btnCapture->setEnabled(true);
                cameraActive = true;
                ui->labelStatus->setText("相机运行中 - " + bgNames[currentBgIndex]);
            } else {
                QMessageBox::warning(this, "错误", "相机启动失败");
                camera->stop();
            }
        } catch (...) {
            QMessageBox::critical(this, "错误", "相机启动异常");
        }
    } else {
        // 关闭相机
        camera->stop();
        viewfinder->setVisible(false);
        ui->btnToggleCamera->setText("📷 开启相机");
        ui->btnCapture->setEnabled(false);
        cameraActive = false;
        ui->labelStatus->setText("相机已停止 - " + bgNames[currentBgIndex]);
    }
}

void BigHeadPictureWindow::onBtnCaptureClicked()
{
    if (!cameraActive || !imageCapture) {
        QMessageBox::warning(this, "错误", "请先开启相机");
        return;
    }

    ui->btnCapture->setEnabled(false);
    ui->labelStatus->setText("正在拍照...");

    // 捕获图像
    imageCapture->capture();
}

void BigHeadPictureWindow::onImageCaptured(int id, const QImage &preview)
{
    Q_UNUSED(id);

    // 合成大头照
    QPixmap result = combineHeadPicture(preview);

    // 保存到文件
    saveHeadPicture(result);

    // 恢复按钮状态
    ui->btnCapture->setEnabled(true);
}

QPixmap BigHeadPictureWindow::combineHeadPicture(const QImage &cameraImage)
{
    // 创建结果图像（与背景相同大小）
    QPixmap result(400, 400);

    // 第一步：绘制背景
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);

    if (currentBgIndex >= 0 && currentBgIndex < backgrounds.size()) {
        painter.drawPixmap(0, 0, backgrounds[currentBgIndex]);
    } else {
        painter.fillRect(result.rect(), Qt::white);
    }

    // 第二步：绘制相机图像（圆形头像）
    if (!cameraImage.isNull()) {
        // 缩放相机图像以适应圆形区域
        QImage scaledCamera = cameraImage.scaled(200, 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        // 创建圆形蒙版
        QPixmap mask(200, 200);
        mask.fill(Qt::transparent);

        QPainter maskPainter(&mask);
        maskPainter.setRenderHint(QPainter::Antialiasing);
        maskPainter.setBrush(Qt::black);
        maskPainter.setPen(Qt::NoPen);
        maskPainter.drawEllipse(0, 0, 200, 200);
        maskPainter.end();

        // 应用圆形蒙版
        QImage circularImage(200, 200, QImage::Format_ARGB32);
        circularImage.fill(Qt::transparent);

        for (int y = 0; y < 200; ++y) {
            for (int x = 0; x < 200; ++x) {
                QRgb maskPixel = mask.toImage().pixel(x, y);
                if (qAlpha(maskPixel) > 0) {
                    int srcX = (x * scaledCamera.width()) / 200;
                    int srcY = (y * scaledCamera.height()) / 200;

                    if (srcX < scaledCamera.width() && srcY < scaledCamera.height()) {
                        circularImage.setPixel(x, y, scaledCamera.pixel(srcX, srcY));
                    }
                }
            }
        }

        // 在中心位置绘制圆形头像
        int xPos = (400 - 200) / 2;
        int yPos = (400 - 200) / 2;
        painter.drawImage(xPos, yPos, circularImage);

        // 添加白色边框
        painter.setPen(QPen(Qt::white, 3));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(xPos, yPos, 200, 200);
    }

    painter.end();

    return result;
}

void BigHeadPictureWindow::saveHeadPicture(const QPixmap &picture)
{
    // 创建保存目录
    QString picturesDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString saveDir = picturesDir + "/BigHeadPictures";
    QDir().mkpath(saveDir);

    // 生成文件名
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString fileName = saveDir + "/bighead_" + timestamp + ".jpg";

    // 保存图像（降低质量以减少文件大小）
    if (picture.save(fileName, "JPG", 80)) {
        ui->labelStatus->setText("保存成功: " + bgNames[currentBgIndex]);
        QMessageBox::information(this, "保存成功",
                                 QString("大头照已保存\n背景: %1").arg(bgNames[currentBgIndex]));
    } else {
        ui->labelStatus->setText("保存失败");
        QMessageBox::warning(this, "保存失败", "无法保存图片");
    }
}

void BigHeadPictureWindow::onBtnPrevBgClicked()
{
    if (currentBgIndex > 0) {
        currentBgIndex--;
        //updateBackgroundDisplay();
        updateUI();
    }
}

void BigHeadPictureWindow::onBtnNextBgClicked()
{
    if (currentBgIndex < backgrounds.size() - 1) {
        currentBgIndex++;
        //updateBackgroundDisplay();
        updateUI();
    }
}

BigHeadPictureWindow::~BigHeadPictureWindow()
{
    // 停止相机
    if (camera && cameraActive) {
        camera->stop();
    }

    delete ui;
}
