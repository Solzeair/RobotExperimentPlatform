// StrategyCore.h
// 策略核心头文件 - 定义策略系统使用的基础数据结构

#ifndef STRATEGYCORE_H
#define STRATEGYCORE_H

#include "GeometryUtils.h"

// 前向声明，避免循环依赖
struct ControlParams;

/**
 * 球信息结构体
 * 包含球的当前位置、速度、运动方向等动态信息
 */
struct BallInfo {
    Point pos;          // 球的当前位置（世界坐标系）
    double vel_x;       // 球在x方向的速度分量（cm/s）
    double vel_y;       // 球在y方向的速度分量（cm/s）
    double velocity;    // 球的合速度大小（cm/s）
    double angle;       // 球的运动方向角（弧度）
    Point predictPos;   // 预测的未来位置

    // 构造函数，初始化所有成员为0
    BallInfo() : vel_x(0), vel_y(0), velocity(0), angle(0) {}
};

/**
 * 轮速结构体
 * 表示机器人左右轮的转速，用于控制机器人运动
 */
struct WheelVelocity {
    double left;        // 左轮速度（cm/s），正值向前
    double right;       // 右轮速度（cm/s），正值向前

    WheelVelocity(double l = 0, double r = 0) : left(l), right(r) {}
};

#endif
