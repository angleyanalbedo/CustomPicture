#include "camerapage.h"
#include <QFont>
#include <QDebug>
#include <QStyle>      // 必须添加这个
#include <QPushButton> // 虽然可能在其他地方包含，但这里明确添加
#include <QIcon>       // 用于图标
#include <QSize>       // 用于设置大小
#include "pageflipeffect.h"  // 添加自定义翻页效果头文件
#include <QScreen>
#include <QGuiApplication>
#include <QResizeEvent>

CameraPage::CameraPage(QWidget *parent)
    : QWidget(parent),
    currentBackgroundIndex(0),      // 先初始化第一个声明的成员
    isAnimating(false),             // 然后按声明顺序
    flipEffect(nullptr),
    pageAnimation(nullptr),
    animationGroup(nullptr),
    countdown(10)
{
    setWindowFlags(windowFlags() | Qt::Window);
    setMinimumSize(400, 300);

    initUI();
    initBackgrounds();
    initButtons();
    initCameraAndCountdown();
    initAnimations();
    timer = new QTimer(this);
    initSignals();

    /* === 摄像头 === */
    m_capture = new CvCapture(0, this);          // /dev/video9
    connect(m_capture, &CvCapture::frameReady, this, &CameraPage::onNewFrame);
    connect(m_capture, &CvCapture::errorString, this, [](const QString &e){
        qDebug() << "Camera error:" << e;
    });
    m_capture->start();

    // 初始化第一个背景
    updateBackground();
    // 初始化第一个背景
    updateBackground();
}
void CameraPage::onNewFrame(const QImage &img)
{
    m_lastFrame = img;                 // 始终保存最新帧
    if (!cameraView->isVisible()) return;
    cameraView->setPixmap(
        QPixmap::fromImage(img).scaled(cameraView->size(),
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation));
}
void CameraPage::updateCountdown()
{
    countdown--;
    countdownLabel->setText(QString::number(countdown));
    if (countdown <= 0) {
        timer->stop();
        countdownLabel->hide();
        reallyCapture();               // <-- 拍照
    }
}

void CameraPage::reallyCapture()
{
    if (m_lastFrame.isNull()) {
        qDebug() << "没有可用帧";
        emit photoFinished({});
        return;
    }
    // 生成文件名
    QString fileName = QString("photo_%1.jpg")
                           .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString fullPath = QDir::currentPath() + "/" + fileName;
    // 保存
    bool ok = m_lastFrame.save(fullPath, nullptr, 95);
    if (!ok) {
        qDebug() << "保存失败";
        emit photoFinished({});
        return;
    }
    m_savePath = fullPath;
    qDebug() << "照片已保存:" << fullPath;

    // 进入后续动画
    enterStage3();
    // 最终动画全部结束后再把路径抛出去
    connect(this,
            qOverload<const QString &>(&CameraPage::photoFinished),
            this,
            [this]{
                emit photoFinished(m_savePath);
            });
}

void CameraPage::initUI()
{
    topContainer = new QWidget(this);
    bottomContainer = new QWidget(this);

    topContainer->setAttribute(Qt::WA_StyledBackground, true);
    bottomContainer->setAttribute(Qt::WA_StyledBackground, true);

    topContent = new QWidget(topContainer);
    topContent->setAttribute(Qt::WA_StyledBackground, false);

    // 背景标签
    backgroundLabel = new QLabel(topContent);
    backgroundLabel->setAlignment(Qt::AlignCenter);

    replaceImageLabel = new QLabel(topContent);
    replaceImageLabel->setAlignment(Qt::AlignCenter);
    replaceImageLabel->hide();

    nextPageLabel = new QLabel(topContainer);
    nextPageLabel->setAlignment(Qt::AlignCenter);
    nextPageLabel->hide();

    bottomBackgroundLabel = new QLabel(bottomContainer);
    bottomBackgroundLabel->setAlignment(Qt::AlignCenter);
    bottomBackgroundLabel->setScaledContents(true);

    bottomTextLabel = new QLabel(bottomContainer);
    bottomTextLabel->setAlignment(Qt::AlignCenter);
    bottomTextLabel->setStyleSheet(
        "QLabel { color: #6b3e26; font-size: 30px; font-weight: 800; letter-spacing: 2px; }"
        );
    bottomTextLabel->show();

    overlayImageLabel = new QLabel(topContainer);
    overlayImageLabel->setAlignment(Qt::AlignCenter);
    overlayImageLabel->hide();
}

void CameraPage::initBackgrounds()
{
    backgroundImages.append(":/images/chrismas.jpg");
    backgroundImages.append(":/images/chrismas1.jpg");

    QPixmap bottomBg(":/images/bottom_bg.png");
    bottomBackgroundLabel->setPixmap(bottomBg);
}

void CameraPage::initButtons()
{
    shootBtn = new QPushButton("立即开拍", bottomContainer);
    shootBtn->setFixedSize(BUTTON_SIZE);
    shootBtn->setStyleSheet(
        "QPushButton { font-size: 20px; font-weight: bold; background-color: #ff4757; color: white; border-radius: 15px; border: 3px solid #ff6b81; }"
        "QPushButton:hover { background-color: #ff6b81; }"
        "QPushButton:pressed { background-color: #ff3838; }"
        );

    // ========= 左右箭头按钮 =========
    prevBtn = new QPushButton(this);
    nextBtn = new QPushButton(this);

    // 1️⃣ 按钮动态大小（和你 layoutButtons 对齐）
    int arrowSize = calcArrowButtonSize();

    prevBtn->setFixedSize(arrowSize, arrowSize);
    nextBtn->setFixedSize(arrowSize, arrowSize);
    prevBtn->setIconSize(QSize(arrowSize, arrowSize));
    nextBtn->setIconSize(QSize(arrowSize, arrowSize));


    prevBtn->setFixedSize(arrowSize, arrowSize);
    nextBtn->setFixedSize(arrowSize, arrowSize);

    // 2️⃣ 设置图片
    QIcon leftIcon(":/images/arrow_left.png");
    QIcon rightIcon(":/images/arrow_right.png");

    prevBtn->setIcon(leftIcon);
    nextBtn->setIcon(rightIcon);

    // 3️⃣ icon 填满按钮
    prevBtn->setIconSize(QSize(arrowSize, arrowSize));
    nextBtn->setIconSize(QSize(arrowSize, arrowSize));

    // 4️⃣ 去掉按钮一切“系统痕迹”
    QString arrowStyle = R"(
    QPushButton {
        background: transparent;
        border: none;
    }
    QPushButton:pressed {
        background: transparent;
    }
    QPushButton:disabled {
        background: transparent;
    }
)";

    prevBtn->setStyleSheet(arrowStyle);
    nextBtn->setStyleSheet(arrowStyle);
}

void CameraPage::initCameraAndCountdown()
{
    cameraView = new QLabel("摄像头画面", topContainer);
    cameraView->setAlignment(Qt::AlignCenter);
    cameraView->setStyleSheet("background:black;color:white;");

    countdownLabel = new QLabel("", topContainer);
    countdownLabel->setAlignment(Qt::AlignCenter);
    QFont f;
    f.setPointSize(48);
    f.setBold(true);
    countdownLabel->setFont(f);
    countdownLabel->setStyleSheet("color:red;");
}

void CameraPage::initAnimations()
{
    pageAnimation = new QPropertyAnimation(this);
    pageAnimation->setDuration(1000);
    pageAnimation->setEasingCurve(QEasingCurve::InOutSine);

    animationGroup = new QParallelAnimationGroup(this);
}

void CameraPage::initSignals()
{
    connect(shootBtn, &QPushButton::clicked, this, &CameraPage::startCountdown);
    connect(timer, &QTimer::timeout, this, &CameraPage::updateCountdown);

    connect(prevBtn, &QPushButton::clicked, this, [=]() {
        animateButtonBounce(prevBtn);
        prevBackground();
    });

    connect(nextBtn, &QPushButton::clicked, this, [=]() {
        animateButtonBounce(nextBtn);
        nextBackground();
    });

    connect(pageAnimation, &QPropertyAnimation::finished, this, &CameraPage::onPageAnimationFinished);
}

QPixmap CameraPage::loadAndScalePixmap(const QString &path, const QSize &targetSize)
{
    QPixmap pixmap(path);
    if (pixmap.isNull()) return QPixmap();

    return pixmap.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

void CameraPage::updateBackground()
{
    if (backgroundImages.isEmpty()) return;

    // 获取当前窗口大小
    QSize windowSize = size();
    if (isFullScreen()) {
        windowSize = screen()->size();
    }
    // 检查是否已经是正确尺寸
    QPixmap currentPixmap = backgroundLabel->pixmap(Qt::ReturnByValue);
    if (!currentPixmap.isNull() && currentPixmap.size() == windowSize) {
        qDebug() << "背景图片已是正确尺寸，跳过重新加载";
        return;
    }
    // 加载原图
    QString imagePath = backgroundImages[currentBackgroundIndex];
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        qDebug() << "错误：无法加载背景图片" << imagePath;
        return;
    }
    // 关键修改：使用高质量缩放，完全填充但不拉伸
    if (pixmap.size() != windowSize) {
        // 计算缩放比例，保持宽高比的同时填充整个区域
        qreal scaleX = (qreal)windowSize.width() / pixmap.width();
        qreal scaleY = (qreal)windowSize.height() / pixmap.height();
        qreal scale = qMax(scaleX, scaleY);  // 取较大的比例确保完全填充

        // 计算缩放后的尺寸
        QSize scaledSize = pixmap.size() * scale;

        // 高质量缩放
        QPixmap scaledPixmap = pixmap.scaled(scaledSize,
                                             Qt::KeepAspectRatio,  // 保持宽高比
                                             Qt::SmoothTransformation);  // 平滑变换

        // 如果缩放后尺寸大于窗口，裁剪到窗口大小
        if (scaledSize.width() > windowSize.width() ||
            scaledSize.height() > windowSize.height()) {

            // 计算裁剪区域（居中裁剪）
            int x = (scaledSize.width() - windowSize.width()) / 2;
            int y = (scaledSize.height() - windowSize.height()) / 2;
            scaledPixmap = scaledPixmap.copy(x, y, windowSize.width(), windowSize.height());
        }

        backgroundLabel->setPixmap(scaledPixmap);
    } else {
        // 图片大小正好匹配窗口
        backgroundLabel->setPixmap(pixmap);
    }

}

// 重载 resizeEvent 以确保背景更新，适应全屏模式
void CameraPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // 计算布局比例
    int topHeight = height() * 0.7;
    int bottomHeight = height() - topHeight;

    // 调整容器位置
    layoutContainers(topHeight, bottomHeight);

    // 调整背景和翻页图片
    updateBackgroundGeometry();

    // 调整摄像头和倒计时位置
    layoutCameraAndCountdown(topHeight);

    // 调整按钮位置
    layoutButtons();

    // 调整底部文字
    layoutBottomText();

    // 调整覆盖图片
    layoutOverlayImage(topHeight);

    // 控件层级管理
    raiseWidgets();
}

// 当窗口显示事件发生时更新背景
void CameraPage::showEvent(QShowEvent *event)
{
    // QWidget::showEvent(event);
    // // 确保背景在显示时正确更新
    // QTimer::singleShot(0, this, &CameraPage::updateBackground);
    QWidget::showEvent(event);


    // 延迟执行，确保窗口完全显示后再加载背景
    QTimer::singleShot(100, this, [=]() {
        // 确保背景在显示时正确更新
        updateBackground();

        // 确保所有控件可见
        backgroundLabel->show();
        cameraView->show();
        countdownLabel->show();
        shootBtn->show();
        prevBtn->show();
        nextBtn->show();

        // 触发一次resizeEvent来设置所有控件位置
        QResizeEvent re(size(), QSize());
        this->resizeEvent(&re);
    });
}

void CameraPage::startCountdown()
{
    // 所有按钮消失
    shootBtn->hide();
    prevBtn->hide();
    nextBtn->hide();

    bottomTextLabel->setText("请看镜头");
    bottomTextLabel->show();          // 2. 显示提示文字

    // ===== 禁用所有操作按钮 =====
    shootBtn->setEnabled(false);

    // ===== 初始化倒计时 =====
    countdown = 10;                    // 建议先 3，调试更舒服
    countdownLabel->setText(QString::number(countdown));
    countdownLabel->show();

    timer->start(1000);

}



void CameraPage::nextBackground()
{
    if (backgroundImages.isEmpty() || isAnimating) return;

    int nextIndex = (currentBackgroundIndex + 1) % backgroundImages.size();
    startPageAnimation(nextIndex, true);  // true 表示向右翻页
    currentBackgroundIndex = nextIndex;
}

void CameraPage::prevBackground()
{
    if (backgroundImages.isEmpty() || isAnimating) return;

    int prevIndex = (currentBackgroundIndex - 1 + backgroundImages.size()) % backgroundImages.size();
    startPageAnimation(prevIndex, false);  // false 表示向左翻页
    currentBackgroundIndex = prevIndex;
}

void CameraPage::toggleFullScreen()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

//倒计时结束阶段
void CameraPage::enterStage3()
{
    // 下半部分文字更新
    bottomTextLabel->setText(
        "正在生成你的\n"
        "今日大日报"
        );

    QFont font;
    font.setPointSize(28);
    font.setBold(true);
    bottomTextLabel->setFont(font);
    bottomTextLabel->setAlignment(Qt::AlignCenter);
    bottomTextLabel->show();

    // 稍等一下再进入阶段4（制造节奏）
    QTimer::singleShot(1200, this, &CameraPage::enterStage4);
}

void CameraPage::enterStage4()
{
    int H = topContainer->height();
    int W = topContainer->width();

    topContainer->setFixedSize(W, H);
    topContainer->setMask(QRegion(0, 0, W, H));


    // 1️⃣ 创建滚动容器（只在第一次创建）
    if (!topContent) {
        topContent = new QWidget(topContainer);
    }

    topContent->setGeometry(0, -H, W, H * 2); // 初始在上方

    // 2️⃣ 新背景（蓝）—— 在上
    QPixmap newPix(":/images/paper.jpg");
    QLabel *newBg = replaceImageLabel;
    newBg->setParent(topContent);
    newBg->setPixmap(
        newPix.scaled(W, H, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)
        );
    newBg->setGeometry(0, 0, W, H);
    newBg->show();

    // 3️⃣ 旧背景（红）—— 在下，携带摄像头
    backgroundLabel->setParent(topContent);
    backgroundLabel->setGeometry(0, H, W, H);

    cameraView->setParent(backgroundLabel);
    countdownLabel->setParent(backgroundLabel);

    cameraView->raise();
    countdownLabel->raise();

    // 4️⃣ 匀速向下滚动（核心）
    QPropertyAnimation *anim =
        new QPropertyAnimation(topContent, "pos", this);

    anim->setDuration(2500);                  // 慢，才有质感
    anim->setStartValue(QPoint(0, -H));
    anim->setEndValue(QPoint(0, 0));
    anim->setEasingCurve(QEasingCurve::Linear); // 匀速！

    connect(anim, &QPropertyAnimation::finished, this, [=]() {
        // 1️⃣ 停止裁剪动画容器
        topContent->hide();

        // 2️⃣ 正式切换为新背景
        backgroundLabel->setParent(topContainer);
        backgroundLabel->setPixmap(
            replaceImageLabel->pixmap(Qt::ReturnByValue)
            );
        backgroundLabel->setGeometry(0, 0, W, H);
        backgroundLabel->show();

        // 3️⃣ 摄像头回到 topContainer（此时已经在新背景上）
        cameraView->setParent(topContainer);
        cameraView->raise();

        // 4️⃣ 清理
        replaceImageLabel->hide();
        topContent->move(0, 0);
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// 启动翻页动画
void CameraPage::startPageAnimation(int newIndex, bool toRight)
{
    if (isAnimating) {
        qDebug() << "正在动画中，跳过";
        return;
    }

    isAnimating = true;



    // 加载新背景
    QSize windowSize = size();

    if (isFullScreen()) {
        windowSize = screen()->size();
    }

    QPixmap newPixmap(backgroundImages[newIndex]);
    if (newPixmap.isNull()) {
        qDebug() << "错误：无法加载新背景图片";
        prevBtn->setEnabled(true);
        nextBtn->setEnabled(true);
        isAnimating = false;
        return;
    }

    int topHeight = windowSize.height() * 0.7;
    // 高质量缩放新图片
    // QPixmap scaledNewPixmap = newPixmap.scaled(windowSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap scaledNewPixmap =
        newPixmap.scaled(
            QSize(windowSize.width(), topHeight),
            Qt::KeepAspectRatioByExpanding,
            Qt::SmoothTransformation
            );
    nextPageLabel->setPixmap(scaledNewPixmap);

    // 设置起始位置
    QRect startRect, endRect;


    if (toRight) {
        // 新页面从右侧进入
        // startRect = QRect(windowSize.width(), 0, windowSize.width(), windowSize.height());
        startRect = QRect(windowSize.width(), 0, windowSize.width(), topHeight);
        // endRect = QRect(0, 0, windowSize.width(), windowSize.height());
        endRect = QRect(0, 0, windowSize.width(), topHeight);

    } else {
        // 新页面从左侧进入
        startRect = QRect(-windowSize.width(), 0, windowSize.width(), windowSize.height());
        endRect = QRect(0, 0, windowSize.width(), windowSize.height());
    }

    // 显示新页面
    nextPageLabel->setGeometry(startRect);
    nextPageLabel->show();
    nextPageLabel->raise();

    nextPageLabel->raise();
    cameraView->raise();
    countdownLabel->raise();
    shootBtn->raise();
    prevBtn->raise();    // 确保按钮在动画页面上层
    nextBtn->raise();    // 确保按钮在动画页面上层

    // 创建滑动动画
    QPropertyAnimation *slideAnimation = new QPropertyAnimation(nextPageLabel, "geometry", this);
    slideAnimation->setDuration(800);
    slideAnimation->setStartValue(startRect);
    slideAnimation->setEndValue(endRect);
    slideAnimation->setEasingCurve(QEasingCurve::OutCubic);

    // 同时让当前页面滑出
    QPropertyAnimation *currentSlideAnimation = new QPropertyAnimation(backgroundLabel, "geometry", this);
    QRect currentEndRect;

    if (toRight) {
        currentEndRect = QRect(-windowSize.width() / 2, 0, windowSize.width(), windowSize.height());
    } else {
        currentEndRect = QRect(windowSize.width() / 2, 0, windowSize.width(), windowSize.height());
    }

    currentSlideAnimation->setDuration(800);
    // currentSlideAnimation->setStartValue(QRect(0, 0, windowSize.width(), windowSize.height()));
    currentSlideAnimation->setStartValue(QRect(0, 0, windowSize.width(), topHeight));
    currentSlideAnimation->setEndValue(currentEndRect);
    currentSlideAnimation->setEasingCurve(QEasingCurve::OutCubic);

    // 创建动画组
    QParallelAnimationGroup *animationGroup = new QParallelAnimationGroup(this);
    animationGroup->addAnimation(slideAnimation);
    animationGroup->addAnimation(currentSlideAnimation);

    // 连接动画完成信号
    connect(animationGroup, &QParallelAnimationGroup::finished, this, [=]() {

        // 动画完成后，更新当前页面
        QPixmap nextPixmap = nextPageLabel->pixmap(Qt::ReturnByValue);
        if (!nextPixmap.isNull()) {
            backgroundLabel->setPixmap(nextPixmap);
        }

        // 重置当前页面位置
        // backgroundLabel->setGeometry(0, 0, windowSize.width(), windowSize.height());
        backgroundLabel->setGeometry(0, 0, windowSize.width(), topHeight);

        // 隐藏下一页标签
        nextPageLabel->hide();


        // 恢复按钮状态
        prevBtn->setEnabled(true);
        nextBtn->setEnabled(true);

        // 重置动画状态
        isAnimating = false;

        // 清理动画对象
        animationGroup->deleteLater();
    });

    // 启动动画
    animationGroup->start();
}

// 动画完成后的处理
void CameraPage::onPageAnimationFinished()
{
    // 动画完成后，更新当前页面
    QPixmap nextPixmap = nextPageLabel->pixmap(Qt::ReturnByValue);
    if (!nextPixmap.isNull()) {
        backgroundLabel->setPixmap(nextPixmap);
    }

    // 隐藏下一页标签并清除效果
    nextPageLabel->hide();
    nextPageLabel->setGraphicsEffect(nullptr);

    if (flipEffect) {
        flipEffect->deleteLater();
        flipEffect = nullptr;
    }

    // 恢复按钮状态
    prevBtn->setEnabled(true);
    nextBtn->setEnabled(true);

    isAnimating = false;
}

// 1️⃣ 容器布局
void CameraPage::layoutContainers(int topHeight, int bottomHeight)
{
    topContainer->setGeometry(0, 0, width(), topHeight);
    bottomContainer->setGeometry(0, topHeight, width(), bottomHeight);
    topContent->setGeometry(0, 0, width(), topHeight * 2); // 动画准备
}

// 2️⃣ 背景与翻页图片
void CameraPage::updateBackgroundGeometry()
{
    backgroundLabel->setGeometry(0, 0, topContainer->width(), topContainer->height());
    replaceImageLabel->setGeometry(0, 0, topContainer->width(), topContainer->height());
    nextPageLabel->setGeometry(0, 0, topContainer->width(), topContainer->height());

    bottomBackgroundLabel->setGeometry(0, 0, bottomContainer->width(), bottomContainer->height());

    QSize windowSize = topContainer->size(); // 关键：使用topContainer的尺寸，而不是整个窗口
    if (windowSize.width() <= 0 || windowSize.height() <= 0) return;

    reloadCurrentBackground(windowSize);
    if (nextPageLabel->isVisible()) {
        reloadNextBackground(windowSize);
    }
}

// 3️⃣ 摄像头与倒计时布局
void CameraPage::layoutCameraAndCountdown(int topHeight)
{
    QSize windowSize = size();
    int cameraWidth = qMin(windowSize.width() / 2, windowSize.width() - 40);
    int cameraHeight = qMin(windowSize.height() * 4 / 10, windowSize.height() - 200);
    cameraHeight = qMax(100, cameraHeight);

    int cameraX = (windowSize.width() - cameraWidth) / 2;
    int cameraY = qMax(50, static_cast<int>(topHeight * 0.15));

    cameraView->setGeometry(cameraX, cameraY, cameraWidth, cameraHeight);
    countdownLabel->setGeometry(cameraX, cameraY, cameraWidth, cameraHeight);

    // 设置字体大小自适应
    QFont f;
    int fontSize = qBound(48, cameraHeight / 2, 120);
    f.setPointSize(fontSize);
    f.setBold(true);
    countdownLabel->setFont(f);
    countdownLabel->setAlignment(Qt::AlignCenter);
    countdownLabel->setStyleSheet(
        "QLabel { color: #FF0000; background-color: rgba(0,0,0,120); }"
        );
}

// 4️⃣ 按钮布局
void CameraPage::layoutButtons()
{
    QSize windowSize = size();

    // "立即开拍"按钮
    int shootBtnWidth = 250, shootBtnHeight = 80;
    int shootBtnX = (bottomContainer->width() - shootBtnWidth) / 2;
    int shootBtnY = (bottomContainer->height() - shootBtnHeight) / 2;
    shootBtnY = qMin(shootBtnY, windowSize.height() - shootBtnHeight - 20);
    shootBtn->setGeometry(shootBtnX, shootBtnY, shootBtnWidth, shootBtnHeight);

    // 箭头按钮
         //动态箭头按钮大小
    int arrowBtnSize = calcArrowButtonSize();
    prevBtn->setFixedSize(arrowBtnSize, arrowBtnSize);
    nextBtn->setFixedSize(arrowBtnSize, arrowBtnSize);
    prevBtn->setIconSize(QSize(arrowBtnSize, arrowBtnSize));
    nextBtn->setIconSize(QSize(arrowBtnSize, arrowBtnSize));

    int cameraX = cameraView->x();
    int cameraY = cameraView->y();
    int cameraHeight = cameraView->height();
    int cameraWidth = cameraView->width();
    int arrowBtnY = cameraY + (cameraHeight - arrowBtnSize) / 2;
    arrowBtnY = qBound(20, arrowBtnY, windowSize.height() - arrowBtnSize - 20);

    int prevBtnX = qMax(10, cameraX - arrowBtnSize - 10);
    int nextBtnX = qMin(windowSize.width() - arrowBtnSize - 10, cameraX + cameraWidth + 10);

    prevBtn->setGeometry(prevBtnX, arrowBtnY, arrowBtnSize, arrowBtnSize);
    nextBtn->setGeometry(nextBtnX, arrowBtnY, arrowBtnSize, arrowBtnSize);
}

// 5️⃣ 底部文字
void CameraPage::layoutBottomText()
{
    int textHeight = 80;
    bottomTextLabel->setGeometry(
        0,
        shootBtn->y() + shootBtn->height() + 15,
        bottomContainer->width(),
        textHeight
        );
}

// 6️⃣ 覆盖图片
void CameraPage::layoutOverlayImage(int topHeight)
{
    overlayImageLabel->setGeometry(0, 0, topContainer->width(), topHeight);
}

// 7️⃣ 控件层级
void CameraPage::raiseWidgets()
{
    backgroundLabel->lower();
    bottomBackgroundLabel->lower();
    overlayImageLabel->raise();
    nextPageLabel->raise();
    cameraView->raise();
    countdownLabel->raise();
    shootBtn->raise();
    prevBtn->raise();
    nextBtn->raise();
}

void CameraPage::reloadCurrentBackground(const QSize &windowSize)
{
    if (currentBackgroundIndex >= 0 && currentBackgroundIndex < backgroundImages.size()) {
        QString path = backgroundImages[currentBackgroundIndex];
        QPixmap pix(path);
        if (!pix.isNull()) {
            backgroundLabel->setPixmap(pix.scaled(windowSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        }
    }
}

void CameraPage::reloadNextBackground(const QSize &windowSize)
{
    int nextIndex = (currentBackgroundIndex + 1) % backgroundImages.size();
    if (nextIndex >= 0 && nextIndex < backgroundImages.size()) {
        QString path = backgroundImages[nextIndex];
        QPixmap pix(path);
        if (!pix.isNull()) {
            nextPageLabel->setPixmap(pix.scaled(windowSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        }
    }
}

void CameraPage::animateButtonBounce(QPushButton *btn)
{
    QRect normal = btn->geometry();

    int dw = normal.width() * 0.04;
    int dh = normal.height() * 0.04;

    QRect shrink(
        normal.x() + dw,
        normal.y() + dh,
        normal.width() - 2 * dw,
        normal.height() - 2 * dh
        );

    // 压下
    QPropertyAnimation *pressAnim =
        new QPropertyAnimation(btn, "geometry");
    pressAnim->setDuration(80);
    pressAnim->setStartValue(normal);
    pressAnim->setEndValue(shrink);
    pressAnim->setEasingCurve(QEasingCurve::OutQuad);

    // 回弹
    QPropertyAnimation *releaseAnim =
        new QPropertyAnimation(btn, "geometry");
    releaseAnim->setDuration(120);
    releaseAnim->setStartValue(shrink);
    releaseAnim->setEndValue(normal);
    releaseAnim->setEasingCurve(QEasingCurve::OutBack);

    connect(pressAnim, &QPropertyAnimation::finished, [=]() {
        releaseAnim->start(QAbstractAnimation::DeleteWhenStopped);
    });

    pressAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

int CameraPage::calcArrowButtonSize() const
{
    int w = width();

    // 设计基准宽度（你 UI 设计时参考的）
    const int DESIGN_WIDTH = 1280;

    // 按钮尺寸区间
    const int NORMAL_SIZE = 180;   // 1280 宽时的标准大小
    const int MIN_SIZE    = 100;   // 最小
    const int MAX_SIZE    = 240;   // 最大

    // 防止 width 为 0
    if (w <= 0)
        return NORMAL_SIZE;

    // ✅ 正确的比例：窗口越小，scale 越小
    double scale = static_cast<double>(w) / DESIGN_WIDTH;

    // 限制缩放范围，防止过大 / 过小
    scale = qBound(0.6, scale, 1.3);

    int size = static_cast<int>(NORMAL_SIZE * scale);

    return qBound(MIN_SIZE, size, MAX_SIZE);
}
