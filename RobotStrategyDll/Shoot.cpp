// Shoot.cpp
// 射门模块实现

#include "Shoot.h"
#include "MotionControl.h"
#include <cmath>
#include <algorithm>

using namespace GeometryUtils;

Shoot::Shoot() {}
Shoot::~Shoot() {}

// ========== 判断是否应该射门 ==========
// 条件：距离球门近、球在车前、球距合适、射门角度好
bool Shoot::shouldShoot(const RobotPose& robot, const BallInfo& ball, const FieldGeometry& field) {
    double distToGoal = pointToPointDistance(robot, field.getOppGoalPos());
    bool nearGoal = distToGoal < 80;              // 距离球门小于80cm
    bool ballInFront = ball.pos.x > robot.x + 3;  // 球在机器人前方
    bool ballNear = pointToPointDistance(robot, ball.pos) < 15;  // 球距离小于15cm

    // 检查射门角度（偏差60度以内）
    double angleToGoal = angleToPoint(robot, field.getOppGoalPos());
    double angleError = fabs(normalizeAngle(robot.theta - angleToGoal));
    bool goodAngle = angleError < M_PI / 3;

    return nearGoal && ballInFront && (ballNear || goodAngle);
}

// ========== 获取射门目标点 ==========
// 策略：射向球门角落，增加进球概率
Point Shoot::getShootTarget(const RobotPose& robot, const BallInfo& ball, const FieldGeometry& field) {
    (void)robot;  // 未使用参数

    Point goal = field.getOppGoalPos();

    // 根据球的位置选择射门角度（射向球门角落）
    if (ball.pos.y > field.getOurGoalPos().y) {
        // 球在上半场，射向上角
        if (field.isOurGoalOnRight()) {
            goal.y = 115 - 8;  // 上角
        } else {
            goal.y = 65 + 8;   // 上角（镜像）
        }
    } else {
        // 球在下半场，射向下角
        if (field.isOurGoalOnRight()) {
            goal.y = 65 + 8;   // 下角
        } else {
            goal.y = 115 - 8;  // 下角（镜像）
        }
    }

    return goal;
}

// ========== 弧线球射门 ==========
// 算法：先移动到曲线中间点，形成弧线轨迹
void Shoot::curveShoot(const RobotPose& robot, const BallInfo& ball, const Point& goal,
                       WheelVelocity& vel, const ControlParams& params) {
    // 计算曲线路径的中间点（球和机器人的中点）
    Point midPoint = {(robot.x + ball.pos.x) / 2, (robot.y + ball.pos.y) / 2};

    // 计算球到球门的线
    double angleToGoal = angleToPoint(ball.pos, goal);
    double perpAngle = angleToGoal + M_PI / 2;  // 垂直方向

    // 偏移量（根据球的位置决定向左还是向右绕）
    double offset = 15.0;
    if (ball.pos.y > 90) {
        offset = -offset;
    }

    // 曲线点 = 中点 + 垂直偏移
    Point curvePoint;
    curvePoint.x = midPoint.x + offset * cos(perpAngle);
    curvePoint.y = midPoint.y + offset * sin(perpAngle);

    // 限制边界
    curvePoint.x = std::max(10.0, std::min(210.0, curvePoint.x));
    curvePoint.y = std::max(10.0, std::min(170.0, curvePoint.y));

    // 如果靠近曲线点，直接射门
    double distToCurve = pointToPointDistance(robot, curvePoint);
    if (distToCurve < 8) {
        MotionControl::moveToPoint(robot, goal, params.max_speed, vel, params);
    } else {
        MotionControl::moveToPoint(robot, curvePoint, params.max_speed * 0.7, vel, params);
    }
}

// ========== 射门末端处理 ==========
// 当机器人带球到位时，加速射门
void Shoot::endProcess(const RobotPose& robot, const BallInfo& ball, const Point& goal,
                       WheelVelocity& vel, const ControlParams& params) {
    static int processCount = 0;  // 射门计时器

    // 计算到球门的角度
    double angleToGoal = angleToPoint(robot, goal);
    double angleError = normalizeAngle(robot.theta - angleToGoal);

    // 球和车的距离
    double ballDist = pointToPointDistance(robot, ball.pos);
    double angleToBall = angleToPoint(robot, ball.pos);
    double distError = fabs(ballDist * sin(normalizeAngle(robot.theta - angleToBall)));

    // 射门条件：球在车前、距离合适、角度偏差小
    bool ballInFront = ball.pos.x > robot.x + 3;
    bool goodDist = ballDist < 12 && distError < 3.5;
    bool goodAngle = fabs(angleError) < M_PI / 4;

    if (ballInFront && goodDist && goodAngle) {
        processCount++;

        // 末端加速：逐渐增加到最大速度
        double speed = params.max_speed;
        if (processCount < 20) {
            speed = params.max_speed * (processCount / 20.0);
        }

        // 根据角度偏差调整轮速
        double turnComp = angleError * 15.0;
        vel.left = speed - turnComp;
        vel.right = speed + turnComp;
    } else {
        // 条件不满足，重置计时器，先移动到球的位置
        processCount = 0;
        MotionControl::moveToPoint(robot, ball.pos, params.max_speed * 0.8, vel, params);
    }
}
