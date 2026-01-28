#include "mainwindow.h"
#include "camerapage.h"
#include "paypage.h"
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    stack = new QStackedWidget(this);

    cameraPage = new CameraPage;
    payPage = new PayPage;

    stack->addWidget(cameraPage);
    stack->addWidget(payPage);

    setCentralWidget(stack);

    // 拍照完成 → 跳转付款
    connect(cameraPage,
            qOverload<const QString &>(&CameraPage::photoFinished),
            this,
            [this](const QString &path)
            {
                if (path.isEmpty()) {
                    QMessageBox::warning(this, "提示", "拍照失败，未生成文件");
                    return;
                }
                qDebug() << "收到照片路径:" << path;
                stack->setCurrentWidget(payPage);
            });

}

MainWindow::~MainWindow() {}
