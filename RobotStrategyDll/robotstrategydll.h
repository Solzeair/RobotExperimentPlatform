// robotstrategydll.h
#ifndef ROBOTSTRATEGYDLL_H
#define ROBOTSTRATEGYDLL_H

#include "robotstrategydll_global.h"
#include "StrategyCore.h"  // 添加这个，包含 RobotPose, Point, BallInfo, WheelVelocity

#ifdef __cplusplus
extern "C" {
#endif

    // 原有接口
    ROBOTSTRATEGYDLL_EXPORT void* CreateStrategy(int type);
    ROBOTSTRATEGYDLL_EXPORT void DestroyStrategy(void* strategy);
    ROBOTSTRATEGYDLL_EXPORT void decide(void* strategy, const RobotPose* robots,
        const Point* oppRobots, const BallInfo* ball,
        WheelVelocity* velocities);
    ROBOTSTRATEGYDLL_EXPORT void reset(void* strategy);
    ROBOTSTRATEGYDLL_EXPORT void setParameter(void* strategy, const char* key, double value);
    ROBOTSTRATEGYDLL_EXPORT double getParameter(void* strategy, const char* key);
    ROBOTSTRATEGYDLL_EXPORT const char* GetStrategyName(int type);
    ROBOTSTRATEGYDLL_EXPORT void setOurGoalOnRight(void* strategy, bool onRight);
    ROBOTSTRATEGYDLL_EXPORT void setOurKickoff(void* strategy, bool isOurKickoff);
    ROBOTSTRATEGYDLL_EXPORT int TestFunction(void);

    // 策略初始化接口
    ROBOTSTRATEGYDLL_EXPORT void InitializeStrategy(void* strategy,
        int ourScore, int oppScore,
        int remainingTime,
        int isFirstHalf,
        int isOurKickoff);

    // 获取/设置策略模式
    ROBOTSTRATEGYDLL_EXPORT int GetCurrentStrategyMode(void* strategy);
    ROBOTSTRATEGYDLL_EXPORT void SetStrategyMode(void* strategy, int mode);

#ifdef __cplusplus
}
#endif

#endif // ROBOTSTRATEGYDLL_H