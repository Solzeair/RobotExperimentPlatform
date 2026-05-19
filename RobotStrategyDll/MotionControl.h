// MotionControl.h
// 运动控制模块 - 提供机器人运动控制算法（PID控制、避障、轨迹跟踪等）

#ifndef MOTIONCONTROL_H
#define MOTIONCONTROL_H

#include "GeometryUtils.h"
#include "ControlParams.h"
#include "StrategyCore.h"

class MotionControl {
public:
    // ========== 基础运动控制 ==========

    // 移动到目标点（使用PID控制）
    // robot: 机器人当前位姿
    // target: 目标点坐标
    // speed: 期望移动速度
    // vel: 输出轮速
    // params: PID控制参数
    // 返回值: 1-已到达, 0-移动中
    static int moveToPoint(const RobotPose& robot, const Point& target,
                           double speed, WheelVelocity& vel, const ControlParams& params = ControlParams());

    // 原地转向到目标角度
    // currentAngle: 当前朝向角
    // targetAngle: 目标朝向角
    // vel: 输出轮速
    // params: PID控制参数
    // 返回值: 1-已到达, 0-转向中
    static int turnToAngle(double currentAngle, double targetAngle,
                           WheelVelocity& vel, const ControlParams& params = ControlParams());

    // 沿指定角度直线移动
    // robot: 机器人位姿
    // angle: 移动方向角
    // speed: 移动速度
    // vel: 输出轮速
    static int moveOnAngle(const RobotPose& robot, double angle,
                           double speed, WheelVelocity& vel);

    // PD控制移动到目标点（带减速区）
    // same_speed: 远端移动速度
    // end_speed: 近端减速速度
    static int toPositionPD(const RobotPose& robot, const Point& target,
                            double same_speed, double end_speed,
                            WheelVelocity& vel, const ControlParams& params = ControlParams());

    // ========== 避障算法 ==========

    // 基础避障（简单斥力法）
    // obstacles: 障碍物位置数组
    // count: 障碍物数量
    // 返回: 避开障碍物后的目标点
    static Point avoidObstacles(const RobotPose& robot, const Point& target,
                                const Point obstacles[], int count);

    // 平滑避障（人工势场法）
    // obstacleVelocities: 障碍物速度（可选，用于动态避障）
    // 返回: 避开障碍物后的目标点
    static Point smoothAvoidObstacles(const RobotPose& robot, const Point& target,
                                      const Point obstacles[], int count,
                                      const Point obstacleVelocities[] = nullptr);

    // 带避障的移动（组合函数）
    static int moveWithAvoidance(const RobotPose& robot, const Point& target,
                                 double speed, const Point obstacles[], int count,
                                 WheelVelocity& vel, const ControlParams& params = ControlParams());

    // 动态窗口避障（DWA算法）
    static Point dynamicWindowAvoid(const RobotPose& robot, const Point& target,
                                    const Point obstacles[], int count,
                                    const Point obstacleVelocities[]);

    // 预测性避障（考虑障碍物运动）
    static Point predictiveAvoid(const RobotPose& robot, const Point& target,
                                 const Point obstacles[], int count,
                                 const Point obstacleVelocities[]);

    // 多机器人避障（同时避开队友和对手）
    // teammates: 队友位姿数组
    // teammateCount: 队友数量
    // opponents: 对手位置数组
    // opponentCount: 对手数量
    static Point avoidAllRobots(const RobotPose& robot, const Point& target,
                                const RobotPose teammates[], int teammateCount,
                                const Point opponents[], int opponentCount);

    // ========== 特殊运动 ==========

    // 圆弧运动
    // radius: 转弯半径（正数向右转，负数向左转）
    // speed: 线速度
    static int circle(double radius, double speed, WheelVelocity& vel);

    // 计算从原点到目标点的转弯半径
    static double solveRadius(const Point& target);

    // ========== 轨迹跟踪 ==========

    // 沿轨迹点移动
    // trajectory: 轨迹点数组
    // trajCount: 轨迹点数量
    // currentIdx: 当前目标点索引（输入输出）
    // 返回值: 1-轨迹完成, 0-跟踪中
    static int followTrajectory(const RobotPose& robot, const Point trajectory[], int trajCount,
                                int& currentIdx, WheelVelocity& vel, const ControlParams& params);

    // ========== 辅助函数 ==========

    // 速度平滑（限制加速度）
    // left, right: 左右轮速度（输入输出）
    // maxAccel: 最大加速度
    static void smoothVelocity(double& left, double& right, double maxAccel);

private:
    // 限制数值范围
    static double limit(double value, double max);

    // 计算障碍物斥力
    static double calculateObstacleForce(const RobotPose& robot, const Point& obstacle,
                                         double safeDistance, double maxForce);

    // 计算目标点引力
    static Point calculateTargetAttraction(const RobotPose& robot, const Point& target,
                                           double maxAttraction);
};

#endif // MOTIONCONTROL_H
