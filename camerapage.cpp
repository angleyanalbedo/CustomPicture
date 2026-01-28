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
#include "imgproc.h"
#include <QDate>

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

    // 1. 准备背景图 (从 Qt 资源加载并转为 Mat)
    QImage bgQImage(":/images/paper.png");
    if (bgQImage.isNull()) {
        qDebug() << "无法加载背景资源";
        return;
    }
    cv::Mat bgMat = ImgProc::qImageToMat(bgQImage);

    // 2. 准备前景图 (当前的拍照帧)
    cv::Mat fgMat = ImgProc::qImageToMat(m_lastFrame);

    // 3. 执行合成
    // 这里传入你希望图片出现在报纸上的位置，例如 x=260, y=260, w=450, h=310
    cv::Rect targetArea(328, 501, 1028, 661);
    cv::Mat resultMat = ImgProc::embedImage(bgMat, fgMat, targetArea);

    // 4. 生成最终保存路径
    QString fileName = QString("newspaper_%1.png")
                           .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString fullPath = QDir::currentPath() + "/" + fileName;

    // 5. 使用 OpenCV 保存最终合成图
    std::string exportPath = fullPath.toLocal8Bit().toStdString();
    bool ok = cv::imwrite(exportPath, resultMat);

    if (!ok) {
        qDebug() << "OpenCV 保存合成图失败";
        // emit photoFinished({});
        return;
    }

    m_savePath = fullPath;
    qDebug() << "合成照片已保存:" << fullPath;

    // 进入后续动画
    enterStage3();

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
        "QLabel { color: #6b3e26; font-size: 15px; font-weight: 600; letter-spacing: 1px; }"
        );
    bottomTextLabel->setText("点击立即开拍\n即可获得同款大日报");
    bottomTextLabel->show();

    overlayImageLabel = new QLabel(topContainer);
    overlayImageLabel->setAlignment(Qt::AlignCenter);
    overlayImageLabel->hide();

    // ===== 左上角日期显示 =====
    dateLabel = new QLabel(this);
    dateLabel->setText(currentDateString());
    dateLabel->setStyleSheet(
        "QLabel {"
        " color: #5A3A1E;"        // 淡棕色（类似纸张色调）
        " font-size: 12px;"       // 字体稍微小一点
        " font-weight: normal;"   // 细一点
        " background: transparent;"
        "}"
        );
    dateLabel->show();

}

void CameraPage::initBackgrounds()
{
    backgroundImages.append(":/images/paper_module1.png");
    backgroundImages.append(":/images/paper_module2.jpg");


    QPixmap bottomBg(":/images/bottom_bg.png");
    bottomBackgroundLabel->setPixmap(bottomBg);
}

void CameraPage::initButtons()
{
    shootBtn = new QPushButton("立即开拍", bottomContainer);

    shootBtn->setStyleSheet(
        "QPushButton { "
        "   font-size: 12px; "             // 文字大小
        "   font-weight: bold; "           // 加粗
        "   color: white; "                // 文字白色
        "   background-color: #3D2B1F; "   // 黑褐色背景 (你可以根据需要微调这个色值)
        "   border-radius: 8px; "         // 圆角：值越大越圆。设为高度的一半可实现全圆角
        "   border: none; "                // 去掉原来的红色边框
        "} "
        "QPushButton:hover { "
        "   background-color: #4D3B2F; "   // 鼠标悬停略微变亮
        "} "
        "QPushButton:pressed { "
        "   background-color: #2D1B0F; "   // 按下略微变暗
        "}"
        );

    // ========= 左右箭头按钮 =========
    // 修改父对象为 bottomContainer
    prevBtn = new QPushButton(topContainer);
    nextBtn = new QPushButton(topContainer);

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
        // // 计算缩放比例，保持宽高比的同时填充整个区域
        // qreal scaleX = (qreal)windowSize.width() / pixmap.width();
        // qreal scaleY = (qreal)windowSize.height() / pixmap.height();
        // qreal scale = qMax(scaleX, scaleY);  // 取较大的比例确保完全填充

        // // 计算缩放后的尺寸
        // QSize scaledSize = pixmap.size() * scale;

        // // 高质量缩放
        // QPixmap scaledPixmap = pixmap.scaled(scaledSize,
        //                                      Qt::KeepAspectRatio,  // 保持宽高比
        //                                      Qt::SmoothTransformation);  // 平滑变换

        // // 如果缩放后尺寸大于窗口，裁剪到窗口大小
        // if (scaledSize.width() > windowSize.width() ||
        //     scaledSize.height() > windowSize.height()) {

        //     // 计算裁剪区域（居中裁剪）
        //     int x = (scaledSize.width() - windowSize.width()) / 2;
        //     int y = (scaledSize.height() - windowSize.height()) / 2;
        //     scaledPixmap = scaledPixmap.copy(x, y, windowSize.width(), windowSize.height());
        // }

        // backgroundLabel->setPixmap(scaledPixmap);

        backgroundLabel->setScaledContents(true);
        backgroundLabel->setPixmap(pixmap);

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
    int topHeight = height() * 0.8;
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

    // ===== 左上角日期位置 =====
    if (dateLabel) {
        dateLabel->move(10, 5);  // 左 20，上 15，可微调
        dateLabel->raise();
    }

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
    hideDateLabel();   // 👈 一进入倒计时就隐藏日期

    // 所有按钮消失
    shootBtn->hide();
    prevBtn->hide();
    nextBtn->hide();

    bottomTextLabel->setText(
        "倒计时结束前摆好 pose\n"
        "笑一笑更好看"
        );
    bottomTextLabel->show();          // 2. 显示提示文字

    // ===== 禁用所有操作按钮 =====
    shootBtn->setEnabled(false);

    // ===== 初始化倒计时 =====
    countdown = 3;                    // 建议先 3，调试更舒服
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
    font.setPointSize(20);
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

    if (!topContent) {
        topContent = new QWidget(topContainer);
    }

    topContent->setGeometry(0, -H, W, H * 2); // 动画容器初始位置

    // --- 上背景：动画用 ---
    QPixmap animPix(":/images/paper.png");
    QLabel *animBg = replaceImageLabel;
    animBg->setParent(topContent);
    animBg->setPixmap(animPix.scaled(
        W, H,
        Qt::KeepAspectRatioByExpanding,
        Qt::SmoothTransformation));
    animBg->setGeometry(0, 0, W, H);
    animBg->show();

    // --- 下背景：原来的 backgroundLabel（动画用） ---
    backgroundLabel->setParent(topContent);
    backgroundLabel->setGeometry(0, H, W, H); // 在下方
    cameraView->setParent(backgroundLabel);
    countdownLabel->setParent(backgroundLabel);
    cameraView->raise();
    countdownLabel->raise();

    // --- 下拉动画，慢一倍 ---
    QPropertyAnimation *anim = new QPropertyAnimation(topContent, "pos", this);
    anim->setDuration(5000);
    anim->setStartValue(QPoint(0, -H));
    anim->setEndValue(QPoint(0, 0));
    anim->setEasingCurve(QEasingCurve::Linear);

    connect(anim, &QPropertyAnimation::finished, this, [=]() {

        // ===============================
        // 1. 上背景固定
        // ===============================
        animBg->setParent(topContainer);
        animBg->setGeometry(0, 0, W, H);
        animBg->show();

        // ===============================
        // 2. ⭐ 真正的下容器背景切换（关键修改）
        // ===============================
        QString bottomBgPath = ":/images/bottom_bg4.png"; // ← 换成你指定路径
        QPixmap finalBg = loadAndScalePixmap(bottomBgPath, bottomContainer->size());

        if (!finalBg.isNull()) {
            bottomBackgroundLabel->setPixmap(finalBg);
            bottomBackgroundLabel->setScaledContents(true);
            bottomBackgroundLabel->setGeometry(bottomContainer->rect());
            bottomBackgroundLabel->show();
            bottomBackgroundLabel->lower(); // 确保文字在上面
        }

        // ===============================
        // 3. 摄像头恢复
        // ===============================
        cameraView->setParent(topContainer);
        cameraView->raise();

        // ===============================
        // 4. 清理动画容器
        // ===============================
        bottomTextLabel->hide();
        topContent->hide();
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// 启动翻页动画
void CameraPage::startPageAnimation(int newIndex, bool toRight)
{
    hideDateLabel();
    if (isAnimating) return;
    isAnimating = true;

    const int W = topContainer->width();
    const int H = topContainer->height();

    // 1️⃣ 准备新页面 pixmap（完全匹配 QLabel 大小）
    QPixmap newPixmap(backgroundImages[newIndex]);
    if (newPixmap.isNull()) {
        isAnimating = false;
        return;
    }

    // ⚠️ 这里直接缩放到 W x H，不保持比例
    QPixmap fixedPixmap = newPixmap.scaled(W, H, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    nextPageLabel->setPixmap(fixedPixmap);
    nextPageLabel->setFixedSize(W, H);

    // 2️⃣ 初始 / 结束位置
    QRect newStart, newEnd(0, 0, W, H);
    QRect oldStart(0, 0, W, H), oldEnd;

    if (toRight) {
        newStart = QRect(W, 0, W, H);
        oldEnd   = QRect(-W, 0, W, H);
    } else {
        newStart = QRect(-W, 0, W, H);
        oldEnd   = QRect(W, 0, W, H);
    }

    nextPageLabel->setGeometry(newStart);
    nextPageLabel->show();
    nextPageLabel->raise();

    // 3️⃣ 摄像头 & UI 永远在最上层
    cameraView->raise();
    countdownLabel->raise();
    prevBtn->raise();
    nextBtn->raise();

    // 4️⃣ 动画
    QPropertyAnimation *inAnim = new QPropertyAnimation(nextPageLabel, "pos");
    inAnim->setDuration(800);
    inAnim->setStartValue(newStart.topLeft());
    inAnim->setEndValue(newEnd.topLeft());
    inAnim->setEasingCurve(QEasingCurve::OutCubic);

    QPropertyAnimation *outAnim = new QPropertyAnimation(backgroundLabel, "pos");
    outAnim->setDuration(800);
    outAnim->setStartValue(oldStart.topLeft());
    outAnim->setEndValue(oldEnd.topLeft());
    outAnim->setEasingCurve(QEasingCurve::OutCubic);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(inAnim);
    group->addAnimation(outAnim);

    connect(group, &QParallelAnimationGroup::finished, this, [=]() {
        backgroundLabel->setPixmap(nextPageLabel->pixmap(Qt::ReturnByValue));
        backgroundLabel->setGeometry(0, 0, W, H);

        nextPageLabel->hide();

        cameraView->raise();
        countdownLabel->raise();
        prevBtn->raise();
        nextBtn->raise();

        showDateLabel();//显示日期

        isAnimating = false;
        group->deleteLater();
    });

    group->start();
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
    if (!isAnimating) {
        topContent->setGeometry(0, 0, width(), topHeight * 2);
    }
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
    // if (nextPageLabel->isVisible()) {
    //     reloadNextBackground(windowSize);
    // }
}

// 3️⃣ 摄像头与倒计时布局
void CameraPage::layoutCameraAndCountdown(int topHeight)
{
    QSize windowSize = size();
    int topWidth = windowSize.width();

    // --- 1. 布局 cameraView (保持你之前的居中逻辑) ---
    // int cameraWidth = topWidth;
    // int cameraHeight = topHeight/4;
    // cameraHeight = qMax(100, cameraHeight);

    // int cameraX = 0;
    // int cameraY = topHeight/4;
    // cameraView->setGeometry(cameraX, cameraY, cameraWidth, cameraHeight);
    int W = topContainer->width();
    int H = topContainer->height();

    cameraView->setGeometry(
        int(W * 0.0742),
        int(H * 0.2776),
        int(W * 0.8524),
        int(H * 0.3450)
        );

    // --- 2. 布局 countdownLabel (新的要求) ---
    // 位置：从 topHeight 的 1/4 开始
    int countdownY = topHeight / 4;
    // 高度：到 1/2 结束，所以高度也是 topHeight 的 1/4
    int countdownHeight = topHeight / 4;




    // 获取摄像头画面的几何信息
    QRect camRect = cameraView->geometry();

    // 倒计时大小（可以跟摄像头比例相关）
    int countdownW = camRect.width();
    int countdownH = camRect.height();

    // 设置倒计时位置：完全覆盖摄像头区域（居中显示）
    countdownLabel->setGeometry(
        camRect.x(),
        camRect.y(),
        countdownW,
        countdownH
        );


    // --- 3. 字体与样式 ---
    // 根据 countdownHeight 动态计算字号，确保视觉比例协调
    int fontSize = qBound(40, countdownHeight / 2, 150);

    QFont f;
    f.setPixelSize(fontSize); // 使用 PixelSize 在不同分辨率下更稳定
    f.setBold(true);

    countdownLabel->setFont(f);
    countdownLabel->setAlignment(Qt::AlignCenter);

    // 注意：既然要跨越背景，通常背景设为透明
    countdownLabel->setStyleSheet(
        "QLabel { color: #FF0000; background-color: transparent; }"
        );


    // 确保倒计时在最顶层，不被 cameraView 遮挡
    countdownLabel->raise();
}

// 4️⃣ 按钮布局
void CameraPage::layoutButtons()
{
    /* ===== 1️⃣ 拍照按钮：仍然在 bottomContainer ===== */
    int bw = bottomContainer->width();
    int bh = bottomContainer->height();
    printf("%d\n",&bw);

    int shootW = 120;
    int shootH = 30;

    shootBtn->setGeometry(
        (bw - shootW) / 2,
        (bh - shootH) / 2-15,
        shootW,
        shootH
        );

    /* ===== 2️⃣ 左右箭头：回到 topContainer 两侧 ===== */
    int arrowSize = calcArrowButtonSize();

    prevBtn->setFixedSize(arrowSize, arrowSize);
    nextBtn->setFixedSize(arrowSize, arrowSize);
    prevBtn->setIconSize(QSize(arrowSize, arrowSize));
    nextBtn->setIconSize(QSize(arrowSize, arrowSize));

    int topW = topContainer->width();
    // ------------------- 修改重点开始 -------------------

    // 1. 获取摄像头控件当前的几何信息 (x, y, width, height)
    QRect camRect = cameraView->geometry();

    // 2. 计算摄像头的垂直中心点 Y 坐标
    // (摄像头顶部Y + 摄像头高度的一半)
    int camCenterY = camRect.y() + (camRect.height()*3 / 5);

    // 3. 计算按钮的 Top Y 坐标
    // (摄像头中心点 - 按钮高度的一半)，这样按钮中心就会对齐摄像头中心
    int btnY = camCenterY - (arrowSize / 2);

    // 4. 设置左右边距 (如果觉得离屏幕边缘太近/太远，调整这个 margin 值)
    int margin = 5;

    // 左箭头位置
    prevBtn->setGeometry(
        margin,
        btnY,           // 使用计算出的新 Y 坐标
        arrowSize,
        arrowSize
        );

    // 右箭头位置
    nextBtn->setGeometry(
        topW - arrowSize - margin,
        btnY,           // 使用计算出的新 Y 坐标
        arrowSize,
        arrowSize
        );

    // ------------------- 修改重点结束 -------------------

    /* ===== 层级 ===== */
    prevBtn->raise();
    nextBtn->raise();
    shootBtn->raise();
}

// 5️⃣ 底部文字
void CameraPage::layoutBottomText()
{
    // 1️⃣ 几何尺寸（小而精致）
    int textHeight = 35;
    int spacing = 4;

    bottomTextLabel->setGeometry(
        0,
        shootBtn->y() + shootBtn->height() + spacing,
        bottomContainer->width(),
        textHeight
        );

    // 2️⃣ 【关键】真正缩小字体
    QFont f;
    f.setBold(true);     // 仍然清晰
    bottomTextLabel->setFont(f);

    // 3️⃣ 对齐方式
    bottomTextLabel->setAlignment(Qt::AlignCenter);

    // 4️⃣ 防止被裁剪（可选但推荐）
    bottomTextLabel->setWordWrap(false);
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
    if (dateLabel) dateLabel->raise();
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
    const int NORMAL_SIZE = 95;   // 1280 宽时的标准大小
    const int MIN_SIZE    = 48;  // 最小
    const int MAX_SIZE    = 125;   // 最大

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

QString CameraPage::currentDateString() const
{
    QDate d = QDate::currentDate();
    return QString("%1年%2月%3日")
        .arg(d.year())
        .arg(d.month())
        .arg(d.day());
}

void CameraPage::showDateLabel()
{
    if (dateLabel)
        dateLabel->show();
}

void CameraPage::hideDateLabel()
{
    if (dateLabel)
        dateLabel->hide();
}
