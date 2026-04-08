// BoundaryHandler.cpp
// 边界处理器实现

#include "BoundaryHandler.h"
#include "MotionControl.h"

using namespace GeometryUtils;

// ========== 边界处理主函数 ==========
void BoundaryHandler::handleBoundary(const RobotPose& robot, const BallInfo& ball,
                                     const FieldGeometry& field, const ControlParams& params,
                                     WheelVelocity& vel) {
    Point stdBall = field.transformToStandard(ball.pos);
    double fieldW = field.getFieldWidth();
    double fieldH = field.getFieldHeight();

    // 球在角落（距离角点小于15cm）
    if ((stdBall.x < 15 && stdBall.y < 15) ||
        (stdBall.x < 15 && stdBall.y > fieldH - 15) ||
        (stdBall.x > fieldW - 15 && stdBall.y < 15) ||
        (stdBall.x > fieldW - 15 && stdBall.y > fieldH - 15)) {
        handleCornerKick(robot, ball, field, vel);
        return;
    }

    // 球靠近边线（距离边界小于20cm）
    if (stdBall.x < 20 || stdBall.x > fieldW - 20 ||
        stdBall.y < 20 || stdBall.y > fieldH - 20) {
        pushBallFromBoundary(robot, ball, field, vel);
        return;
    }

    // 正常处理：移动到边界推球目标点
    Point target = field.getBoundaryPushTarget(ball.pos);
    MotionControl::moveToPoint(robot, target, 70.0, vel, params);
}

// ========== 边线推球 ==========
void BoundaryHandler::pushBallFromBoundary(const RobotPose& robot, const BallInfo& ball,
                                           const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getBoundaryPushTarget(ball.pos);
    double distToBall = pointToPointDistance(robot, ball.pos);

    // 如果已经靠近球，准备推球
    if (distToBall < 8) {
        double angleToTarget = angleToPoint(robot, target);
        double angleError = normalizeAngle(robot.theta - angleToTarget);

        // 方向未对准，先转向
        if (fabs(angleError) > 0.3) {
            MotionControl::turnToAngle(robot.theta, angleToTarget, vel, ControlParams());
        } else {
            // 方向对准，向前推球
            vel.left = 70;
            vel.right = 70;
        }
    } else {
        // 距离球较远，先移动到球的位置
        MotionControl::moveToPoint(robot, ball.pos, 65.0, vel, ControlParams());
    }
}

// ========== 角球处理 ==========
void BoundaryHandler::handleCornerKick(const RobotPose& robot, const BallInfo& ball,
                                       const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();  // 默认踢向对方球门

    // 如果球在我方半场，先推离危险区域
    if (field.isInOurHalf(ball.pos)) {
        target = field.getBoundaryPushTarget(ball.pos);
    }

    MotionControl::moveToPoint(robot, target, 75.0, vel, ControlParams());
}

// ========== 获取边界防守站位 ==========
Point BoundaryHandler::getBoundaryDefensePosition(const RobotPose& robot, const BallInfo& ball,
                                                  const FieldGeometry& field) {
    (void)robot;
    Point stdBall = field.transformToStandard(ball.pos);
    double fieldW = field.getFieldWidth();
    double fieldH = field.getFieldHeight();

    Point defensePos;

    // 根据球的位置选择防守站位
    if (stdBall.x < 30) {
        // 球在左侧，站位在左边界附近
        defensePos.x = 15;
        defensePos.y = std::max(20.0, std::min(fieldH - 20.0, stdBall.y));
    } else if (stdBall.y < 30) {
        // 球在下边界，站位在下边界附近
        defensePos.x = std::max(20.0, std::min(fieldW - 20.0, stdBall.x));
        defensePos.y = 15;
    } else if (stdBall.y > fieldH - 30) {
        // 球在上边界，站位在上边界附近
        defensePos.x = std::max(20.0, std::min(fieldW - 20.0, stdBall.x));
        defensePos.y = fieldH - 15;
    } else {
        // 正常情况，使用防守线站位
        defensePos = field.getDefenseLinePosition(ball.pos);
    }

    return field.transformFromStandard(defensePos);
}
