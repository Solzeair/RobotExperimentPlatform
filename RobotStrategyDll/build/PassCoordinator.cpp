// PassCoordinator.cpp
// 传球协调器实现

#include "PassCoordinator.h"
#include "MotionControl.h"
#include <algorithm>
#include <cmath>

using namespace GeometryUtils;

PassCoordinator::PassCoordinator() {}
PassCoordinator::~PassCoordinator() {}

// ========== 评估传球选项 ==========
// 遍历所有潜在接球队员，计算每个传球的成功率，选择最佳方案
PassOption PassCoordinator::evaluatePass(const RobotPose& passer, const BallInfo& ball,
                                         const RobotPose receivers[], int receiverCount,
                                         const RobotPose opponents[], int opponentCount) {
    PassOption bestOption;
    bestOption.successRate = 0;
    (void)ball;  // 未使用参数

    for (int i = 0; i < receiverCount; i++) {
        double distance = pointToPointDistance(passer, receivers[i]);

        // 传球距离超过150cm，不考虑
        if (distance > 150) continue;

        // 计算传球成功率
        double successRate = calculateSuccessRate(passer, receivers[i], opponents, opponentCount, distance);

        // 更新最佳传球选项
        if (successRate > bestOption.successRate) {
            bestOption.targetRobotId = i;
            bestOption.targetPos = receivers[i];
            bestOption.successRate = successRate;
            bestOption.passSpeed = std::min(80.0, distance / 2.0);

            // 根据距离确定传球类型
            if (distance < 40) {
                bestOption.passType = "short";
                bestOption.isOneTouch = true;
            } else if (distance < 100) {
                bestOption.passType = "long";
                bestOption.isOneTouch = false;
            } else {
                bestOption.passType = "through";
                bestOption.isOneTouch = false;
            }
        }
    }

    return bestOption;
}

// ========== 执行传球 ==========
bool PassCoordinator::executePass(const RobotPose& passer, const PassOption& pass,
                                  WheelVelocity& vel, const ControlParams& params) {
    double passAngle = angleToPoint(passer, pass.targetPos);
    double angleError = normalizeAngle(passAngle - passer.theta);

    // 角度偏差太大，先调整方向
    if (fabs(angleError) > 0.2) {
        MotionControl::turnToAngle(passer.theta, passAngle, vel, params);
        return false;
    }

    // 方向对准，执行传球
    vel.left = pass.passSpeed;
    vel.right = pass.passSpeed;
    return true;
}

// ========== 撞墙配合（二过一）==========
bool PassCoordinator::wallPass(const RobotPose& passer, const RobotPose& wallPlayer,
                               const Point& target, BallInfo& ball, WheelVelocity& vel) {
    (void)target;
    (void)ball;

    // 计算墙点（传球者和墙队员的中点）
    Point wallPoint = {
        (passer.x + wallPlayer.x) / 2,
        (passer.y + wallPlayer.y) / 2
    };

    double distToWall = pointToPointDistance(passer, wallPoint);
    double passSpeed = std::min(70.0, distToWall);

    double passAngle = angleToPoint(passer, wallPoint);
    double angleError = normalizeAngle(passAngle - passer.theta);

    // 调整方向
    if (fabs(angleError) > 0.15) {
        MotionControl::turnToAngle(passer.theta, passAngle, vel, ControlParams());
        return false;
    }

    // 执行传球
    vel.left = passSpeed;
    vel.right = passSpeed;
    return true;
}

// ========== 直塞球 ==========
// 传向跑动队员的前方，让队员在跑动中接球
bool PassCoordinator::throughBall(const RobotPose& passer, const RobotPose& runner,
                                  const Point& target, WheelVelocity& vel) {
    double runnerSpeed = sqrt(runner.vx * runner.vx + runner.vy * runner.vy);
    double runAngle = angleToPoint(runner, target);

    // 计算提前量：接球点 = 跑动队员位置 + 速度方向 * 1.5倍时间
    double leadDistance = runnerSpeed * 1.5;
    Point receivePoint = {
        runner.x + cos(runAngle) * leadDistance,
        runner.y + sin(runAngle) * leadDistance
    };

    // 限制在场地范围内
    receivePoint.x = std::max(0.0, std::min(220.0, receivePoint.x));
    receivePoint.y = std::max(0.0, std::min(180.0, receivePoint.y));

    double passAngle = angleToPoint(passer, receivePoint);
    double angleError = normalizeAngle(passAngle - passer.theta);

    // 调整方向
    if (fabs(angleError) > 0.2) {
        MotionControl::turnToAngle(passer.theta, passAngle, vel, ControlParams());
        return false;
    }

    // 执行直塞
    vel.left = 80;
    vel.right = 80;
    return true;
}

// ========== 传中 ==========
bool PassCoordinator::crossBall(const RobotPose& passer, const Point& crossPoint,
                                WheelVelocity& vel) {
    double crossAngle = angleToPoint(passer, crossPoint);
    double angleError = normalizeAngle(crossAngle - passer.theta);

    // 调整方向
    if (fabs(angleError) > 0.1) {
        MotionControl::turnToAngle(passer.theta, crossAngle, vel, ControlParams());
        return false;
    }

    // 执行传中（带弧线，左右轮速度不同）
    vel.left = 75;
    vel.right = 65;
    return true;
}

// ========== 一脚传球 ==========
bool PassCoordinator::oneTouchPass(const RobotPose& passer, const Point& receivePoint,
                                   WheelVelocity& vel) {
    double passAngle = angleToPoint(passer, receivePoint);
    double angleError = normalizeAngle(passAngle - passer.theta);

    // 调整方向（一脚传球要求角度更精确）
    if (fabs(angleError) > 0.1) {
        MotionControl::turnToAngle(passer.theta, passAngle, vel, ControlParams());
        return false;
    }

    // 执行一脚传球
    vel.left = 70;
    vel.right = 70;
    return true;
}

// ========== 计算传球成功率 ==========
// 考虑因素：距离、对方防守队员位置
double PassCoordinator::calculateSuccessRate(const RobotPose& passer, const Point& target,
                                             const RobotPose opponents[], int opponentCount,
                                             double distance) {
    (void)passer;
    double successRate = 0.9;  // 基础成功率90%

    // 距离惩罚：距离越远成功率越低
    if (distance > 80) {
        successRate *= (1.0 - (distance - 80) / 200.0);
    }

    // 防守队员惩罚：对方离传球路线越近，成功率越低
    for (int i = 0; i < opponentCount; i++) {
        double distToLine = pointToPointDistance(opponents[i], target);
        if (distToLine < 15) {
            successRate *= 0.7;
        }
    }

    return std::max(0.1, std::min(0.95, successRate));
}

// ========== 获取最佳接应点 ==========
Point PassCoordinator::getBestReceivePoint(const RobotPose& receiver, const BallInfo& ball,
                                           const RobotPose opponents[], int opponentCount) {
    (void)ball;
    Point bestPoint = receiver;

    // 寻找最近的对手
    double minOppDist = 1000;
    for (int i = 0; i < opponentCount; i++) {
        double dist = pointToPointDistance(receiver, opponents[i]);
        if (dist < minOppDist) {
            minOppDist = dist;
        }
    }

    // 如果对手太近，向远离对手的方向移动
    if (minOppDist < 20) {
        bestPoint.x += 10;
    }

    return bestPoint;
}
