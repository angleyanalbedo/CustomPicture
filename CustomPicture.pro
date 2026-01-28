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
# ---- Windows 环境配置 ----
    # 注意：请将下面的路径替换为你电脑上 OpenCV 的真实安装路径
    OPENCV_PATH = D:/source/opencv-4.11.0/build/install

    INCLUDEPATH += $$OPENCV_PATH/include

    # 根据编译器选择库文件夹 (VC15/VC16 对应 MSVC, x64 对应 64位)
    # 如果你使用的是 MinGW，路径可能是 $$OPENCV_PATH/x64/mingw/lib
    LIBS += -L$$OPENCV_PATH/x64/mingw/lib

    # Windows 下通常需要区分 Debug 和 Release 版本（带 d 的是 Debug）
    CONFIG(debug, debug|release) {
    # 由于我只编译了Release
        LIBS += -lopencv_world4110
    } else {
        LIBS += -lopencv_world4110
    }

    # 如果你的 OpenCV 不是合在一起的 opencv_world，则需要像 Linux 那样单独列出：
    # LIBS += -lopencv_core4xx ...

    message("Windows build: using $$OPENCV_PATH")

    # ---- 基础路径设置 (请确保路径正确) ----
    OPENCV_BIN_PATH = D:/source/opencv-4.11.0/build

    # ---- 自动复制 DLL 到运行目录 ----
    # 替换下面的 4xxx 为你的实际版本号，例如 4100
    CONFIG(debug, debug|release) {
        DLL_FILE = libopencv_world4110.dll
    } else {
        DLL_FILE = libopencv_world4110.dll
    }

    # 将路径中的 / 转换为 Windows 习惯的 \ 以执行命令
    COPY_SOURCE = $$replace($$quote($$OPENCV_BIN_PATH/$$DLL_FILE), /, \\)
    COPY_DEST = $$replace($$quote($$DESTDIR), /, \\)

    # 如果 DESTDIR 为空（默认情况），则复制到当前构建目录
    isEmpty(COPY_DEST): COPY_DEST = .

    # 执行复制命令
    QMAKE_POST_LINK += xcopy /d /y $$COPY_SOURCE $$COPY_DEST

    message("Post-link: Setting up DLL copy from $$OPENCV_BIN_PATH")
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
