// strategyfactory.h
#ifndef STRATEGYFACTORY_H
#define STRATEGYFACTORY_H

#include "StrategyBase.h"
#include "export.h" 

enum class StrategyType {
    UNIFIED = 0
};


// ========== C 接口函数声明（使用统一的导出宏）==========
extern "C" {
    ROBOTSTRATEGYDLL_API void* __cdecl CreateStrategy(int type);
    ROBOTSTRATEGYDLL_API void __cdecl DestroyStrategy(void* strategy);
    ROBOTSTRATEGYDLL_API void __cdecl decide(void* strategy, const RobotPose* robots,
        const Point* oppRobots, const BallInfo* ball,
        WheelVelocity* velocities);
    ROBOTSTRATEGYDLL_API void __cdecl reset(void* strategy);
    ROBOTSTRATEGYDLL_API void __cdecl setParameter(void* strategy, const char* key, double value);
    ROBOTSTRATEGYDLL_API double __cdecl getParameter(void* strategy, const char* key);
    ROBOTSTRATEGYDLL_API const char* __cdecl GetStrategyName(int type);
    ROBOTSTRATEGYDLL_API void __cdecl setOurGoalOnRight(void* strategy, bool onRight);
    ROBOTSTRATEGYDLL_API void __cdecl setOurKickoff(void* strategy, bool isOurKickoff);
    ROBOTSTRATEGYDLL_API int __cdecl TestFunction();
    ROBOTSTRATEGYDLL_API void __cdecl InitializeStrategy(void* strategy,
        int ourScore, int oppScore, int remainingTime,
        int isFirstHalf, int isOurKickoff);
    ROBOTSTRATEGYDLL_API int __cdecl GetCurrentStrategyMode(void* strategy);
    ROBOTSTRATEGYDLL_API void __cdecl SetStrategyMode(void* strategy, int mode);
}

// ========== C++ 包装函数（内联，避免重复定义）==========
inline StrategyBase* createStrategy(StrategyType type) {
    return static_cast<StrategyBase*>(CreateStrategy(static_cast<int>(type)));
}

inline void destroyStrategy(StrategyBase* strategy) {
    DestroyStrategy(strategy);
}

#endif // STRATEGYFACTORY_H