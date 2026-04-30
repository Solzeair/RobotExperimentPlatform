QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtWidgetDesign
TEMPLATE = app

SOURCES += \
    main.cpp \
    QtWidgetDesign.cpp \
    CameraDlg.cpp \
    RobotDlg.cpp \
    DemarcateDlg.cpp \
    ColorDlg.cpp \
    MatchDlg_5vs5.cpp \
    Camera.cpp \
    Debug.cpp \
    Strategy.cpp \
    Strategy_5vs5_1.cpp \
    Strategy_5vs5_2.cpp \
    Strategy_sjxbs.cpp

HEADERS += \
    QtWidgetDesign.h \
    CameraDlg.h \
    RobotDlg.h \
    DemarcateDlg.h \
    ColorDlg.h \
    MatchDlg_5vs5.h \
    Camera.h \
    Debug.h \
    USB340HID61_DEF.h \
    Strategy.h \
    Strategy_5vs5_1.h \
    Strategy_5vs5_2.h \
    Strategy_sjxbs.h

CONFIG += c++11

LIBS += -L$$PWD/ -lUSB340HID61

# OpenCV配置
INCLUDEPATH += C:/opencv/build/include
LIBS += -LC:/opencv/build/x64/vc15/lib \
    -lopencv_core455 \
    -lopencv_imgproc455 \
    -lopencv_highgui455 \
    -lopencv_videoio455

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
default: target.path = $$[QT_INSTALL_EXAMPLES]/$${TARGET}
INSTALLS += target
