# TestRobotStrategy.pro
QT += core widgets

TARGET = TestRobotStrategy
TEMPLATE = app

CONFIG += c++17 console
CONFIG -= app_bundle

DEFINES += UNICODE _UNICODE

SOURCES += \
    main.cpp \
    parameterdialog.cpp

HEADERS += \
    parameterdialog.h

# 不链接DLL库，只动态加载
# LIBS +=

# 设置工作目录
DESTDIR = $$OUT_PWD/debug
