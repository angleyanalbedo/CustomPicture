QT       += core gui multimedia multimediawidgets printsupport quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    backend/CameraManager.cpp \
    backend/ImageComposer.cpp \
    backend/TemplateManager.cpp \
    backend/backenddisk.cpp \
    backend/backendmem.cpp \
    bigheadpicturewindow.cpp \
    cmerawindows.cpp \
    editablepixmapitem.cpp \
    imageeditor.cpp \
    main.cpp \
    mainwindow.cpp \
    mainwindow2.cpp \
    postertemplate.cpp

HEADERS += \
    backend/CameraManager.h \
    backend/ImageComposer.h \
    backend/LiveImageProvider.h \
    backend/TemplateManager.h \
    backend/backenddisk.h \
    backend/backendmem.h \
    bigheadpicturewindow.h \
    cmerawindows.h \
    editablepixmapitem.h \
    imageeditor.h \
    mainwindow.h \
    mainwindow2.h \
    postertemplate.h

FORMS += \
    bigheadpicturewindow.ui \
    cmerawindows.ui \
    mainwindow.ui \
    mainwindow2.ui

# SYSROOT = /opt/atk-dlrk356x-toolchain/aarch64-buildroot-linux-gnu/sysroot

# INCLUDEPATH += $$SYSROOT/usr/include/opencv4
# LIBS += -L$$SYSROOT/usr/lib \
#         -lopencv_core \
#         -lopencv_imgproc \
#         -lopencv_imgcodecs \
#         -lopencv_videoio \
#         -lopencv_highgui \
#         -lpthread -ldl -lz



contains(QT_DEVICE_TARGET, "RK3568") {
    RK3568_SYSROOT = /opt/atk-dlrk356x-toolchain/aarch64-buildroot-linux-gnu/sysroot

    INCLUDEPATH += $$RK3568_SYSROOT/usr/include/opencv4
    LIBS += -L$$RK3568_SYSROOT/usr/lib

    LIBS += -lopencv_core \
            -lopencv_imgproc \
            -lopencv_imgcodecs \
            -lopencv_videoio \
            -lopencv_highgui

    LIBS += -lpthread -ldl -lz

    message("Cross-build: using RK3568 sysroot OpenCV")

}
else {
    # ----  1. 头文件  ----
    INCLUDEPATH += /usr/local/include/opencv4

    # ----  2. 库路径  ----
    LIBS += -L/usr/local/lib

    # ----  3. 常用 OpenCV 模块（缺啥补啥） ----
    LIBS += -lopencv_core \
            -lopencv_imgproc \
            -lopencv_imgcodecs \
            -lopencv_videoio \
            -lopencv_highgui

    # ----  4. 系统辅助库 ----
    LIBS += -lpthread -ldl -lz

    message("Local build: using /usr/local OpenCV")
}
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    qml/Main.qml

RESOURCES += \
    resource.qrc
