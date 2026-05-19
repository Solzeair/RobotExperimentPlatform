// PassCoordinator.h
// 传球协调器 - 评估传球选项并执行传球动作

#ifndef PASSCOORDINATOR_H
#define PASSCOORDINATOR_H

#include "GeometryUtils.h"
#include "ControlParams.h"
#include "StrategyCore.h"
#include <string>

// 传球选项结构体
// 描述一个可行的传球方案
struct PassOption {
    int targetRobotId;      // 目标机器人ID
    Point targetPos;        // 传球目标点
    double successRate;     // 传球成功率 (0-1)
    double passSpeed;       // 传球速度 (cm/s)
    bool isOneTouch;        // 是否一脚传球
    std::string passType;   // 传球类型: "short", "long", "through", "cross"
};

class PassCoordinator {
public:
    PassCoordinator();
    ~PassCoordinator();

    // ========== 传球评估 ==========

    // 评估传球选项
    // passer: 传球机器人位姿
    // ball: 球的信息
    // receivers: 潜在接球机器人数组
    // receiverCount: 接球机器人数量
    // opponents: 对方机器人位姿数组
    // opponentCount: 对方机器人数量
    // 返回: 最佳传球选项
    PassOption evaluatePass(const RobotPose& passer, const BallInfo& ball,
                            const RobotPose receivers[], int receiverCount,
                            const RobotPose opponents[], int opponentCount);

    // ========== 传球执行 ==========

    // 执行传球
    // passer: 传球机器人位姿
    // pass: 传球选项
    // vel: 输出轮速
    // params: 控制参数
    // 返回: true-传球已执行, false-还在调整方向
    bool executePass(const RobotPose& passer, const PassOption& pass,
                     WheelVelocity& vel, const ControlParams& params);

    // ========== 特殊传球 ==========

    // 撞墙配合（二过一）
    // passer: 传球机器人
    // wallPlayer: 做墙机器人
    // target: 最终目标点
    // ball: 球的信息（输入输出）
    // vel: 输出轮速
    // 返回: true-传球已执行
    bool wallPass(const RobotPose& passer, const RobotPose& wallPlayer,
                  const Point& target, BallInfo& ball, WheelVelocity& vel);

    // 直塞球（传向跑动队员的前方）
    // passer: 传球机器人
    // runner: 前插跑动的机器人
    // target: 目标区域
    // vel: 输出轮速
    // 返回: true-传球已执行
    bool throughBall(const RobotPose& passer, const RobotPose& runner,
                     const Point& target, WheelVelocity& vel);

    // 传中（从边路传向禁区）
    // passer: 传球机器人
    // crossPoint: 传中点
    // vel: 输出轮速
    // 返回: true-传球已执行
    bool crossBall(const RobotPose& passer, const Point& crossPoint,
                   WheelVelocity& vel);

    // 一脚传球（不停球直接传出）
    // passer: 传球机器人
    // receivePoint: 接球点
    // vel: 输出轮速
    // 返回: true-传球已执行
    bool oneTouchPass(const RobotPose& passer, const Point& receivePoint,
                      WheelVelocity& vel);

    // ========== 接应点计算 ==========

    // 获取最佳接应点
    // receiver: 接球机器人
    // ball: 球的信息
    // opponents: 对方机器人数组
    // opponentCount: 对方机器人数量
    // 返回: 最佳接应点坐标
    Point getBestReceivePoint(const RobotPose& receiver, const BallInfo& ball,
                              const RobotPose opponents[], int opponentCount);

private:
    // 计算传球成功率
    // passer: 传球机器人
    // target: 传球目标点
    // opponents: 对方机器人数组
    // opponentCount: 对方机器人数量
    // distance: 传球距离
    // 返回: 成功率 (0-1)
    double calculateSuccessRate(const RobotPose& passer, const Point& target,
                                const RobotPose opponents[], int opponentCount,
                                double distance);

    // 计算接球点
    // receiver: 接球机器人
    // ballPos: 球的位置
    // ballVelocity: 球的速度
    // 返回: 最佳接球点
    Point calculateReceivePoint(const RobotPose& receiver, const Point& ballPos,
                                double ballVelocity);
};

#endif
