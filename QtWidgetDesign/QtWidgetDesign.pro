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
    MatchDlg_5vs5.cpp

HEADERS += \
    QtWidgetDesign.h \
    CameraDlg.h \
    RobotDlg.h \
    DemarcateDlg.h \
    ColorDlg.h \
    MatchDlg_5vs5.h

CONFIG += c++11

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
default: target.path = $$[QT_INSTALL_EXAMPLES]/$${TARGET}
INSTALLS += target
