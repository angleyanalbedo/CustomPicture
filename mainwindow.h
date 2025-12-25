#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QListWidgetItem>
#include <QMap>
#include <QPrinter>

#include "imageeditor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class EditablePixmapItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件操作
    void onActionNew();
    void onActionOpen();
    void onActionSave();
    void onActionExport();

    // 相机操作
    void onBtnCameraClicked();
    void onBtnCaptureClicked();
    void onBtnCloseCameraClicked();
    void onImageCaptured(int id, const QImage &preview);

    // 图片操作
    void onBtnLoadClicked();
    void onBtnCropClicked();
    void onBtnRotateClicked();
    void onBtnFilterClicked();
    void onBtnAdjustClicked();

    // 模板操作
    void onTemplateItemClicked(QListWidgetItem *item);
    void onBtnCustomTemplateClicked();

    // 输出操作
    void onBtnSaveClicked();
    void onBtnClearClicked();

    // 视图操作
    void onZoomSliderValueChanged(int value);
    void onActionZoomIn();
    void onActionZoomOut();
    void onActionFitView();

    // 编辑操作
    void onActionUndo();
    void onActionRedo();

    // 帮助
    void onActionAbout();

    // 图片选择
    void onPhotoSelected(EditablePixmapItem *item);
    void onPhotoDeleted(EditablePixmapItem *item);

    // 更新状态
    void updateStatusBar();
    void updateToolButtons();

    void exportToPdf(const QString &fileName);
    void setupCustomGridTemplate(int rows, int cols);

private:
    Ui::MainWindow *ui;

    // 图形场景
    QGraphicsScene *scene;
    QList<EditablePixmapItem*> photoItems;
    EditablePixmapItem *selectedItem;

    // 相机相关
    QCamera *camera;
    QCameraViewfinder *viewfinder;
    QCameraImageCapture *imageCapture;
    bool cameraActive;

    // 模板相关
    QMap<QString, QList<QRectF>> templates;
    QString currentTemplate;

    // 状态
    QPixmap currentPoster;
    qreal zoomFactor;

    // 初始化方法
    void initUI();
    void initCamera();
    void initTemplates();
    void initConnections();

    // 工具方法
    void setupDefaultTemplates();
    void applyTemplate(const QString &templateName);
    void addPhotoToScene(const QPixmap &pixmap);
    void removePhotoFromScene(EditablePixmapItem *item);
    void clearAllPhotos();
    void savePosterImage(const QString &fileName, const QString &format);
    void exportHighQuality(const QString &fileName, const QString &format);

    // 编辑方法
    void cropSelectedPhoto();
    void rotateSelectedPhoto(qreal angle);
    void applyFilterToSelected(const QString &filterName);

    // 工具方法
    QPixmap createPosterPreview();
    QString getSaveFileName(const QString &title, const QString &filter);


};

#endif // MAINWINDOW_H
