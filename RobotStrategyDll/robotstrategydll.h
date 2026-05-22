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

// 比赛状态控制
    ROBOTSTRATEGYDLL_EXPORT void SetMatchState(void* strategy, int state);
    // ==================== 新增接口（用于比赛界面完整控制）====================

// 开球方式设置
// type: 0=普通, 1=点球, 2=门球, 3=任意球, 4=争球, 5=收车
    ROBOTSTRATEGYDLL_EXPORT void SetKickoffType(void* strategy, int type);

    // 阵型选择（单双后卫）
    ROBOTSTRATEGYDLL_EXPORT void SetFormationType(void* strategy, int isSingleDefender);

    // 开球方式（普通/点球/门球/任意球/争球/收车）
    ROBOTSTRATEGYDLL_EXPORT void SetKickoffType(void* strategy, int type);

    // 点球设置
    ROBOTSTRATEGYDLL_EXPORT void SetPenaltyKickMode(void* strategy, int shootType, int goaliePos);

    // 策略选择（1-4号策略）
    ROBOTSTRATEGYDLL_EXPORT void SelectStrategy(void* strategy, int strategyIndex);

    // 收车/归位
    ROBOTSTRATEGYDLL_EXPORT void ParkRobots(void* strategy);

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