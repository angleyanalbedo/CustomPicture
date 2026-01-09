#ifndef BIGHEADPICTUREWINDOW_H
#define BIGHEADPICTUREWINDOW_H

#include "qcamerainfo.h"
#include <QMainWindow>
#include <QObject>
// 前置声明
class QCamera;
class QCameraViewfinder;
class QCameraImageCapture;
class QPixmap;
class QPushButton;
class QElapsedTimer;

namespace Ui {
class BigHeadPictureWindow;
}

class BigHeadPictureWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BigHeadPictureWindow(QWidget *parent = nullptr);
    QCameraInfo chooseCamera();
    ~BigHeadPictureWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onBtnToggleCameraClicked();
    void onBtnCaptureClicked();
    void onBtnPrevBgClicked();
    void onBtnNextBgClicked();

    void onImageCaptured(int id, const QImage &preview);

private:
    void initUI();
    void initCamera();
    void initBackgrounds();
    void initConnections();
    void updateUI();
    void updateBackgroundDisplay();

    QPixmap combineHeadPicture(const QImage &cameraImage);
    void saveHeadPicture(const QPixmap &picture);

private:
    Ui::BigHeadPictureWindow *ui;

    // 相机相关
    QCamera *camera;
    QCameraViewfinder *viewfinder;
    QCameraImageCapture *imageCapture;
    bool cameraActive;

    // 背景相关
    QList<QPixmap> backgrounds;
    QStringList bgNames;
    int currentBgIndex;
};

#endif // BIGHEADPICTUREWINDOW_H
