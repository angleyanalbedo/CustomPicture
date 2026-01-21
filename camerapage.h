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

class CameraPage : public QWidget
{
    Q_OBJECT

public:
    explicit CameraPage(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
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

signals:
    void photoFinished();

};
#endif // CAMERAPAGE_H
