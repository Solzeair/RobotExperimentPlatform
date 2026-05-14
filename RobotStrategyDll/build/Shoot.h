// Shoot.h
// 射门模块 - 判断射门时机并执行射门动作

#ifndef SHOOT_H
#define SHOOT_H

#include "StrategyCore.h"
#include "FieldGeometry.h"
#include "ControlParams.h"

class Shoot {
public:
    Shoot();
    ~Shoot();

    // ========== 射门判断 ==========

    // 判断是否应该射门
    // robot: 机器人位姿
    // ball: 球的信息
    // field: 场地几何信息
    // 返回: true-应该射门
    static bool shouldShoot(const RobotPose& robot, const BallInfo& ball, const FieldGeometry& field);

    // 获取射门目标点（球门角落）
    static Point getShootTarget(const RobotPose& robot, const BallInfo& ball, const FieldGeometry& field);

    // ========== 射门执行 ==========

    // 弧线球射门（绕过防守队员）
    static void curveShoot(const RobotPose& robot, const BallInfo& ball, const Point& goal,
                           WheelVelocity& vel, const ControlParams& params);

    // 射门末端处理（加速射门）
    // 当机器人带球到合适位置时，加速踢球
    static void endProcess(const RobotPose& robot, const BallInfo& ball, const Point& goal,
                           WheelVelocity& vel, const ControlParams& params);

    static void penaltyShoot(const RobotPose& robot, const BallInfo& ball,
        int shootType, WheelVelocity& vel,
        const FieldGeometry& field, const ControlParams& params);
};

#endif
