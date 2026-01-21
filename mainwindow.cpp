#include "mainwindow.h"
#include "camerapage.h"
#include "paypage.h"

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
    connect(cameraPage, &CameraPage::photoFinished, this, [=](){
        stack->setCurrentWidget(payPage);
    });
}

MainWindow::~MainWindow() {}
