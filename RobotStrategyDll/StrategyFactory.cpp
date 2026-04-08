// StrategyFactory.cpp
// 策略工厂实现

#include "strategyfactory.h"
#include "UnifiedStrategy.h"
#include <cstring>
#include <iostream>

// 全局策略实例
static UnifiedStrategy* g_strategy = nullptr;

extern "C" {

// ========== 创建策略 ==========
ROBOTSTRATEGYDLL_EXPORT void* __cdecl CreateStrategy(int type) {
    (void)type;
    std::cout << "CreateStrategy called" << std::endl;
    g_strategy = new UnifiedStrategy();
    return g_strategy;
}

// ========== 销毁策略 ==========
ROBOTSTRATEGYDLL_EXPORT void __cdecl DestroyStrategy(void* strategy) {
    std::cout << "DestroyStrategy called" << std::endl;
    if (strategy) {
        delete static_cast<UnifiedStrategy*>(strategy);
        g_strategy = nullptr;
    }
}

// ========== 决策函数 ==========
ROBOTSTRATEGYDLL_EXPORT void __cdecl decide(void* strategy, const RobotPose* robots,
                                            const Point* oppRobots, const BallInfo* ball,
                                            WheelVelocity* velocities) {
    if (strategy && robots && oppRobots && ball && velocities) {
        static_cast<UnifiedStrategy*>(strategy)->decide(robots, oppRobots, *ball, velocities);
    }
}

// ========== 重置策略 ==========
ROBOTSTRATEGYDLL_EXPORT void __cdecl reset(void* strategy) {
    if (strategy) {
        static_cast<UnifiedStrategy*>(strategy)->reset();
    }
}

// ========== 设置参数 ==========
ROBOTSTRATEGYDLL_EXPORT void __cdecl setParameter(void* strategy, const char* key, double value) {
    if (strategy && key) {
        static_cast<UnifiedStrategy*>(strategy)->setParameter(std::string(key), value);
    }
}

// ========== 获取参数 ==========
ROBOTSTRATEGYDLL_EXPORT double __cdecl getParameter(void* strategy, const char* key) {
    if (strategy && key) {
        std::string keyStr(key);
        // 返回默认值
        if (keyStr == "max_speed") return 75.0;
        if (keyStr == "min_speed") return 15.0;
        if (keyStr == "kp_pos") return 12.0;
        if (keyStr == "kp_angle") return 22.0;
        if (keyStr == "attack_aggression") return 0.6;
        if (keyStr == "defense_depth") return 0.5;
        if (keyStr == "pressing_intensity") return 0.5;
    }
    return 0;
}

// ========== 获取策略名称 ==========
ROBOTSTRATEGYDLL_EXPORT const char* __cdecl GetStrategyName(int type) {
    (void)type;
    return "Unified MFC Strategy";
}

// ========== 设置我方球门方向 ==========
ROBOTSTRATEGYDLL_EXPORT void __cdecl setOurGoalOnRight(void* strategy, bool onRight) {
    if (strategy) {
        static_cast<UnifiedStrategy*>(strategy)->setOurGoalOnRight(onRight);
    }
}

// ========== 设置我方开球 ==========
ROBOTSTRATEGYDLL_EXPORT void __cdecl setOurKickoff(void* strategy, bool isOurKickoff) {
    if (strategy) {
        static_cast<UnifiedStrategy*>(strategy)->setOurKickoff(isOurKickoff);
    }
}

// ========== 测试函数 ==========
ROBOTSTRATEGYDLL_EXPORT int __cdecl TestFunction() {
    return 42;
}

}  // extern "C"

// ========== C++接口实现 ==========
StrategyBase* createStrategy(StrategyType type) {
    return static_cast<StrategyBase*>(CreateStrategy(static_cast<int>(type)));
}

void destroyStrategy(StrategyBase* strategy) {
    DestroyStrategy(strategy);
}
// ========== 策略初始化接口 ==========
ROBOTSTRATEGYDLL_EXPORT void __cdecl InitializeStrategy(void* strategy,
    int ourScore, int oppScore,
    int remainingTime,
    int isFirstHalf,
    int isOurKickoff) {
    if (!strategy) return;

    MatchContext context;
    context.ourScore = ourScore;
    context.oppScore = oppScore;
    context.remainingTime = remainingTime;
    context.isFirstHalf = (isFirstHalf != 0);
    context.isOurKickoff = (isOurKickoff != 0);
    context.ourRobotsCount = 5;
    context.oppRobotsCount = 5;

    StrategyInitializer::initializeStrategy(static_cast<UnifiedStrategy*>(strategy), context);
}

// 获取当前策略模式
ROBOTSTRATEGYDLL_EXPORT int __cdecl GetCurrentStrategyMode(void* strategy) {
    (void)strategy;
    return static_cast<int>(StrategySelector::getInstance().getCurrentConfig().mode);
}

// 动态切换策略（注意：比赛开始后调用可能无效）
ROBOTSTRATEGYDLL_EXPORT void __cdecl SetStrategyMode(void* strategy, int mode) {
    if (!strategy) return;

    StrategySelector& selector = StrategySelector::getInstance();

    // 如果已锁定，不能切换
    if (selector.isLocked()) {
        std::cout << "Strategy is locked, cannot switch mode!" << std::endl;
        return;
    }

    StrategyConfig config = selector.generateConfig(static_cast<StrategyMode>(mode));
    selector.applyConfig(static_cast<UnifiedStrategy*>(strategy), config);
}