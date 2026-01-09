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
    editablepixmapitem.cpp \
    imageeditor.cpp \
    main.cpp \
    mainwindow.cpp \
    postertemplate.cpp

HEADERS += \
    backend/CameraManager.h \
    backend/ImageComposer.h \
    backend/TemplateManager.h \
    editablepixmapitem.h \
    imageeditor.h \
    mainwindow.h \
    postertemplate.h

FORMS += \
    mainwindow.ui

SYSROOT = /opt/atk-dlrk356x-toolchain/aarch64-buildroot-linux-gnu/sysroot

INCLUDEPATH += $$SYSROOT/usr/include/opencv4
LIBS += -L$$SYSROOT/usr/lib \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_imgcodecs \
        -lopencv_videoio \
        -lopencv_highgui \
        -lpthread -ldl -lz


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    qml/Main.qml

RESOURCES += \
    resource.qrc
