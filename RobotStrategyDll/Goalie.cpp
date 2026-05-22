// Goalie.cpp
// 守门员模块实现

#include "Goalie.h"
#include "MotionControl.h"
#include <cmath>
#include <algorithm>

using namespace GeometryUtils;

Goalie::Goalie() {}
Goalie::~Goalie() {}

// ========== 计算球的轨迹与球门线的交点 ==========
double Goalie::getCrossPointWithGoalLine(const BallInfo& ball, double x) {
    // 水平速度为零，返回当前位置
    if (fabs(ball.vel_x) < 0.01) {
        return ball.pos.y;
    }

    // 计算到达球门线的时间
    double t = (x - ball.pos.x) / ball.vel_x;
    return ball.pos.y + ball.vel_y * t;
}
static int s_goaliePosition = 1; // 0=左,1=中,2=右

void Goalie::setGoaliePosition(int position) {
    s_goaliePosition = position;
}

// ========== 守门员主逻辑 ==========
void Goalie::goalieAction(const RobotPose& robot, const BallInfo& ball,
                          WheelVelocity& vel, const FieldGeometry& field,
                          const ControlParams& params) {
    // 计算防守位置（球门线内8cm）
    double defendX = field.getOurGoalLineX();
    if (field.isOurGoalOnRight()) {
        defendX -= 8.0;   // 球门线内8cm
    } else {
        defendX += 8.0;
    }
    // 预测球的落点
    double targetY; 
    switch (s_goaliePosition) {
    case 0: targetY = 70; break;
    case 2: targetY = 110; break;
    default: targetY = 90; break;
    }
    // 只有当球在运动且距离足够远时才预测
    if (ball.vel_x != 0 && ball.pos.x > 50) {
        targetY = getCrossPointWithGoalLine(ball, defendX);
    }

    // 限制在球门范围内（球门高度40cm，中心90，范围65-115）
    targetY = std::max(65.0, std::min(115.0, targetY));

    // 如果球运动缓慢或不在门框内，防守球门中心
    if (ball.velocity < 5 || !(ball.vel_x > 0 && ball.pos.x > 50)) {
        targetY = 90.0;
    }

    Point target = {defendX, targetY};

    // 计算到目标的距离
    double dist = pointToPointDistance(robot, target);

    // 判断是否已经到位
    if (dist < 3) {
        // 已经到位，面向球
        double targetAngle = angleToPoint(robot, ball.pos);
        MotionControl::turnToAngle(robot.theta, targetAngle, vel, params);
    } else {
        // 移动到防守位置（守门员速度稍慢）
        double speed = params.max_speed * 0.6;
        MotionControl::moveToPoint(robot, target, speed, vel, params);
    }

    // ========== 出击拦截 ==========
    double ballDist = pointToPointDistance(robot, ball.pos);
    bool ballInFront = ball.pos.x > robot.x - 3;
    bool ballNear = ballDist < 10;
    bool ballMovingToGoal = ball.vel_x > 0 && ball.pos.x > 50;

    // 条件满足时出击拦截
    if (ballNear && ballInFront && ballMovingToGoal) {
        MotionControl::moveToPoint(robot, ball.pos, params.max_speed, vel, params);
    }

    // ========== 撞墙处理 ==========
    // 如果守门员撞到门柱，调整方向
    if (robot.y > 115 || robot.y < 65) {
        double theta = normalizeAngle(robot.theta);
        // 判断是否朝向门柱
        if (theta < M_PI / 4 || (theta > M_PI && theta - M_PI < M_PI / 4)) {
            MotionControl::turnToAngle(robot.theta, M_PI / 2, vel, params);
        } else if ((2 * M_PI - theta) < M_PI / 4 || (theta < M_PI && M_PI - theta < M_PI / 4)) {
            MotionControl::turnToAngle(robot.theta, M_PI / 2, vel, params);
        }
    }
}
