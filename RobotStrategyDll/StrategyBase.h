// StrategyBase.h
// 策略基类 - 定义所有策略类的统一接口

#ifndef STRATEGYBASE_H
#define STRATEGYBASE_H

#include "StrategyCore.h"
#include <string>

class StrategyBase {
public:
    virtual ~StrategyBase() = default;

    // ========== 核心决策接口 ==========

    // 决策主函数
    // robots: 我方机器人位姿数组
    // oppRobots: 对方机器人位置数组
    // ball: 球的信息
    // velocities: 输出轮速数组
    virtual void decide(const RobotPose robots[], const Point oppRobots[],
                        const BallInfo& ball, WheelVelocity velocities[]) = 0;

    // 重置策略状态
    virtual void reset() = 0;

    // 设置参数
    // key: 参数名
    // value: 参数值
    virtual void setParameter(const std::string& key, double value) = 0;

    // 获取策略名称
    virtual std::string getStrategyName() const = 0;
};

#endif
