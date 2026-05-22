// StrategyTypes.h
// 策略类型定义 - 确保与外部系统兼容

#ifndef STRATEGYTYPES_H
#define STRATEGYTYPES_H

#include <cstring>

// 基础几何结构（与外部系统保持一致的内存布局）
struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
};

struct RobotPose {
    double x, y, theta;
    double vx, vy, vtheta;
    RobotPose(double x = 0, double y = 0, double theta = 0,
        double vx = 0, double vy = 0, double vtheta = 0)
        : x(x), y(y), theta(theta), vx(vx), vy(vy), vtheta(vtheta) {
    }
};

struct BallInfo {
    Point pos;
    double vel_x, vel_y;
    double velocity;
    double angle;
    Point predictPos;
    BallInfo() : vel_x(0), vel_y(0), velocity(0), angle(0) {}
};

struct WheelVelocity {
    double left, right;
    WheelVelocity(double l = 0, double r = 0) : left(l), right(r) {}
};

// 静态断言确保结构体大小与外部系统一致
static_assert(sizeof(Point) == 16, "Point size mismatch");
#pragma message("sizeof(RobotPose) = " __STRINGIFY(sizeof(RobotPose)))
#pragma message("sizeof(BallInfo) = " __STRINGIFY(sizeof(BallInfo)))static_assert(sizeof(WheelVelocity) == 16, "WheelVelocity size mismatch");

#endif // STRATEGYTYPES_H
