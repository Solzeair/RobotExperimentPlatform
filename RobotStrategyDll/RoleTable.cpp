// RoleTable.cpp
#define TO_POINT(pose) Point((pose).x, (pose).y)
#include "RoleTable.h"
#include "MotionControl.h"
#include "Goalie.h"
#include "Shoot.h"
#include <cmath>
#include <algorithm>
using namespace GeometryUtils;

// ==================== 基础角色实现 ====================

void RoleTable::roleStop(const RobotPose& robot, WheelVelocity& vel) {
    (void)robot;
    vel.left = 0;
    vel.right = 0;
}

void RoleTable::roleGoToPoint(const RobotPose& robot, const Point& target,
                              double speed, WheelVelocity& vel, const ControlParams& params) {
    MotionControl::moveToPoint(robot, target, speed, vel, params);
}

// ==================== 边界角色实现 ====================

void RoleTable::roleLeftBound(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel) {
    (void)field;
    Point target(8, ball.pos.y);
    target.y = std::max(20.0, std::min(field.getFieldHeight() - 20.0, target.y));

    double dist = pointToPointDistance(TO_POINT(robot), target);
    if (dist < 5) {
        double targetAngle = angleToPoint(robot, ball.pos);
        MotionControl::turnToAngle(robot.theta, targetAngle, vel, ControlParams());
    } else {
        MotionControl::moveToPoint(robot, target, 60.0, vel, ControlParams());
    }
}

void RoleTable::roleTopBound(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel) {
    (void)field;
    Point target(ball.pos.x, field.getFieldHeight() - 8);
    target.x = std::max(20.0, std::min(field.getFieldWidth() - 20.0, target.x));

    double dist = pointToPointDistance(TO_POINT(robot), target);
    if (dist < 5) {
        double targetAngle = angleToPoint(robot, ball.pos);
        MotionControl::turnToAngle(robot.theta, targetAngle, vel, ControlParams());
    } else {
        MotionControl::moveToPoint(robot, target, 60.0, vel, ControlParams());
    }
}

void RoleTable::roleRightBound(const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field, WheelVelocity& vel) {
    (void)field;
    Point target(field.getFieldWidth() - 8, ball.pos.y);
    target.y = std::max(20.0, std::min(field.getFieldHeight() - 20.0, target.y));

    double dist = pointToPointDistance(TO_POINT(robot), target);
    if (dist < 5) {
        double targetAngle = angleToPoint(robot, ball.pos);
        MotionControl::turnToAngle(robot.theta, targetAngle, vel, ControlParams());
    } else {
        MotionControl::moveToPoint(robot, target, 60.0, vel, ControlParams());
    }
}

void RoleTable::roleBottomBound(const RobotPose& robot, const BallInfo& ball,
                                const FieldGeometry& field, WheelVelocity& vel) {
    (void)field;
    Point target(ball.pos.x, 8);
    target.x = std::max(20.0, std::min(field.getFieldWidth() - 20.0, target.x));

    double dist = pointToPointDistance(TO_POINT(robot), target);
    if (dist < 5) {
        double targetAngle = angleToPoint(robot, ball.pos);
        MotionControl::turnToAngle(robot.theta, targetAngle, vel, ControlParams());
    } else {
        MotionControl::moveToPoint(robot, target, 60.0, vel, ControlParams());
    }
}

void RoleTable::roleBoundPush(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getBoundaryPushTarget(ball.pos);
    double distToBall = pointToPointDistance(robot, ball.pos);

    if (distToBall < 8) {
        double angleToTarget = angleToPoint(TO_POINT(robot), target);
        double angleError = normalizeAngle(robot.theta - angleToTarget);

        if (fabs(angleError) > 0.2) {
            MotionControl::turnToAngle(robot.theta, angleToTarget, vel, ControlParams());
        } else {
            vel.left = 70;
            vel.right = 70;
        }
    } else {
        MotionControl::moveToPoint(robot, ball.pos, 65.0, vel, ControlParams());
    }
}

void RoleTable::rolePushOut(const RobotPose& robot, const BallInfo& ball,
                            const FieldGeometry& field, WheelVelocity& vel) {
    (void)ball;
    Point target = field.getOppGoalPos();
    MotionControl::moveToPoint(robot, target, 70.0, vel, ControlParams());
}

void RoleTable::roleCornerDefense(const RobotPose& robot, const BallInfo& ball,
                                  const FieldGeometry& field, WheelVelocity& vel) {
    Point stdBall = field.transformToStandard(ball.pos);
    double fieldW = field.getFieldWidth();
    double fieldH = field.getFieldHeight();

    Point defensePos;
    if (stdBall.x < 20 && stdBall.y < 20) {
        defensePos = Point(15, 15);
    } else if (stdBall.x < 20 && stdBall.y > fieldH - 20) {
        defensePos = Point(15, fieldH - 15);
    } else if (stdBall.x > fieldW - 20 && stdBall.y < 20) {
        defensePos = Point(fieldW - 15, 15);
    } else {
        defensePos = Point(fieldW - 15, fieldH - 15);
    }

    defensePos = field.transformFromStandard(defensePos);
    MotionControl::moveToPoint(robot, defensePos, 65.0, vel, ControlParams());
}

void RoleTable::roleSidelinePush(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getBoundaryPushTarget(ball.pos);
    MotionControl::moveToPoint(robot, target, 70.0, vel, ControlParams());
}

// ==================== 防守角色实现 ====================

void RoleTable::roleClear(const RobotPose& robot, const BallInfo& ball,
                          const FieldGeometry& field, const ControlParams& params,
                          WheelVelocity& vel) {
    Point target = field.getOppGoalPos();
    double distToBall = pointToPointDistance(robot, ball.pos);

    if (distToBall < 10) {
        double angleToTarget = angleToPoint(TO_POINT(robot), target);
        double angleError = normalizeAngle(robot.theta - angleToTarget);

        if (fabs(angleError) > 0.15) {
            MotionControl::turnToAngle(robot.theta, angleToTarget, vel, params);
        } else {
            vel.left = 74;
            vel.right = 74;
        }
    } else {
        MotionControl::moveToPoint(robot, ball.pos, 70.0, vel, params);
    }
}

void RoleTable::roleSweeper(const RobotPose& robot, const BallInfo& ball,
                            const FieldGeometry& field, WheelVelocity& vel) {
    Point goalPos = field.getOurGoalPos();
    double defenseX = goalPos.x + (field.isOurGoalOnRight() ? -20 : 20);
    Point target(defenseX, ball.pos.y);
    target.y = std::max(30.0, std::min(field.getFieldHeight() - 30.0, target.y));

    roleWait(robot, target, ball, vel);
}

void RoleTable::roleZonalDefense(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel) {
    Point stdBall = field.transformToStandard(ball.pos);
    double zoneX = 30.0 + (stdBall.x / field.getFieldWidth()) * 30.0;
    zoneX = std::max(25.0, std::min(60.0, zoneX));

    Point target(zoneX, ball.pos.y);
    target.y = std::max(20.0, std::min(field.getFieldHeight() - 20.0, target.y));
    target = field.transformFromStandard(target);

    roleWait(robot, target, ball, vel);
}

void RoleTable::roleManMark(const RobotPose& robot, const Point& opponent,
                            WheelVelocity& vel) {
    Point target = opponent;
    target.x -= 8;
    target.y = opponent.y;

    MotionControl::moveToPoint(robot, target, 65.0, vel, ControlParams());
}

void RoleTable::roleOffsideTrap(const RobotPose& robot, const BallInfo& ball,
                                const FieldGeometry& field, WheelVelocity& vel) {
    (void)ball;
    double trapX = field.getFieldWidth() / 2 - 10;
    Point target(trapX, field.getFieldHeight() / 2);
    target.y = robot.y;

    MotionControl::moveToPoint(robot, target, 60.0, vel, ControlParams());
}

void RoleTable::roleTackle(const RobotPose& robot, const BallInfo& ball,
                           const FieldGeometry& field, WheelVelocity& vel) {
    (void)field;
    MotionControl::moveToPoint(robot, ball.pos, 80.0, vel, ControlParams());
}

void RoleTable::roleIntercept(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel) {
    (void)field;
    Point predictedPos = ball.pos;
    predictedPos.x += ball.vel_x * 0.3;
    predictedPos.y += ball.vel_y * 0.3;

    roleWait(robot, predictedPos, ball, vel);
}

void RoleTable::roleMark(const RobotPose& robot, const Point& opponent,
                         WheelVelocity& vel) {
    Point target = opponent;
    target.x -= 5;
    target.y = opponent.y;

    MotionControl::moveToPoint(robot, target, 60.0, vel, ControlParams());
}

void RoleTable::roleBlockShot(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel) {
    Point goalPos = field.getOurGoalPos();
    Point blockPos;

    if (ball.pos.x > goalPos.x - 30) {
        double t = (goalPos.x - ball.pos.x) / (ball.vel_x + 0.001);
        double y = ball.pos.y + ball.vel_y * t;
        blockPos = Point(goalPos.x - 15, y);
    } else {
        blockPos = Point(goalPos.x - 20, goalPos.y);
    }

    blockPos.y = std::max(20.0, std::min(field.getFieldHeight() - 20.0, blockPos.y));
    MotionControl::moveToPoint(robot, blockPos, 70.0, vel, ControlParams());
}

void RoleTable::roleCoverGap(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel) {
    Point goalPos = field.getOurGoalPos();
    Point coverPos(goalPos.x - 15, goalPos.y);

    if (ball.pos.y > goalPos.y + 15) {
        coverPos.y = goalPos.y + 20;
    } else if (ball.pos.y < goalPos.y - 15) {
        coverPos.y = goalPos.y - 20;
    }

    MotionControl::moveToPoint(robot, coverPos, 65.0, vel, ControlParams());
}

void RoleTable::roleSpecialDefender(const RobotPose& robot, const BallInfo& ball,
                                    const FieldGeometry& field, bool isUp,
                                    WheelVelocity& vel) {
    double offset = isUp ? 12 : -12;
    Point target(18, std::min(field.getFieldHeight() - 20.0,
                              std::max(20.0, ball.pos.y + offset)));
    target = field.transformFromStandard(target);

    double dist = pointToPointDistance(TO_POINT(robot), target);
    if (dist < 3) {
        MotionControl::turnToAngle(robot.theta, M_PI/2, vel, ControlParams());
    } else {
        MotionControl::moveToPoint(robot, target, 70, vel, ControlParams());
    }
}

void RoleTable::rolePenaltyArea(const RobotPose& robot, const BallInfo& ball,
                                const FieldGeometry& field, bool isUp,
                                WheelVelocity& vel) {
    (void)ball;
    double goalCenterY = field.getFieldHeight() / 2;
    Point target = isUp ? Point(45, goalCenterY + 20) : Point(45, goalCenterY - 20);
    target = field.transformFromStandard(target);

    double dist = pointToPointDistance(TO_POINT(robot), target);
    if (dist < 3) {
        MotionControl::turnToAngle(robot.theta, M_PI/2, vel, ControlParams());
    } else {
        MotionControl::moveToPoint(robot, target, 65, vel, ControlParams());
    }
}

void RoleTable::roleGoalLineDefense(const RobotPose& robot, const BallInfo& ball,
                                    const FieldGeometry& field, WheelVelocity& vel) {
    Point goalPos = field.getOurGoalPos();
    Point target(goalPos.x - 10, ball.pos.y);
    target.y = std::max(goalPos.y - 20, std::min(goalPos.y + 20, target.y));

    MotionControl::moveToPoint(robot, target, 70.0, vel, ControlParams());
}

void RoleTable::roleNearPost(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel) {
    Point goalPos = field.getOurGoalPos();
    Point nearPost(goalPos.x - 8, goalPos.y - 12);
    if (ball.pos.y > goalPos.y) {
        nearPost.y = goalPos.y + 12;
    }

    MotionControl::moveToPoint(robot, nearPost, 65.0, vel, ControlParams());
}

void RoleTable::roleFarPost(const RobotPose& robot, const BallInfo& ball,
                            const FieldGeometry& field, WheelVelocity& vel) {
    Point goalPos = field.getOurGoalPos();
    Point farPost(goalPos.x - 8, goalPos.y + 12);
    if (ball.pos.y > goalPos.y) {
        farPost.y = goalPos.y - 12;
    }

    MotionControl::moveToPoint(robot, farPost, 65.0, vel, ControlParams());
}

// ==================== 进攻角色实现 ====================

void RoleTable::roleShoot(const RobotPose& robot, const BallInfo& ball,
                          const FieldGeometry& field, const ControlParams& params,
                          WheelVelocity& vel) {
    if (Shoot::shouldShoot(robot, ball, field)) {
        Point goal = Shoot::getShootTarget(robot, ball, field);
        Shoot::endProcess(robot, ball, goal, vel, params);
    } else {
        MotionControl::moveToPoint(robot, ball.pos, 75.0, vel, params);
    }
}

void RoleTable::roleDirectCharge(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, const ControlParams& params,
                                 WheelVelocity& vel) {
    Point target = field.getOppGoalPos();
    target.x -= 30;

    if (pointToPointDistance(robot, ball.pos) < 15) {
        roleShoot(robot, ball, field, params, vel);
    } else {
        MotionControl::moveToPoint(robot, target, 85.0, vel, params);
    }
}

void RoleTable::roleHighSpeedLine(const RobotPose& robot, const BallInfo& ball,
                                  const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();
    double dist = pointToPointDistance(TO_POINT(robot), target);

    if (dist < 20) {
        roleShoot(robot, ball, field, ControlParams(), vel);
    } else {
        MotionControl::moveToPoint(robot, target, 85, vel, ControlParams());
    }
}

void RoleTable::roleHighSpeedArc(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel) {
    (void)ball;
    Point target = field.getOppGoalPos();
    double angleError = normalizeAngle(angleToPoint(TO_POINT(robot), target) - robot.theta);

    if (fabs(angleError) > 0.2) {
        MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
    } else {
        vel.left = 80;
        vel.right = 70;
    }
}

void RoleTable::roleWing(const RobotPose& robot, const BallInfo& ball,
                         const FieldGeometry& field, bool isLeft,
                         WheelVelocity& vel) {
    Point target;
    if (isLeft) {
        target = Point(35, ball.pos.y);
    } else {
        target = Point(field.getFieldWidth() - 35, ball.pos.y);
    }

    target.x = std::max(15.0, std::min(field.getFieldWidth() - 15.0, target.x));
    target.y = std::max(15.0, std::min(field.getFieldHeight() - 15.0, target.y));

    roleWait(robot, target, ball, vel);
}

void RoleTable::roleHoldBall(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel) {
    Point target = ball.pos;
    double distToBall = pointToPointDistance(robot, ball.pos);

    if (distToBall < 5) {
        double angle = angleToPoint(robot, field.getOppGoalPos());
        target.x = robot.x - 10 * cos(angle);
        target.y = robot.y - 10 * sin(angle);
    }

    roleWait(robot, target, ball, vel);
}

void RoleTable::roleCross(const RobotPose& robot, const BallInfo& ball,
                          const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();
    target.y = ball.pos.y;

    double distToBall = pointToPointDistance(robot, ball.pos);
    if (distToBall < 12) {
        double angleError = normalizeAngle(robot.theta - angleToPoint(TO_POINT(robot), target));
        if (fabs(angleError) > 0.15) {
            MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
        } else {
            vel.left = 70;
            vel.right = 60;
        }
    } else {
        MotionControl::moveToPoint(robot, ball.pos, 70.0, vel, ControlParams());
    }
}

void RoleTable::roleHeader(const RobotPose& robot, const BallInfo& ball,
                           const FieldGeometry& field, WheelVelocity& vel) {
    Point target = ball.pos;
    target.y += (ball.pos.y > field.getFieldHeight()/2 ? -5 : 5);

    MotionControl::moveToPoint(robot, target, 75.0, vel, ControlParams());
}

void RoleTable::roleVolley(const RobotPose& robot, const BallInfo& ball,
                           const FieldGeometry& field, WheelVelocity& vel) {
    (void)field;
    if (pointToPointDistance(robot, ball.pos) < 10 && fabs(ball.vel_y) > 10) {
        vel.left = 80;
        vel.right = 80;
    } else {
        MotionControl::moveToPoint(robot, ball.pos, 75.0, vel, ControlParams());
    }
}

void RoleTable::roleQuickCounter(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();

    if (pointToPointDistance(robot, ball.pos) < 12) {
        roleShoot(robot, ball, field, ControlParams(), vel);
    } else {
        MotionControl::moveToPoint(robot, target, 85.0, vel, ControlParams());
    }
}

void RoleTable::rolePressure(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel) {
    (void)field;
    MotionControl::moveToPoint(robot, ball.pos, 80.0, vel, ControlParams());
}

void RoleTable::roleFallback(const RobotPose& robot, const BallInfo& /*ball*/,
                             const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOurGoalPos();
    target.x += 20;

    MotionControl::moveToPoint(robot, target, 65.0, vel, ControlParams());
}

void RoleTable::roleDecoy(const RobotPose& robot, const BallInfo& ball,
                          const FieldGeometry& field, WheelVelocity& vel) {
    Point target = ball.pos;
    target.x += 15;
    target.y += (ball.pos.y > field.getFieldHeight()/2 ? -20 : 20);

    MotionControl::moveToPoint(robot, target, 70.0, vel, ControlParams());
}

void RoleTable::rolePenetration(const RobotPose& robot, const BallInfo& ball,
                                const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();
    target.x -= 20;
    target.y = ball.pos.y;

    MotionControl::moveToPoint(robot, target, 80.0, vel, ControlParams());
}

void RoleTable::roleChannelRun(const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();

    if (ball.pos.y > field.getFieldHeight()/2) {
        target.y = field.getFieldHeight() - 30;
    } else {
        target.y = 30;
    }

    MotionControl::moveToPoint(robot, target, 80.0, vel, ControlParams());
}

// ==================== 等待接应角色实现 ====================

void RoleTable::roleWait(const RobotPose& robot, const Point& target,
                         const BallInfo& ball, WheelVelocity& vel) {
    double dist = pointToPointDistance(TO_POINT(robot), target);

    if (dist < 3) {
        double targetAngle = angleToPoint(robot, ball.pos);
        MotionControl::turnToAngle(robot.theta, targetAngle, vel, ControlParams());
    } else {
        MotionControl::moveToPoint(robot, target, 65.0, vel, ControlParams());
    }
}

void RoleTable::roleWait135Up(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel) {
    Point target(25, field.getFieldHeight() - 40);
    target = field.transformFromStandard(target);
    roleWait(robot, target, ball, vel);
}

void RoleTable::roleWait45Up(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel) {
    Point target(25, 40);
    target = field.transformFromStandard(target);
    roleWait(robot, target, ball, vel);
}

void RoleTable::roleWaitHorizontal(const RobotPose& robot, const BallInfo& ball,
                                   const FieldGeometry& field, double offset,
                                   WheelVelocity& vel) {
    Point target(offset, ball.pos.y);
    target = field.transformFromStandard(target);
    roleWait(robot, target, ball, vel);
}

void RoleTable::roleWaitCenter(const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field, WheelVelocity& vel) {
    Point target(field.getFieldWidth() / 2, field.getFieldHeight() / 2);
    roleWait(robot, target, ball, vel);
}

void RoleTable::roleWaitLeft(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel) {
    Point target(40, ball.pos.y);
    target = field.transformFromStandard(target);
    roleWait(robot, target, ball, vel);
}

void RoleTable::roleWaitRight(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel) {
    Point target(field.getFieldWidth() - 40, ball.pos.y);
    target = field.transformFromStandard(target);
    roleWait(robot, target, ball, vel);
}

void RoleTable::roleWaitSupport(const RobotPose& robot, const BallInfo& ball,
                                const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getSupportPosition(ball.pos, 0);
    roleWait(robot, target, ball, vel);
}

// ==================== 传球角色实现 ====================

void RoleTable::roleLongPass(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();
    target.x -= 40;

    double angleError = normalizeAngle(robot.theta - angleToPoint(TO_POINT(robot), target));
    if (fabs(angleError) > 0.15) {
        MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
    } else {
        vel.left = 75;
        vel.right = 75;
    }
}

void RoleTable::roleShortPass(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getSupportPosition(ball.pos, 0);

    double angleError = normalizeAngle(robot.theta - angleToPoint(TO_POINT(robot), target));
    if (fabs(angleError) > 0.1) {
        MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
    } else {
        vel.left = 60;
        vel.right = 60;
    }
}

void RoleTable::roleThroughBall(const RobotPose& robot, const BallInfo& ball,
                                const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();

    double angleError = normalizeAngle(robot.theta - angleToPoint(TO_POINT(robot), target));
    if (fabs(angleError) > 0.1) {
        MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
    } else {
        vel.left = 80;
        vel.right = 80;
    }
}

void RoleTable::roleWallPass(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel) {
    Point target = ball.pos;
    target.x += 20;

    double angleError = normalizeAngle(robot.theta - angleToPoint(TO_POINT(robot), target));
    if (fabs(angleError) > 0.12) {
        MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
    } else {
        vel.left = 65;
        vel.right = 65;
    }
}

void RoleTable::roleOverlap(const RobotPose& robot, const BallInfo& ball,
                            const FieldGeometry& field, WheelVelocity& vel) {
    Point target = ball.pos;
    target.x += 30;
    target.y += 20;

    MotionControl::moveToPoint(robot, target, 80.0, vel, ControlParams());
}

void RoleTable::roleUnderlap(const RobotPose& robot, const BallInfo& ball,
    const FieldGeometry& /*field*/, WheelVelocity& vel) {
    Point target = ball.pos;
    target.x += 30;
    target.y -= 20;

    MotionControl::moveToPoint(robot, target, 80.0, vel, ControlParams());
}

void RoleTable::roleOneTouch(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();

    double angleError = normalizeAngle(robot.theta - angleToPoint(TO_POINT(robot), target));
    if (fabs(angleError) > 0.08) {
        MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
    } else {
        vel.left = 70;
        vel.right = 70;
    }
}

void RoleTable::roleBackPass(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOurGoalPos();
    target.x += 20;

    double angleError = normalizeAngle(robot.theta - angleToPoint(TO_POINT(robot), target));
    if (fabs(angleError) > 0.1) {
        MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
    } else {
        vel.left = 55;
        vel.right = 55;
    }
}

void RoleTable::roleSwitchSide(const RobotPose& robot, const BallInfo& /*ball*/,
                               const FieldGeometry& field, WheelVelocity& vel) {
    Point target(field.getFieldWidth() / 2, field.getFieldHeight() / 2);

    double angleError = normalizeAngle(robot.theta - angleToPoint(TO_POINT(robot), target));
    if (fabs(angleError) > 0.15) {
        MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
    } else {
        vel.left = 70;
        vel.right = 70;
    }
}

// ==================== 门将角色实现 ====================

void RoleTable::roleGoalie(const RobotPose& robot, const BallInfo& ball,
                           const FieldGeometry& field, const ControlParams& params,
                           WheelVelocity& vel) {
    Goalie::goalieAction(robot, ball, vel, field, params);
}

void RoleTable::roleGoalieNormal(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, const ControlParams& params,
                                 WheelVelocity& vel) {
    roleGoalie(robot, ball, field, params, vel);
}

void RoleTable::roleGoalieAggressive(const RobotPose& robot, const BallInfo& ball,
                                     const FieldGeometry& field, const ControlParams& params,
                                     WheelVelocity& vel) {
    double distToBall = pointToPointDistance(robot, ball.pos);
    double ballToGoal = pointToPointDistance(ball.pos, field.getOurGoalPos());

    if (distToBall < 50 && ballToGoal < 60 && ball.vel_x > 5) {
        MotionControl::moveToPoint(robot, ball.pos, 75.0, vel, params);
    } else {
        roleGoalie(robot, ball, field, params, vel);
    }
}

void RoleTable::roleGoalieConservative(const RobotPose& robot, const BallInfo& ball,
                                       const FieldGeometry& field, const ControlParams& params,
                                       WheelVelocity& vel) {
    double defendX = field.getOurGoalLineX();
    if (field.isOurGoalOnRight()) {
        defendX -= 12;
    } else {
        defendX += 12;
    }

    Point target(defendX, field.getFieldHeight() / 2);
    MotionControl::moveToPoint(robot, target, 50.0, vel, params);
}

void RoleTable::roleGoalieSweeper(const RobotPose& robot, const BallInfo& ball,
                                  const FieldGeometry& field, const ControlParams& params,
                                  WheelVelocity& vel) {
    Point stdBall = field.transformToStandard(ball.pos);

    if (stdBall.x > 40 && stdBall.x < field.getFieldWidth() - 40) {
        Point target(stdBall.x, field.getFieldHeight() / 2);
        target = field.transformFromStandard(target);
        MotionControl::moveToPoint(robot, target, 70.0, vel, params);
    } else {
        roleGoalie(robot, ball, field, params, vel);
    }
}

void RoleTable::roleGoalieOneOnOne(const RobotPose& robot, const BallInfo& ball,
                                   const FieldGeometry& field, const ControlParams& params,
                                   WheelVelocity& vel) {
    double angleToBall = angleToPoint(robot, ball.pos);
    double closingAngle = angleToBall + M_PI / 6;

    Point closingPoint(robot.x + 15 * cos(closingAngle),
                       robot.y + 15 * sin(closingAngle));

    MotionControl::moveToPoint(robot, closingPoint, 80.0, vel, params);
}

// ==================== 特殊战术角色实现 ====================

void RoleTable::roleCornerKicker(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();

    double angleError = normalizeAngle(robot.theta - angleToPoint(TO_POINT(robot), target));
    if (fabs(angleError) > 0.15) {
        MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
    } else {
        vel.left = 70;
        vel.right = 60;
    }
}

void RoleTable::roleFreeKicker(const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();

    if (pointToPointDistance(robot, ball.pos) < 10) {
        double angleError = normalizeAngle(robot.theta - angleToPoint(TO_POINT(robot), target));
        if (fabs(angleError) > 0.1) {
            MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
        } else {
            vel.left = 75;
            vel.right = 75;
        }
    } else {
        MotionControl::moveToPoint(robot, ball.pos, 70.0, vel, ControlParams());
    }
}

void RoleTable::rolePenaltyKicker(const RobotPose& robot, const BallInfo& ball,
                                  const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();
    target.y = field.getFieldHeight() / 2;

    double angleError = normalizeAngle(robot.theta - angleToPoint(TO_POINT(robot), target));
    if (fabs(angleError) > 0.05) {
        MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
    } else {
        vel.left = 90;
        vel.right = 90;
    }
}

void RoleTable::roleKickoffTaker(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel) {
    Point target = field.getOppGoalPos();

    if (pointToPointDistance(robot, ball.pos) < 8) {
        double angleError = normalizeAngle(robot.theta - angleToPoint(TO_POINT(robot), target));
        if (fabs(angleError) > 0.1) {
            MotionControl::turnToAngle(robot.theta, angleToPoint(TO_POINT(robot), target), vel, ControlParams());
        } else {
            vel.left = 60;
            vel.right = 60;
        }
    } else {
        MotionControl::moveToPoint(robot, ball.pos, 65.0, vel, ControlParams());
    }
}

// ==================== 辅助角色实现 ====================

void RoleTable::roleAssistNear(const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field, bool isUp,
                               WheelVelocity& vel) {
    Point target;
    if (isUp) {
        target = Point(35, ball.pos.y + 15);
    } else {
        target = Point(35, ball.pos.y - 15);
    }
    target = field.transformFromStandard(target);
    roleWait(robot, target, ball, vel);
}

void RoleTable::roleTriangle(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, int type,
                             WheelVelocity& vel) {
    Point target;
    switch (type) {
    case ROLE_TRIANGLE_CUT:
        target = Point(50, ball.pos.y);
        break;
    case ROLE_TRIANGLE_ASSIST:
        target = Point(30, ball.pos.y + 20);
        break;
    case ROLE_TRIANGLE_FRONT:
        target = Point(70, ball.pos.y);
        break;
    case ROLE_TRIANGLE_BACK:
        target = Point(20, ball.pos.y);
        break;
    default:
        target = ball.pos;
        break;
    }
    target = field.transformFromStandard(target);
    roleWait(robot, target, ball, vel);
}

// ==================== 辅助函数 ====================

double RoleTable::calculateAvoidanceForce(const RobotPose& robot, const Point& target,
                                          const Point obstacles[], int count,
                                          Point& forceOut) {
    double forceX = 0, forceY = 0;
    double maxForce = 0;

    for (int i = 0; i < count; i++) {
        double dx = robot.x - obstacles[i].x;
        double dy = robot.y - obstacles[i].y;
        double dist = sqrt(dx*dx + dy*dy);

        if (dist < 25.0 && dist > 0.01) {
            double strength = (25.0 - dist) / 25.0 * 100.0;
            strength = std::min(strength, 80.0);
            forceX += dx / dist * strength;
            forceY += dy / dist * strength;
            maxForce = std::max(maxForce, strength);
        }
    }

    forceOut.x = forceX;
    forceOut.y = forceY;
    return maxForce;
}

bool RoleTable::isCollisionRisk(const RobotPose& robot, const Point& target,
                                const Point obstacles[], int count) {
    for (int i = 0; i < count; i++) {
        double dist = pointToPointDistance(robot, obstacles[i]);
        if (dist < 15.0) return true;
    }
    return false;
}

// ==================== 公开接口函数 ====================

Point RoleTable::getRoleTarget(int roleId, const RobotPose& robot,
                               const BallInfo& ball, const FieldGeometry& field) {
    Point stdBall = field.transformToStandard(ball.pos);

    switch (roleId) {
    case ROLE_LEFT_BOUND:
        return Point(8, stdBall.y);
    case ROLE_TOP_BOUND:
        return Point(stdBall.x, field.getFieldHeight() - 8);
    case ROLE_RIGHT_BOUND:
        return Point(field.getFieldWidth() - 8, stdBall.y);
    case ROLE_BOTTOM_BOUND:
        return Point(stdBall.x, 8);
    case ROLE_BOUND_PUSH:
    case ROLE_PUSH_OUT:
        return field.getBoundaryPushTarget(ball.pos);
    case ROLE_SHOOT:
        return Shoot::getShootTarget(robot, ball, field);
    case ROLE_GOALIE:
    case ROLE_GOALIE_NORMAL:
    case ROLE_GOALIE_AGGRESSIVE:
    case ROLE_GOALIE_CONSERVATIVE:
        return field.getOurGoalPos();
    case ROLE_SPECIAL_DEFENDER_UP:
        return Point(18, stdBall.y + 12);
    case ROLE_SPECIAL_DEFENDER_DOWN:
        return Point(18, stdBall.y - 12);
    case ROLE_PENALTY_AREA_UP:
        return Point(45, field.getFieldHeight()/2 + 20);
    case ROLE_PENALTY_AREA_DOWN:
        return Point(45, field.getFieldHeight()/2 - 20);
    case ROLE_WAIT_135_UP:
        return Point(25, field.getFieldHeight() - 40);
    case ROLE_WAIT_45_UP:
        return Point(25, 40);
    case ROLE_WAIT_CENTER:
        return Point(field.getFieldWidth()/2, field.getFieldHeight()/2);
    case ROLE_WAIT_LEFT:
        return Point(40, stdBall.y);
    case ROLE_WAIT_RIGHT:
        return Point(field.getFieldWidth() - 40, stdBall.y);
    default:
        return ball.pos;
    }
}

double RoleTable::getRoleMaxSpeed(int roleId) {
    switch (roleId) {
    case ROLE_HIGH_SPEED_LINE:
    case ROLE_HIGH_SPEED_ARC:
    case ROLE_DIRECT_CHARGE:
    case ROLE_QUICK_COUNTER:
        return 85.0;
    case ROLE_SHOOT:
        return 80.0;
    case ROLE_GOALIE_AGGRESSIVE:
        return 75.0;
    case ROLE_GOALIE_CONSERVATIVE:
        return 50.0;
    case ROLE_STOP:
        return 0;
    default:
        return 65.0;
    }
}

std::string RoleTable::getRoleName(int roleId) {
    switch (roleId) {
    case ROLE_STOP: return "停止";
    case ROLE_LEFT_BOUND: return "左边界";
    case ROLE_TOP_BOUND: return "上边界";
    case ROLE_RIGHT_BOUND: return "右边界";
    case ROLE_BOTTOM_BOUND: return "下边界";
    case ROLE_BOUND_PUSH: return "边线推球";
    case ROLE_PUSH_OUT: return "推出边界";
    case ROLE_SHOOT: return "射门";
    case ROLE_GOALIE: return "守门员";
    case ROLE_SPECIAL_DEFENDER_UP: return "上专职后卫";
    case ROLE_SPECIAL_DEFENDER_DOWN: return "下专职后卫";
    case ROLE_PENALTY_AREA_UP: return "大禁区上后卫";
    case ROLE_PENALTY_AREA_DOWN: return "大禁区下后卫";
    case ROLE_HIGH_SPEED_LINE: return "高速直线";
    case ROLE_HIGH_SPEED_ARC: return "高速弧线";
    case ROLE_DIRECT_CHARGE: return "直冲";
    case ROLE_WAIT_135_UP: return "135度等球上";
    case ROLE_WAIT_45_UP: return "45度等球上";
    case ROLE_WAIT_CENTER: return "中间等球";
    case ROLE_WAIT_LEFT: return "左边等球";
    case ROLE_WAIT_RIGHT: return "右边等球";
    default: return "未知角色";
    }
}

double RoleTable::getRoleAggression(int roleId) {
    switch (roleId) {
    case ROLE_SHOOT:
    case ROLE_DIRECT_CHARGE:
    case ROLE_TACKLE:
        return 0.95;
    case ROLE_HIGH_SPEED_LINE:
    case ROLE_HIGH_SPEED_ARC:
    case ROLE_WING_LEFT:
    case ROLE_WING_RIGHT:
        return 0.85;
    case ROLE_GOALIE_AGGRESSIVE:
        return 0.8;
    case ROLE_GOALIE_CONSERVATIVE:
        return 0.2;
    default:
        return 0.5;
    }
}

double RoleTable::getRoleDefense(int roleId) {
    switch (roleId) {
    case ROLE_SPECIAL_DEFENDER_UP:
    case ROLE_SPECIAL_DEFENDER_DOWN:
    case ROLE_PENALTY_AREA_UP:
    case ROLE_PENALTY_AREA_DOWN:
    case ROLE_SWEEPER:
    case ROLE_MAN_MARK:
    case ROLE_TACKLE:
    case ROLE_INTERCEPT:
    case ROLE_BLOCK_SHOT:
        return 0.9;
    case ROLE_GOALIE:
    case ROLE_GOALIE_NORMAL:
    case ROLE_GOALIE_CONSERVATIVE:
        return 0.95;
    default:
        return 0.4;
    }
}

double RoleTable::getRoleOffense(int roleId) {
    switch (roleId) {
    case ROLE_SHOOT:
    case ROLE_DIRECT_CHARGE:
    case ROLE_WING_LEFT:
    case ROLE_WING_RIGHT:
    case ROLE_CROSS:
    case ROLE_THROUGH_BALL:
        return 0.9;
    case ROLE_HIGH_SPEED_LINE:
    case ROLE_HIGH_SPEED_ARC:
        return 0.85;
    default:
        return 0.3;
    }
}

bool RoleTable::shouldPass(int roleId, const RobotPose& robot, const BallInfo& ball) {
    (void)roleId;
    double distToBall = pointToPointDistance(robot, ball.pos);
    return distToBall < 12 && distToBall > 3;
}

Point RoleTable::getPassTarget(int roleId, const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field) {
    (void)roleId;
    return field.getSupportPosition(ball.pos, 0);
}

void RoleTable::executeRole(int roleId, const RobotPose& robot, const BallInfo& ball,
                            const Point oppRobots[], int oppCount,
                            const FieldGeometry& field, const ControlParams& params,
                            WheelVelocity& vel) {
    switch (roleId) {
    case ROLE_STOP:
        roleStop(robot, vel);
        break;
    case ROLE_LEFT_BOUND:
        roleLeftBound(robot, ball, field, vel);
        break;
    case ROLE_TOP_BOUND:
        roleTopBound(robot, ball, field, vel);
        break;
    case ROLE_RIGHT_BOUND:
        roleRightBound(robot, ball, field, vel);
        break;
    case ROLE_BOTTOM_BOUND:
        roleBottomBound(robot, ball, field, vel);
        break;
    case ROLE_BOUND_PUSH:
        roleBoundPush(robot, ball, field, vel);
        break;
    case ROLE_PUSH_OUT:
        rolePushOut(robot, ball, field, vel);
        break;
    case ROLE_CORNER_DEFENSE:
        roleCornerDefense(robot, ball, field, vel);
        break;
    case ROLE_CLEAR:
        roleClear(robot, ball, field, params, vel);
        break;
    case ROLE_SWEEPER:
        roleSweeper(robot, ball, field, vel);
        break;
    case ROLE_ZONAL_DEFENSE:
        roleZonalDefense(robot, ball, field, vel);
        break;
    case ROLE_MAN_MARK:
        if (oppCount > 0) roleManMark(robot, oppRobots[0], vel);
        break;
    case ROLE_OFFSIDE_TRAP:
        roleOffsideTrap(robot, ball, field, vel);
        break;
    case ROLE_TACKLE:
        roleTackle(robot, ball, field, vel);
        break;
    case ROLE_INTERCEPT:
        roleIntercept(robot, ball, field, vel);
        break;
    case ROLE_MARK:
        if (oppCount > 0) roleMark(robot, oppRobots[0], vel);
        break;
    case ROLE_BLOCK_SHOT:
        roleBlockShot(robot, ball, field, vel);
        break;
    case ROLE_COVER_GAP:
        roleCoverGap(robot, ball, field, vel);
        break;
    case ROLE_SPECIAL_DEFENDER_UP:
        roleSpecialDefender(robot, ball, field, true, vel);
        break;
    case ROLE_SPECIAL_DEFENDER_DOWN:
        roleSpecialDefender(robot, ball, field, false, vel);
        break;
    case ROLE_PENALTY_AREA_UP:
        rolePenaltyArea(robot, ball, field, true, vel);
        break;
    case ROLE_PENALTY_AREA_DOWN:
        rolePenaltyArea(robot, ball, field, false, vel);
        break;
    case ROLE_GOAL_LINE_DEFENSE:
        roleGoalLineDefense(robot, ball, field, vel);
        break;
    case ROLE_NEAR_POST:
        roleNearPost(robot, ball, field, vel);
        break;
    case ROLE_FAR_POST:
        roleFarPost(robot, ball, field, vel);
        break;
    case ROLE_SHOOT:
        roleShoot(robot, ball, field, params, vel);
        break;
    case ROLE_DIRECT_CHARGE:
        roleDirectCharge(robot, ball, field, params, vel);
        break;
    case ROLE_HIGH_SPEED_LINE:
        roleHighSpeedLine(robot, ball, field, vel);
        break;
    case ROLE_HIGH_SPEED_ARC:
        roleHighSpeedArc(robot, ball, field, vel);
        break;
    case ROLE_WING_LEFT:
        roleWing(robot, ball, field, true, vel);
        break;
    case ROLE_WING_RIGHT:
        roleWing(robot, ball, field, false, vel);
        break;
    case ROLE_HOLD_BALL:
        roleHoldBall(robot, ball, field, vel);
        break;
    case ROLE_CROSS:
        roleCross(robot, ball, field, vel);
        break;
    case ROLE_HEADER:
        roleHeader(robot, ball, field, vel);
        break;
    case ROLE_VOLLEY:
        roleVolley(robot, ball, field, vel);
        break;
    case ROLE_QUICK_COUNTER:
        roleQuickCounter(robot, ball, field, vel);
        break;
    case ROLE_PRESSURE:
        rolePressure(robot, ball, field, vel);
        break;
    case ROLE_FALLBACK:
        roleFallback(robot, ball, field, vel);
        break;
    case ROLE_DECOY:
        roleDecoy(robot, ball, field, vel);
        break;
    case ROLE_PENETRATION:
        rolePenetration(robot, ball, field, vel);
        break;
    case ROLE_CHANNEL_RUN:
        roleChannelRun(robot, ball, field, vel);
        break;
    case ROLE_WAIT_135_UP:
        roleWait135Up(robot, ball, field, vel);
        break;
    case ROLE_WAIT_45_UP:
        roleWait45Up(robot, ball, field, vel);
        break;
    case ROLE_WAIT_HORIZONTAL:
        roleWaitHorizontal(robot, ball, field, 25, vel);
        break;
    case ROLE_WAIT_HORIZONTAL_2:
        roleWaitHorizontal(robot, ball, field, 35, vel);
        break;
    case ROLE_WAIT_HORIZONTAL_3:
        roleWaitHorizontal(robot, ball, field, 45, vel);
        break;
    case ROLE_WAIT_CENTER:
        roleWaitCenter(robot, ball, field, vel);
        break;
    case ROLE_WAIT_LEFT:
        roleWaitLeft(robot, ball, field, vel);
        break;
    case ROLE_WAIT_RIGHT:
        roleWaitRight(robot, ball, field, vel);
        break;
    case ROLE_WAIT_SUPPORT:
        roleWaitSupport(robot, ball, field, vel);
        break;
    case ROLE_LONG_PASS:
        roleLongPass(robot, ball, field, vel);
        break;
    case ROLE_SHORT_PASS:
        roleShortPass(robot, ball, field, vel);
        break;
    case ROLE_THROUGH_BALL:
        roleThroughBall(robot, ball, field, vel);
        break;
    case ROLE_WALL_PASS:
        roleWallPass(robot, ball, field, vel);
        break;
    case ROLE_OVERLAP:
        roleOverlap(robot, ball, field, vel);
        break;
    case ROLE_UNDERLAP:
        roleUnderlap(robot, ball, field, vel);
        break;
    case ROLE_ONE_TOUCH:
        roleOneTouch(robot, ball, field, vel);
        break;
    case ROLE_BACK_PASS:
        roleBackPass(robot, ball, field, vel);
        break;
    case ROLE_SWITCH_SIDE:
        roleSwitchSide(robot, ball, field, vel);
        break;
    case ROLE_GOALIE:
    case ROLE_GOALIE_NORMAL:
        roleGoalieNormal(robot, ball, field, params, vel);
        break;
    case ROLE_GOALIE_AGGRESSIVE:
        roleGoalieAggressive(robot, ball, field, params, vel);
        break;
    case ROLE_GOALIE_CONSERVATIVE:
        roleGoalieConservative(robot, ball, field, params, vel);
        break;
    case ROLE_GOALIE_SWEEPER:
        roleGoalieSweeper(robot, ball, field, params, vel);
        break;
    case ROLE_GOALIE_ONE_ON_ONE:
        roleGoalieOneOnOne(robot, ball, field, params, vel);
        break;
    case ROLE_CORNER_KICKER:
        roleCornerKicker(robot, ball, field, vel);
        break;
    case ROLE_FREE_KICKER:
        roleFreeKicker(robot, ball, field, vel);
        break;
    case ROLE_PENALTY_KICKER:
        rolePenaltyKicker(robot, ball, field, vel);
        break;
    case ROLE_KICKOFF_TAKER:
        roleKickoffTaker(robot, ball, field, vel);
        break;
    case ROLE_ASSIST_NEAR_UP:
        roleAssistNear(robot, ball, field, true, vel);
        break;
    case ROLE_ASSIST_NEAR_DOWN:
        roleAssistNear(robot, ball, field, false, vel);
        break;
    case ROLE_TRIANGLE_CUT:
    case ROLE_TRIANGLE_ASSIST:
    case ROLE_TRIANGLE_FRONT:
    case ROLE_TRIANGLE_BACK:
        roleTriangle(robot, ball, field, roleId, vel);
        break;
    default:
        roleWait(robot, ball.pos, ball, vel);
        break;
    }
}
