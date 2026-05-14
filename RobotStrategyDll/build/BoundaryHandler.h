// BoundaryHandler.h
// 边界处理器 - 处理球靠近边界时的策略
#ifndef BOUNDARYHANDLER_H
#define BOUNDARYHANDLER_H

#include "StrategyCore.h"
#include "FieldGeometry.h"


class BoundaryHandler {
public:
    // ========== 主处理函数 ==========

    // 边界处理主函数
    // robot: 机器人位姿
    // ball: 球的信息
    // field: 场地几何信息
    // params: 控制参数
    // vel: 输出轮速
    static void handleBoundary(const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field, const ControlParams& params,
                               WheelVelocity& vel);
        // ========== 具体处理 ==========

    // 边线推球
    static void pushBallFromBoundary(const RobotPose& robot, const BallInfo& ball,
                                     const FieldGeometry& field, WheelVelocity& vel);

    // 角球处理
    static void handleCornerKick(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel);

    // 边界防守站位
    static Point getBoundaryDefensePosition(const RobotPose& robot, const BallInfo& ball,
                                            const FieldGeometry& field);

private:  // 获取边界区域编号
    static int getBoundaryArea(const Point& pos, const FieldGeometry& field);
};

#endif
