#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "editablepixmapitem.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QCameraInfo>
#include <QImageWriter>
#include <QCloseEvent>
#include <QInputDialog>
#include <QSpinBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))
    , selectedItem(nullptr)
    , camera(nullptr)
    , viewfinder(nullptr)
    , imageCapture(nullptr)
    , cameraActive(false)
    , zoomFactor(1.0)
{
    ui->setupUi(this);

    initUI();
    initCamera();
    initTemplates();
    initConnections();
}

void MainWindow::initUI()
{
    // 设置场景
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);

    // 设置初始状态
    ui->cameraWidget->setVisible(false);
    ui->btnCapture->setEnabled(false);

    // 设置状态栏
    ui->statusBar->showMessage("就绪");

    // 添加默认模板
    setupDefaultTemplates();

    // 选中第一个模板
    if (ui->templateList->count() > 0) {
        ui->templateList->setCurrentRow(0);
    }
}

void MainWindow::initCamera()
{
#if NOT_RK3568
    // 检查可用摄像头
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (cameras.isEmpty()) {
        ui->btnCamera->setEnabled(false);
        ui->btnCamera->setText("无摄像头");
        return;
    }

    // 使用第一个摄像头
    camera = new QCamera(cameras.first(), this);
#else
    camera = new QCamera(chooseCamera(),this);
#endif

    viewfinder = new QCameraViewfinder(this);
    imageCapture = new QCameraImageCapture(camera, this);

    camera->setViewfinder(viewfinder);

    // 添加到布局
    ui->cameraContainerLayout->addWidget(viewfinder);
}

QCameraInfo MainWindow::chooseCamera(){
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

void MainWindow::initTemplates()
{
    // 清空模板列表
    ui->templateList->clear();

    // 添加模板项
    QStringList templateNames = {
        "四宫格海报",
        "九宫格海报",
        "横向拼接",
        "纵向拼接",
        "心形布局",
        "圆形布局"
    };

    foreach (const QString &name, templateNames) {
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setIcon(QIcon(":/icons/template.png"));
        ui->templateList->addItem(item);
    }
}

void MainWindow::initConnections()
{
    // 文件菜单
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::onActionNew);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onActionOpen);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onActionSave);
    connect(ui->actionExport, &QAction::triggered, this, &MainWindow::onActionExport);
    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);

    // 编辑菜单
    connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::onActionUndo);
    connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::onActionRedo);
    connect(ui->actionCrop, &QAction::triggered, this, &MainWindow::onBtnCropClicked);
    connect(ui->actionRotate, &QAction::triggered, this, &MainWindow::onBtnRotateClicked);
    connect(ui->actionFilter, &QAction::triggered, this, &MainWindow::onBtnFilterClicked);

    // 视图菜单
    connect(ui->actionZoomIn, &QAction::triggered, this, &MainWindow::onActionZoomIn);
    connect(ui->actionZoomOut, &QAction::triggered, this, &MainWindow::onActionZoomOut);
    connect(ui->actionFitView, &QAction::triggered, this, &MainWindow::onActionFitView);

    // 帮助菜单
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onActionAbout);

    // 按钮连接
    connect(ui->btnCamera, &QPushButton::clicked, this, &MainWindow::onBtnCameraClicked);
    connect(ui->btnCapture, &QPushButton::clicked, this, &MainWindow::onBtnCaptureClicked);
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::onBtnLoadClicked);
    connect(ui->btnCrop, &QPushButton::clicked, this, &MainWindow::onBtnCropClicked);
    connect(ui->btnRotate, &QPushButton::clicked, this, &MainWindow::onBtnRotateClicked);
    connect(ui->btnFilter, &QPushButton::clicked, this, &MainWindow::onBtnFilterClicked);
    connect(ui->btnAdjust, &QPushButton::clicked, this, &MainWindow::onBtnAdjustClicked);
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::onBtnSaveClicked);
    connect(ui->btnClear, &QPushButton::clicked, this, &MainWindow::onBtnClearClicked);
    connect(ui->btnExport, &QPushButton::clicked, this, &MainWindow::onBtnSaveClicked);
    connect(ui->btnCustomTemplate, &QPushButton::clicked, this, &MainWindow::onBtnCustomTemplateClicked);
    connect(ui->btnCloseCamera, &QPushButton::clicked, this, &MainWindow::onBtnCloseCameraClicked);

    // 列表连接
    connect(ui->templateList, &QListWidget::itemClicked,
            this, &MainWindow::onTemplateItemClicked);

    // 滑块连接
    connect(ui->zoomSlider, &QSlider::valueChanged,
            this, &MainWindow::onZoomSliderValueChanged);

    // 相机连接
    if (imageCapture) {
        connect(imageCapture, &QCameraImageCapture::imageCaptured,
                this, &MainWindow::onImageCaptured);
    }
}

void MainWindow::onBtnCameraClicked()
{
    if (!camera) return;

    if (!cameraActive) {
        camera->start();
        ui->cameraWidget->setVisible(true);
        ui->btnCapture->setEnabled(true);
        ui->btnCamera->setText("关闭摄像头");
        cameraActive = true;
    } else {
        camera->stop();
        ui->cameraWidget->setVisible(false);
        ui->btnCapture->setEnabled(false);
        ui->btnCamera->setText("打开摄像头");
        cameraActive = false;
    }
}

void MainWindow::onBtnCaptureClicked()
{
    if (imageCapture && cameraActive) {
        imageCapture->capture();
    }
}

void MainWindow::onImageCaptured(int id, const QImage &preview)
{
    Q_UNUSED(id);

    QPixmap pixmap = QPixmap::fromImage(preview);
    addPhotoToScene(pixmap);

    // 显示预览消息
    ui->statusBar->showMessage("拍照成功，已添加到海报", 2000);
}

void MainWindow::addPhotoToScene(const QPixmap &pixmap)
{
    EditablePixmapItem *item = new EditablePixmapItem(pixmap);
    item->setEditable(true);

    connect(item, &EditablePixmapItem::itemSelected,
            this, &MainWindow::onPhotoSelected);
    connect(item, &EditablePixmapItem::itemDeleted,
            this, &MainWindow::onPhotoDeleted);

    photoItems.append(item);
    scene->addItem(item);

    // 自动布局
    if (!currentTemplate.isEmpty()) {
        applyTemplate(currentTemplate);
    }

    updateToolButtons();
}

void MainWindow::onPhotoSelected(EditablePixmapItem *item)
{
    // 取消之前的选择
    if (selectedItem && selectedItem != item) {
        selectedItem->setSelected(false);
    }

    // 设置新的选择
    selectedItem = item;
    if (selectedItem) {
        selectedItem->setSelected(true);
        updateToolButtons();
    }
}

void MainWindow::updateToolButtons()
{
    bool hasSelection = (selectedItem != nullptr);
    bool hasPhotos = !photoItems.isEmpty();

    ui->btnCrop->setEnabled(hasSelection);
    ui->btnRotate->setEnabled(hasSelection);
    ui->btnFilter->setEnabled(hasSelection);
    ui->btnAdjust->setEnabled(hasSelection);

    ui->actionCrop->setEnabled(hasSelection);
    ui->actionRotate->setEnabled(hasSelection);
    ui->actionFilter->setEnabled(hasSelection);

    ui->btnSave->setEnabled(hasPhotos);
    ui->btnExport->setEnabled(hasPhotos);
    ui->btnClear->setEnabled(hasPhotos);
}


void MainWindow::setupDefaultTemplates()
{
    // 四宫格
    QList<QRectF> grid4;
    grid4 << QRectF(0.1, 0.1, 0.4, 0.4)   // 左上
          << QRectF(0.5, 0.1, 0.4, 0.4)   // 右上
          << QRectF(0.1, 0.5, 0.4, 0.4)   // 左下
          << QRectF(0.5, 0.5, 0.4, 0.4);  // 右下
    templates["四宫格海报"] = grid4;

    // 九宫格
    QList<QRectF> grid9;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            grid9 << QRectF(col * 0.33, row * 0.33, 0.3, 0.3);
        }
    }
    templates["九宫格海报"] = grid9;

    // 横向拼接
    QList<QRectF> horizontal;
    horizontal << QRectF(0.0, 0.0, 0.5, 1.0)
               << QRectF(0.5, 0.0, 0.5, 1.0);
    templates["横向拼接"] = horizontal;

    // 纵向拼接
    QList<QRectF> vertical;
    vertical << QRectF(0.0, 0.0, 1.0, 0.5)
             << QRectF(0.0, 0.5, 1.0, 0.5);
    templates["纵向拼接"] = vertical;
}

void MainWindow::applyTemplate(const QString &templateName)
{
    if (!templates.contains(templateName)) {
        return;
    }

    currentTemplate = templateName;
    QList<QRectF> positions = templates[templateName];

    QRectF sceneRect = ui->graphicsView->sceneRect();
    if (sceneRect.isNull()) {
        sceneRect = QRectF(0, 0, 800, 600);
    }

    // 应用布局
    for (int i = 0; i < qMin(photoItems.size(), positions.size()); ++i) {
        EditablePixmapItem *item = photoItems[i];
        QRectF pos = positions[i];

        // 转换为实际坐标
        qreal x = sceneRect.width() * pos.x();
        qreal y = sceneRect.height() * pos.y();
        qreal width = sceneRect.width() * pos.width();
        qreal height = sceneRect.height() * pos.height();

        // 设置位置和大小
        item->setPos(x, y);

        // 调整图片大小以适应位置
        QPixmap pixmap = item->pixmap();
        QPixmap scaled = pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        item->setPixmap(scaled);
    }

    scene->update();
    ui->statusBar->showMessage(QString("已应用模板: %1").arg(templateName), 2000);
}

void MainWindow::onTemplateItemClicked(QListWidgetItem *item)
{
    if (item) {
        QString templateName = item->text();
        applyTemplate(templateName);
    }
}

void MainWindow::onBtnLoadClicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          "选择图片",
                                                          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                          "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");

    if (fileNames.isEmpty()) {
        return;
    }

    foreach (QString fileName, fileNames) {
        QPixmap pixmap(fileName);
        if (!pixmap.isNull()) {
            addPhotoToScene(pixmap);
        } else {
            QMessageBox::warning(this, "错误", QString("无法加载图片: %1").arg(fileName));
        }
    }

    ui->statusBar->showMessage(QString("已加载 %1 张图片").arg(fileNames.size()), 2000);
}

void MainWindow::onBtnCropClicked()
{
    if (!selectedItem) {
        return;
    }

    // 这里可以打开一个裁剪对话框，为了简单，我们直接裁剪一半
    QPixmap pixmap = selectedItem->pixmap();
    QRect cropRect = QRect(0, 0, pixmap.width() / 2, pixmap.height() / 2);
    selectedItem->crop(cropRect);

    scene->update();
    ui->statusBar->showMessage("已裁剪图片", 2000);
}

void MainWindow::onBtnRotateClicked()
{
    if (!selectedItem) {
        return;
    }

    selectedItem->rotate(90);
    scene->update();
    ui->statusBar->showMessage("已旋转图片", 2000);
}

void MainWindow::onBtnFilterClicked()
{
    if (!selectedItem) {
        return;
    }

    // 这里可以打开滤镜选择对话框，为了简单，我们直接应用一个滤镜
    QPixmap pixmap = selectedItem->pixmap();
    ImageEditor editor;
    QPixmap filtered = editor.applyFilter(pixmap, FILTER_SEPIA);
    selectedItem->setPixmap(filtered);

    scene->update();
    ui->statusBar->showMessage("已应用滤镜", 2000);
}

void MainWindow::onBtnAdjustClicked()
{
    // 打开调整对话框（亮度、对比度等）
    // 这里暂时不实现
    QMessageBox::information(this, "提示", "调整功能暂未实现");
}

void MainWindow::onBtnCustomTemplateClicked()
{
    bool ok;
    int rows = QInputDialog::getInt(this, "自定义模板", "行数:", 2, 1, 10, 1, &ok);
    if (!ok) return;

    int cols = QInputDialog::getInt(this, "自定义模板", "列数:", 2, 1, 10, 1, &ok);
    if (!ok) return;

    // 创建自定义模板
    QList<QRectF> customTemplate;
    qreal cellWidth = 1.0 / cols;
    qreal cellHeight = 1.0 / rows;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            customTemplate << QRectF(col * cellWidth, row * cellHeight, cellWidth, cellHeight);
        }
    }

    QString templateName = QString("自定义 %1x%2").arg(rows).arg(cols);
    templates[templateName] = customTemplate;

    // 添加到列表
    QListWidgetItem *item = new QListWidgetItem(templateName);
    item->setIcon(QIcon(":/icons/custom.png"));
    ui->templateList->addItem(item);

    ui->statusBar->showMessage("自定义模板已创建", 2000);
}

void MainWindow::onBtnSaveClicked()
{
    if (photoItems.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有图片可以保存");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "保存海报",
                                                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/poster.png",
                                                    "PNG图片 (*.png);;JPEG图片 (*.jpg *.jpeg);;所有文件 (*.*)");

    if (fileName.isEmpty()) {
        return;
    }

    // 确定格式
    QString format = "PNG";
    if (fileName.endsWith(".jpg", Qt::CaseInsensitive) || fileName.endsWith(".jpeg", Qt::CaseInsensitive)) {
        format = "JPG";
    }

    savePosterImage(fileName, format);
}

void MainWindow::savePosterImage(const QString &fileName, const QString &format)
{
    // 创建海报图像
    QPixmap poster = createPosterPreview();

    if (poster.save(fileName, format.toLatin1(), 90)) {
        ui->statusBar->showMessage(QString("海报已保存到: %1").arg(fileName), 3000);
    } else {
        QMessageBox::warning(this, "错误", "保存失败");
    }
}

QPixmap MainWindow::createPosterPreview()
{
    // 获取场景的边界
    QRectF itemsRect = scene->itemsBoundingRect();

    // 创建图像
    QPixmap poster(itemsRect.size().toSize());
    poster.fill(Qt::white);

    QPainter painter(&poster);
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter, QRectF(), itemsRect);

    return poster;
}

void MainWindow::onBtnClearClicked()
{
    clearAllPhotos();
}

void MainWindow::clearAllPhotos()
{
    foreach (EditablePixmapItem *item, photoItems) {
        scene->removeItem(item);
        delete item;
    }
    photoItems.clear();
    selectedItem = nullptr;

    updateToolButtons();
    scene->update();

    ui->statusBar->showMessage("已清空所有图片", 2000);
}

void MainWindow::onPhotoDeleted(EditablePixmapItem *item)
{
    if (item) {
        scene->removeItem(item);
        photoItems.removeOne(item);

        if (selectedItem == item) {
            selectedItem = nullptr;
        }

        delete item;
        updateToolButtons();
        scene->update();
    }
}

void MainWindow::onZoomSliderValueChanged(int value)
{
    zoomFactor = value / 100.0;
    QTransform transform;
    transform.scale(zoomFactor, zoomFactor);
    ui->graphicsView->setTransform(transform);

    ui->statusBar->showMessage(QString("缩放: %1%").arg(value), 1000);
}

void MainWindow::onActionZoomIn()
{
    int value = ui->zoomSlider->value() + 10;
    if (value > ui->zoomSlider->maximum()) {
        value = ui->zoomSlider->maximum();
    }
    ui->zoomSlider->setValue(value);
}

void MainWindow::onActionZoomOut()
{
    int value = ui->zoomSlider->value() - 10;
    if (value < ui->zoomSlider->minimum()) {
        value = ui->zoomSlider->minimum();
    }
    ui->zoomSlider->setValue(value);
}

void MainWindow::onActionFitView()
{
    // 调整缩放以使所有项目适合视图
    QRectF itemsRect = scene->itemsBoundingRect();
    if (itemsRect.isNull()) {
        return;
    }

    QRectF viewRect = ui->graphicsView->viewport()->rect();
    qreal scale = qMin(viewRect.width() / itemsRect.width(),
                       viewRect.height() / itemsRect.height());

    int sliderValue = qBound(ui->zoomSlider->minimum(),
                             static_cast<int>(scale * 100),
                             ui->zoomSlider->maximum());
    ui->zoomSlider->setValue(sliderValue);
}

void MainWindow::onActionUndo()
{
    // 实现撤销功能
    ui->statusBar->showMessage("撤销功能暂未实现");
}

void MainWindow::onActionRedo()
{
    // 实现重做功能
    ui->statusBar->showMessage("重做功能暂未实现");
}

void MainWindow::onActionAbout()
{
    QMessageBox::about(this, "关于照片海报拼接软件",
                       "<h3>照片海报拼接软件</h3>"
                       "<p>版本 1.0</p>"
                       "<p>一个使用Qt开发的照片拼接海报工具。</p>"
                       "<p>支持拍照、导入图片、多种模板、图片编辑和导出功能。</p>");
}

void MainWindow::onBtnCloseCameraClicked()
{
    if (camera && cameraActive) {
        camera->stop();
        ui->cameraWidget->setVisible(false);
        ui->btnCapture->setEnabled(false);
        ui->btnCamera->setText("打开摄像头");
        cameraActive = false;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 停止摄像头
    if (camera && cameraActive) {
        camera->stop();
    }

    event->accept();
}

void MainWindow::onActionNew()
{
    // 创建新的海报项目
    if (!photoItems.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "新建海报", "当前海报将丢失，是否继续？",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) {
            return;
        }
    }

    clearAllPhotos();
    scene->clear();
    photoItems.clear();
    selectedItem = nullptr;

    // 重置模板选择
    if (ui->templateList->count() > 0) {
        ui->templateList->setCurrentRow(0);
    }

    currentTemplate.clear();

    // 更新状态
    updateToolButtons();
    ui->statusBar->showMessage("已创建新的海报", 2000);
}

void MainWindow::onActionOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("打开海报项目"), "",
                                                    tr("海报项目 (*.poster);;所有文件 (*)"));

    if (!fileName.isEmpty()) {
        // 这里可以添加加载项目的逻辑
        // 暂时先清空当前内容
        onActionNew();

        QMessageBox::information(this, "提示",
                                 "项目文件加载功能待实现\n文件: " + fileName);
    }
}

void MainWindow::onActionSave()
{
    if (photoItems.isEmpty()) {
        QMessageBox::warning(this, "保存失败", "当前没有图片可以保存");
        return;
    }

    QString defaultName = QString("海报_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("保存海报"), defaultName,
                                                    tr("PNG图片 (*.png);;JPEG图片 (*.jpg *.jpeg);;BMP图片 (*.bmp)"));

    if (!fileName.isEmpty()) {
        // 获取文件格式
        QString format = "PNG";
        if (fileName.endsWith(".jpg", Qt::CaseInsensitive) ||
            fileName.endsWith(".jpeg", Qt::CaseInsensitive)) {
            format = "JPG";
        } else if (fileName.endsWith(".bmp", Qt::CaseInsensitive)) {
            format = "BMP";
        }

        savePosterImage(fileName, format);
        ui->statusBar->showMessage(QString("海报已保存: %1").arg(fileName), 3000);
    }
}

void MainWindow::onActionExport()
{
    if (photoItems.isEmpty()) {
        QMessageBox::warning(this, "导出失败", "当前没有图片可以导出");
        return;
    }

    QString defaultName = QString("高清海报_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("导出高清海报"), defaultName,
                                                    tr("PNG图片 (*.png);;JPEG图片 (*.jpg *.jpeg);;PDF文档 (*.pdf)"));

    if (!fileName.isEmpty()) {
        // 获取文件格式
        QString format = "PNG";
        if (fileName.endsWith(".jpg", Qt::CaseInsensitive) ||
            fileName.endsWith(".jpeg", Qt::CaseInsensitive)) {
            format = "JPG";
        } else if (fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
            format = "PDF";
        }

        // 如果是PDF，使用特定方法
        if (format == "PDF") {
            exportToPdf(fileName);
        } else {
            exportHighQuality(fileName, format);
        }

        ui->statusBar->showMessage(QString("高清海报已导出: %1").arg(fileName), 3000);
    }
}

// 更新状态栏
void MainWindow::updateStatusBar()
{
    QString status;

    if (selectedItem) {
        QPointF pos = selectedItem->pos();
        status = QString("选中项目 | 位置: (%1, %2) | 缩放: %3%")
                     .arg(pos.x(), 0, 'f', 1)
                     .arg(pos.y(), 0, 'f', 1)
                     .arg(zoomFactor * 100, 0, 'f', 1);
    } else {
        status = QString("就绪 | 图片数量: %1 | 缩放: %2%")
                     .arg(photoItems.size())
                     .arg(zoomFactor * 100, 0, 'f', 1);
    }

    ui->statusBar->showMessage(status);
}



// 实现 exportHighQuality 方法
void MainWindow::exportHighQuality(const QString &fileName, const QString &format)
{
    // 高清导出 - 使用更大的分辨率
    QRectF totalRect = scene->itemsBoundingRect();
    QSize originalSize = totalRect.size().toSize();

    // 高清尺寸（2倍）
    QSize hdSize = originalSize * 2;
    QImage image(hdSize, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 缩放渲染
    painter.scale(2.0, 2.0);
    scene->render(&painter);
    painter.end();

    // 保存图像
    if (!image.save(fileName, format.toLatin1(), 100)) { // 100%质量
        QMessageBox::critical(this, "导出失败", "无法导出高清图像");
    }
}

// 实现 exportToPdf 方法
void MainWindow::exportToPdf(const QString &fileName)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setFullPage(true);

    QPainter painter(&printer);
    painter.setRenderHint(QPainter::Antialiasing);

    // 计算缩放比例以适应页面
    QRectF sceneRect = scene->itemsBoundingRect();
    qreal xscale = printer.pageRect().width() / sceneRect.width();
    qreal yscale = printer.pageRect().height() / sceneRect.height();
    qreal scale = qMin(xscale, yscale);

    painter.scale(scale, scale);
    scene->render(&painter);
    painter.end();
}


// 实现 cropSelectedPhoto 方法
void MainWindow::cropSelectedPhoto()
{
    if (!selectedItem) {
        QMessageBox::warning(this, "裁剪失败", "请先选择一张图片");
        return;
    }
     QRectF itemRect = selectedItem->boundingRect();
    bool ok = false;
    int x = QInputDialog::getInt(this, "裁剪图片", "X:", 0, 0,
                                 int(itemRect.width()),  1, &ok);
    if (!ok) return;
    int y = QInputDialog::getInt(this, "裁剪图片", "Y:", 0, 0,
                                 int(itemRect.height()), 1, &ok);
    if (!ok) return;
    int w = QInputDialog::getInt(this, "裁剪图片", "宽度:", int(itemRect.width()),
                                 1, int(itemRect.width()), 1, &ok);
    if (!ok) return;
    int h = QInputDialog::getInt(this, "裁剪图片", "高度:", int(itemRect.height()),
                                 1, int(itemRect.height()), 1, &ok);
    if (!ok) return;

    QRect cropRect(x, y, w, h);
    if (!cropRect.isValid()) return;

    if (ok && cropRect.isValid()) {
        QPixmap original = selectedItem->pixmap();
        QPixmap cropped = ImageEditor::cropImage(original, cropRect);
        selectedItem->setPixmap(cropped);
        ui->statusBar->showMessage("图片已裁剪", 2000);
    }
}

// 实现 rotateSelectedPhoto 方法
void MainWindow::rotateSelectedPhoto(qreal angle)
{
    if (!selectedItem) return;

    QPixmap original = selectedItem->pixmap();
    QPixmap rotated = ImageEditor::rotateImage(original, angle);
    selectedItem->setPixmap(rotated);
}

// 实现 applyFilterToSelected 方法
void MainWindow::applyFilterToSelected(const QString &filterName)
{
    if (!selectedItem) return;

    // 这里可以根据filterName应用不同的滤镜
    // 简化版：应用灰度滤镜

    QPixmap original = selectedItem->pixmap();
    QPixmap filtered = ImageEditor::applyFilter(original, FILTER_GRAYSCALE);
    selectedItem->setPixmap(filtered);

    ui->statusBar->showMessage(QString("已应用滤镜: %1").arg(filterName), 2000);
}

void MainWindow::setupCustomGridTemplate(int rows, int cols)
{
    if (rows <= 0 || cols <= 0) return;

    // 计算场景中的有效区域（减去边距）
    QRectF sceneRect = scene->sceneRect();
    qreal margin = 20;
    qreal usableWidth = sceneRect.width() - 2 * margin;
    qreal usableHeight = sceneRect.height() - 2 * margin;

    // 计算每个格子的尺寸
    qreal cellWidth = usableWidth / cols;
    qreal cellHeight = usableHeight / rows;

    // 重新排列现有图片
    for (int i = 0; i < photoItems.size(); ++i) {
        EditablePixmapItem *item = photoItems[i];

        // 计算行和列
        int row = i / cols;
        int col = i % cols;

        // 如果超过格子数量，停止排列
        if (row >= rows) break;

        // 计算位置
        qreal x = margin + col * cellWidth + cellWidth / 2;
        qreal y = margin + row * cellHeight + cellHeight / 2;

        // 设置位置（将中心点对齐到格子中心）
        item->setPos(x - item->boundingRect().width() / 2,
                     y - item->boundingRect().height() / 2);

        // 缩放图片以适应格子
        QRectF itemRect = item->boundingRect();
        qreal scaleX = cellWidth * 0.8 / itemRect.width();  // 80%的格子宽度
        qreal scaleY = cellHeight * 0.8 / itemRect.height(); // 80%的格子高度
        qreal scale = qMin(scaleX, scaleY);

        if (scale < 1.0) {
            item->setScale(scale);
        }
    }

    // 更新UI
    ui->templateList->addItem(QString("自定义%1x%2").arg(rows).arg(cols));
    ui->templateList->setCurrentRow(ui->templateList->count() - 1);
}

MainWindow::~MainWindow()
{
    delete ui;
}
