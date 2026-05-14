QT -= gui

TEMPLATE = lib
DEFINES += ROBOTSTRATEGYDLL_LIBRARY

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AreaDivider.cpp \
    BallPredictor.cpp \
    BoundaryHandler.cpp \
    FieldGeometry.cpp \
    Formation.cpp \
    GeometryUtils.cpp \
    Goalie.cpp \
    MotionControl.cpp \
    ParameterTuning.cpp \
    PassCoordinator.cpp \
    RoleAllocator.cpp \
    RoleTable.cpp \
    Shoot.cpp \
    StrategyFactory.cpp \
    UnifiedStrategy.cpp \
    robotstrategydll.cpp

HEADERS += \
    AreaDivider.h \
    BallPredictor.h \
    BoundaryHandler.h \
    ControlParams.h \
    FieldGeometry.h \
    Formation.h \
    GeometryUtils.h \
    Goalie.h \
    MotionControl.h \
    ParameterTuning.h \
    PassCoordinator.h \
    Role.h \
    RoleAllocator.h \
    RoleTable.h \
    Shoot.h \
    StrategyBase.h \
    StrategyCore.h \
    StrategyFactory.h \
    StrategyTypes.h \
    UnifiedStrategy.h \
    robotstrategydll.h \
    robotstrategydll_global.h

TRANSLATIONS += \
    RobotStrategyDll_zh_CN.ts

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
