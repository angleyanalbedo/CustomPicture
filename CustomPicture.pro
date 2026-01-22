QT       += core gui multimedia multimediawidgets printsupport quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    camerapage.cpp \
    cameraview.cpp \
    countdown.cpp \
    cvcapture.cpp \
    imgproc.cpp \
    main.cpp \
    mainwindow.cpp \
    pageflipeffect.cpp \
    paypage.cpp \
    printpage.cpp

HEADERS += \
    camerapage.h \
    cameraview.h \
    countdown.h \
    cvcapture.h \
    imgproc.h \
    mainwindow.h \
    pageflipeffect.h \
    paypage.h \
    printpage.h

# SYSROOT = /opt/atk-dlrk356x-toolchain/aarch64-buildroot-linux-gnu/sysroot

# INCLUDEPATH += $$SYSROOT/usr/include/opencv4
# LIBS += -L$$SYSROOT/usr/lib \
#         -lopencv_core \
#         -lopencv_imgproc \
#         -lopencv_imgcodecs \
#         -lopencv_videoio \
#         -lopencv_highgui \
#         -lpthread -ldl -lz

# ---- 1. Windows 环境 ----
win32 {
    OPENCV_PATH = D:/source/opencv-4.11.0/build/install
    INCLUDEPATH += $$OPENCV_PATH/include
    LIBS += -L$$OPENCV_PATH/x64/mingw/lib
    LIBS += -lopencv_world4110

    # DLL 拷贝脚本保持不变...
    OPENCV_BIN_PATH = D:/source/opencv-4.11.0/build
    CONFIG(debug, debug|release) { DLL_FILE = libopencv_world4110.dll }
    else { DLL_FILE = libopencv_world4110.dll }
    COPY_SOURCE = $$replace($$quote($$OPENCV_BIN_PATH/$$DLL_FILE), /, \\)
    COPY_DEST = $$replace($$quote($$DESTDIR), /, \\)
    isEmpty(COPY_DEST): COPY_DEST = .
    QMAKE_POST_LINK += xcopy /d /y $$COPY_SOURCE $$COPY_DEST

    message("Windows build: using $$OPENCV_PATH")
}

# ---- 2. Linux 环境 (区分 ARM 开发板 vs PC 本机) ----
unix:!macx:!android {

    # 关键修改：检测目标架构是否为 ARM (aarch64 或 arm64)
    contains(QT_ARCH, aarch64)|contains(QT_ARCH, arm64) {
        # >>>> 这里是 RK3568 交叉编译配置 <<<<

        RK3568_SYSROOT = /opt/atk-dlrk356x-toolchain/aarch64-buildroot-linux-gnu/sysroot

        INCLUDEPATH += $$RK3568_SYSROOT/usr/include/opencv4
        LIBS += -L$$RK3568_SYSROOT/usr/lib

        LIBS += -lopencv_core \
                -lopencv_imgproc \
                -lopencv_imgcodecs \
                -lopencv_videoio \
                -lopencv_highgui

        LIBS += -lpthread -ldl -lz

        message("Cross-build (RK3568): Using Sysroot OpenCV")

    } else {
        # >>>> 这里是 Ubuntu PC 本机配置 (x86_64) <<<<
        # 只有在不是 ARM 的情况下，才加载 /usr/local

        INCLUDEPATH += /usr/local/include/opencv4
        LIBS += -L/usr/local/lib

        LIBS += -lopencv_core \
                -lopencv_imgproc \
                -lopencv_imgcodecs \
                -lopencv_videoio \
                -lopencv_highgui

        LIBS += -lpthread -ldl -lz

        message("Local build (x86_64): Using /usr/local OpenCV")
    }
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



RESOURCES += \
    resources.qrc
