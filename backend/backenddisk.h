// Backend_disk.h
#pragma once
#include <QObject>
#include <QTimer>
#include "CameraManager.h"


class BackendDisk : public QObject
{
    Q_OBJECT
public:
    explicit BackendDisk(QObject *parent = nullptr);

    Q_INVOKABLE void capture();          // 拍照按钮调用

signals:
    void liveChanged();                  // 通知 QML 刷新

private slots:
    void composeOneFrame();              // 定时合成

private:
    CameraManager cam;
    QTimer timer;
};
