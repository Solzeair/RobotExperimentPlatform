// Goalie.h
// 守门员模块 - 守门员专用逻辑

#ifndef GOALIE_H
#define GOALIE_H

#include "StrategyCore.h"
#include "FieldGeometry.h"

class Goalie {
public:
    Goalie();
    ~Goalie();
    static void setGoaliePosition(int position);
    // ========== 守门员主逻辑 ==========

    // 守门员动作
    // robot: 守门员机器人位姿
    // ball: 球的信息
    // vel: 输出轮速
    // field: 场地几何信息
    // params: 控制参数
    static void goalieAction(const RobotPose& robot, const BallInfo& ball,
                             WheelVelocity& vel, const FieldGeometry& field,
                             const ControlParams& params);

    // ========== 辅助函数 ==========

    // 计算球的轨迹与球门线的交点
    // ball: 球的信息
    // x: 球门线x坐标
    // 返回: 交点y坐标
    static double getCrossPointWithGoalLine(const BallInfo& ball, double x);
};

#endif // GOALIE_H
