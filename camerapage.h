#ifndef CAMERAPAGE_H
#define CAMERAPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPixmap>
#include <QStringList>
#include "pageflipeffect.h"
#include <QImage>
#include <QDateTime>
#include <QDir>
#include "cvcapture.h"
class CvCapture;

class CameraPage : public QWidget
{
    Q_OBJECT

public:
    explicit CameraPage(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:

    // 设计分辨率
    static constexpr int DESIGN_WIDTH  = 1080;
    static constexpr int DESIGN_HEIGHT = 1920;

    // 当前缩放比例
    double uiScale = 1.0;

    // ==============================
    // UI控件
    // ==============================
    QWidget *topContainer;
    QWidget *topContent;
    QWidget *bottomContainer;

    QLabel *backgroundLabel;
    QLabel *replaceImageLabel;
    QLabel *nextPageLabel;
    QLabel *bottomBackgroundLabel;
    QLabel *bottomTextLabel;
    QLabel *overlayImageLabel;

    QLabel *cameraView;
    QLabel *countdownLabel;

    QPushButton *shootBtn;
    QPushButton *prevBtn;
    QPushButton *nextBtn;

    // ==============================
    // 背景管理
    // ==============================
    QStringList backgroundImages;
    int currentBackgroundIndex;

    // ==============================
    // 动画控制
    // ==============================
    QPropertyAnimation *pageAnimation;
    QParallelAnimationGroup *animationGroup;
    bool isAnimating;

    PageFlipEffect *flipEffect;

    // ==============================
    // 倒计时
    // ==============================
    QTimer *timer;
    int countdown;


    // ==============================
    // 常量 & 配置
    // ==============================
    const qreal TOP_RATIO = 0.7;       // 上半部分占比
    const QSize BUTTON_SIZE = QSize(250, 80);
    const int ARROW_SIZE = 100;

    // ==============================
    // 初始化
    // ==============================
    void initUI();
    void initBackgrounds();
    void initButtons();
    void initCameraAndCountdown();
    void initAnimations();
    void initSignals();

    // ==============================
    // 布局管理
    // ==============================

    //重构布局
    void layoutContainers(int topHeight, int bottomHeight);
    void updateBackgroundGeometry();
    void layoutCameraAndCountdown(int topHeight);
    void layoutOverlayImage(int topHeight);
    void raiseWidgets();
    void layoutCameraAndCountdown();
    void layoutButtons();
    void layoutBottomText();
    void toggleFullScreen();

    // ==============================
    // 背景处理
    // ==============================
    void updateBackground();
    QPixmap loadAndScalePixmap(const QString &path, const QSize &targetSize);

    // ==============================
    // 倒计时控制
    // ==============================
    void startCountdown();
    void updateCountdown();

    // ==============================
    // 阶段控制
    // ==============================
    void enterStage3();
    void enterStage4();

    // ==============================
    // 翻页控制
    // ==============================
    void startPageAnimation(int newIndex, bool toRight);
    void nextBackground();
    void prevBackground();
    void onPageAnimationFinished();

    void reloadCurrentBackground(const QSize &windowSize);
    void reloadNextBackground(const QSize &windowSize);

    void animateButtonBounce(QPushButton *btn);//点击按钮时变形

    int calcArrowButtonSize() const;//箭头大小随窗口动态变动


    CvCapture *m_capture = nullptr;
    QImage m_lastFrame;          // 拍照瞬间的帧
    QString m_savePath;          // 本次照片保存路径
private slots:
    void onNewFrame(const QImage &img);
    void reallyCapture();        // 倒计时到 0 时调用

signals:
    void photoFinished();
    void photoFinished(const QString &filePath);  // 把路径带给上层

};
#endif // CAMERAPAGE_H
