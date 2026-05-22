// MotionControl.cpp
// 运动控制模块实现

#include "MotionControl.h"
#include <cmath>
#include <algorithm>
#include <vector>

using namespace GeometryUtils;

// 辅助函数：将RobotPose转换为Point
static inline Point toPoint(const RobotPose& pose) {
    return Point(pose.x, pose.y);
}

// ========== 移动到目标点（PID控制）==========
// 算法：计算目标方向角，使用PID控制转向，同时根据距离调整速度
int MotionControl::moveToPoint(const RobotPose& robot, const Point& target,
                               double speed, WheelVelocity& vel, const ControlParams& params) {
    Point robotPos(robot.x, robot.y);
    double dist = pointToPointDistance(robotPos, target);

    // 已到达目标点（距离<0.5cm）
    if (dist < 0.5) {
        vel.left = vel.right = 0;
        return 1;
    }

    // 计算目标方向角
    double targetAngle = angleToPoint(robotPos, target);
    double angleError = normalizeAngle(targetAngle - robot.theta);

    // 距离减速：距离越近速度越慢
    double finalSpeed = speed;
    if (dist < 40) {
        finalSpeed = speed * (dist / 40.0);
        finalSpeed = std::max(finalSpeed, 20.0);  // 最低速度20
    }

    // PD控制计算转向补偿
    static double lastError = 0;
    double p = params.kp_pos * angleError;                           // 比例项
    double d = params.kd_pos * (angleError - lastError);             // 微分项
    double turnComp = p + d;
    turnComp = limit(turnComp, finalSpeed * 0.8);                    // 限制转向补偿
    lastError = angleError;

    // 计算轮速：左轮 = 速度 - 转向补偿，右轮 = 速度 + 转向补偿
    vel.left = finalSpeed - turnComp;
    vel.right = finalSpeed + turnComp;

    // 速度平滑
    smoothVelocity(vel.left, vel.right, 50.0);

    return 0;
}

// ========== 原地转向 ==========
int MotionControl::turnToAngle(double currentAngle, double targetAngle,
                               WheelVelocity& vel, const ControlParams& params) {
    double angleError = normalizeAngle(currentAngle - targetAngle);

    // 角度误差小于阈值，认为已到达
    if (fabs(angleError) < params.angle_error) {
        vel.left = vel.right = 0;
        return 1;
    }

    // P控制计算转向速度
    double turnSpeed = params.kp_angle * angleError;
    turnSpeed = limit(turnSpeed, params.max_speed);

    // 左轮负速度，右轮正速度 = 顺时针旋转
    vel.left = -turnSpeed;
    vel.right = turnSpeed;

    return 0;
}

// ========== 沿指定角度直线移动 ==========
int MotionControl::moveOnAngle(const RobotPose& robot, double angle,
                               double speed, WheelVelocity& vel) {
    double angleError = normalizeAngle(robot.theta - angle);
    // 角度误差越大，速度损失越大
    double factor = cos(angleError);
    if (factor < 0) factor = 0;

    vel.left = speed;
    vel.right = speed * factor;

    return 0;
}

// ========== PD控制移动到目标点（带减速区）==========
int MotionControl::toPositionPD(const RobotPose& robot, const Point& target,
                                double same_speed, double end_speed,
                                WheelVelocity& vel, const ControlParams& params) {
    Point robotPos(robot.x, robot.y);
    double dist = pointToPointDistance(robotPos, target);

    // 速度规划：远距离用same_speed，近距离线性减速
    double speed;
    if (dist > 40)
        speed = same_speed;
    else
        speed = (dist / 40) * same_speed;
    if (speed < end_speed) speed = end_speed;

    // 计算目标方向
    double targetAngle = angleToPoint(robotPos, target);
    double angleError = normalizeAngle(targetAngle - robot.theta);

    // 计算移动方向符号和旋转方向符号
    int move_sign = (angleError > 0) ? 1 : -1;
    int clock_sign = (angleError > 0) ? 1 : -1;

    // PD控制
    static double lastError = 0;
    double control = params.kp_pos * angleError +
                     params.kd_pos * (angleError - lastError);
    control = limit(control, 100);
    lastError = angleError;

    // 计算轮速
    vel.left = speed * move_sign + clock_sign * control;
    vel.right = speed * move_sign - clock_sign * control;

    return 0;
}

// ========== 基础避障（简单斥力法）==========
// 算法：检测最近的障碍物，如果距离过近则垂直避开
Point MotionControl::avoidObstacles(const RobotPose& robot, const Point& target,
                                    const Point obstacles[], int count) {
    Point robotPos(robot.x, robot.y);
    Point newTarget = target;
    double minDist = 1000;
    int nearest = -1;

    // 寻找最近的障碍物
    for (int i = 0; i < count; i++) {
        double dist = pointToPointDistance(robotPos, obstacles[i]);
        if (dist < 18 && dist < minDist) {
            minDist = dist;
            nearest = i;
        }
    }

    // 如果有障碍物太近，计算避开方向
    if (nearest >= 0) {
        double angle = angleToPoint(robotPos, obstacles[nearest]);
        double perpAngle = angle + M_PI/2;  // 垂直方向
        newTarget.x = obstacles[nearest].x + 18 * cos(perpAngle);
        newTarget.y = obstacles[nearest].y + 18 * sin(perpAngle);
    }

    return newTarget;
}

// ========== 平滑避障（人工势场法）==========
// 算法：目标点产生引力，障碍物产生斥力，合力方向为移动方向
Point MotionControl::smoothAvoidObstacles(const RobotPose& robot, const Point& target,
                                          const Point obstacles[], int count,
                                          const Point obstacleVelocities[]) {
    Point robotPos(robot.x, robot.y);
    if (count == 0) return target;

    double forceX = 0, forceY = 0;
    double safeDistance = 25.0;   // 安全距离
    double maxForce = 100.0;      // 最大斥力

    // 计算每个障碍物产生的斥力
    for (int i = 0; i < count; i++) {
        double dx = robotPos.x - obstacles[i].x;
        double dy = robotPos.y - obstacles[i].y;
        double dist = sqrt(dx*dx + dy*dy);

        if (dist < safeDistance && dist > 0.01) {
            // 斥力大小与距离成反比
            double strength = (safeDistance - dist) / safeDistance * maxForce;
            strength = std::min(strength, maxForce);

            // 如果障碍物在移动且朝向机器人，增加斥力
            if (obstacleVelocities != nullptr) {
                double velComp = (obstacleVelocities[i].x * dx + obstacleVelocities[i].y * dy) / dist;
                if (velComp > 0) strength *= (1.0 + velComp / 50.0);
            }

            // 斥力方向：远离障碍物
            forceX += dx / dist * strength;
            forceY += dy / dist * strength;
        }
    }

    // 计算目标点产生的引力
    double targetDist = pointToPointDistance(robotPos, target);
    double attraction = std::min(60.0, targetDist / 2.0);
    double targetAngle = angleToPoint(robotPos, target);
    double forceTargetX = cos(targetAngle) * attraction;
    double forceTargetY = sin(targetAngle) * attraction;

    // 合力 = 引力 + 斥力
    double totalForceX = forceTargetX + forceX;
    double totalForceY = forceTargetY + forceY;

    // 根据合力方向计算新目标点
    Point newTarget;
    double forceLen = sqrt(totalForceX*totalForceX + totalForceY*totalForceY);

    if (forceLen > 0.01) {
        double moveDist = std::min(35.0, targetDist / 1.5);
        newTarget.x = robotPos.x + totalForceX / forceLen * moveDist;
        newTarget.y = robotPos.y + totalForceY / forceLen * moveDist;
    } else {
        newTarget = target;
    }

    // 限制在场地范围内（留出边距）
    newTarget.x = std::max(8.0, std::min(212.0, newTarget.x));
    newTarget.y = std::max(8.0, std::min(172.0, newTarget.y));

    return newTarget;
}

// ========== 多机器人避障 ==========
Point MotionControl::avoidAllRobots(const RobotPose& robot, const Point& target,
                                    const RobotPose teammates[], int teammateCount,
                                    const Point opponents[], int opponentCount) {
    const int maxObstacles = 10;
    Point obstacles[maxObstacles];
    int obsCount = 0;
    Point robotPos(robot.x, robot.y);

    // 添加队友作为障碍物（距离<20cm）
    for (int i = 0; i < teammateCount && obsCount < maxObstacles; i++) {
        Point teammatePos(teammates[i].x, teammates[i].y);
        double dist = pointToPointDistance(robotPos, teammatePos);
        if (dist < 20 && dist > 0.1) {
            obstacles[obsCount++] = teammatePos;
        }
    }

    // 添加对手作为障碍物（距离<30cm）
    for (int i = 0; i < opponentCount && obsCount < maxObstacles; i++) {
        double dist = pointToPointDistance(robotPos, opponents[i]);
        if (dist < 30) {
            obstacles[obsCount++] = opponents[i];
        }
    }

    return smoothAvoidObstacles(robot, target, obstacles, obsCount, nullptr);
}

// ========== 圆弧运动 ==========
// 半径正数向右转，负数向左转
int MotionControl::circle(double radius, double speed, WheelVelocity& vel) {
    if (radius == 0) {
        // 半径为零：原地旋转
        vel.left = -speed;
        vel.right = speed;
    } else {
        // 圆弧运动：内外轮速度不同
        if (speed > 0) {
            vel.left = (radius - 4) / radius * speed;   // 内轮速度
            vel.right = (radius + 4) / radius * speed;  // 外轮速度
        } else {
            vel.left = (radius + 4) / radius * speed;
            vel.right = (radius - 4) / radius * speed;
        }
    }
    return 0;
}

// ========== 计算转弯半径 ==========
double MotionControl::solveRadius(const Point& target) {
    if (fabs(target.x) <= 0.0000001) return -1;
    if (fabs(target.y) <= 0.0000001) return fabs(target.x) / 2;

    Point origin = {0, 0};
    Point mid = {(origin.x + target.x)/2, (origin.y + target.y)/2};

    if (target.x == 0) return -1;
    double k = (target.y - origin.y) / (target.x - origin.x);
    double intercept = mid.y - k * mid.x;

    return fabs(intercept);
}

// ========== 轨迹跟踪 ==========
int MotionControl::followTrajectory(const RobotPose& robot, const Point trajectory[], int trajCount,
                                    int& currentIdx, WheelVelocity& vel, const ControlParams& params) {
    // 轨迹已走完
    if (currentIdx >= trajCount) {
        vel.left = vel.right = 0;
        return 1;
    }

    Point target = trajectory[currentIdx];
    Point robotPos(robot.x, robot.y);
    double dist = pointToPointDistance(robotPos, target);

    // 到达当前目标点，移动到下一个
    if (dist < 5.0) {
        currentIdx++;
        return followTrajectory(robot, trajectory, trajCount, currentIdx, vel, params);
    }

    return moveToPoint(robot, target, 70.0, vel, params);
}

// ========== 速度平滑（限制加速度）==========
void MotionControl::smoothVelocity(double& left, double& right, double maxAccel) {
    static double lastLeft = 0, lastRight = 0;

    double leftDiff = left - lastLeft;
    double rightDiff = right - lastRight;

    // 限制加速度
    if (fabs(leftDiff) > maxAccel) {
        left = lastLeft + (leftDiff > 0 ? maxAccel : -maxAccel);
    }
    if (fabs(rightDiff) > maxAccel) {
        right = lastRight + (rightDiff > 0 ? maxAccel : -maxAccel);
    }

    lastLeft = left;
    lastRight = right;
}

// ========== 限制数值范围 ==========
double MotionControl::limit(double value, double max) {
    if (value > max) return max;
    if (value < -max) return -max;
    return value;
}

// ========== 带避障的移动 ==========
int MotionControl::moveWithAvoidance(const RobotPose& robot, const Point& target,
                                     double speed, const Point obstacles[], int count,
                                     WheelVelocity& vel, const ControlParams& params) {
    Point avoidTarget = smoothAvoidObstacles(robot, target, obstacles, count, nullptr);
    return moveToPoint(robot, avoidTarget, speed, vel, params);
}
