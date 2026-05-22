// StrategyFactory.cpp
// 策略工厂实现

#include "strategyfactory.h"
#include "UnifiedStrategy.h"
#include "StrategyContext.h"
#include "StrategyInitializer.h"
#include "StrategySelector.h"
#include <cstring>
#include <iostream>

// 全局策略实例
static UnifiedStrategy* g_strategy = nullptr;

// ========== 所有函数只在 extern "C" 块内定义一次 ==========
extern "C" {
    ROBOTSTRATEGYDLL_API void __cdecl SetMatchState(void* strategy, int state) {
        if (strategy) {
            static_cast<UnifiedStrategy*>(strategy)->setMatchState(state);
        }
    }

    ROBOTSTRATEGYDLL_API void __cdecl SetFormationType(void* strategy, int isSingleDefender) {
        if (strategy) {
            static_cast<UnifiedStrategy*>(strategy)->setFormationType(isSingleDefender != 0);
        }
    }

    ROBOTSTRATEGYDLL_API void __cdecl SetKickoffType(void* strategy, int type) {
        if (strategy) {
            static_cast<UnifiedStrategy*>(strategy)->setKickoffType(type);
        }
    }

    ROBOTSTRATEGYDLL_API void __cdecl SetPenaltyKickMode(void* strategy, int shootType, int goaliePos) {
        if (strategy) {
            static_cast<UnifiedStrategy*>(strategy)->setPenaltyKickMode(shootType, goaliePos);
        }
    }

    ROBOTSTRATEGYDLL_API void __cdecl SelectStrategy(void* strategy, int strategyIndex) {
        if (strategy) {
            static_cast<UnifiedStrategy*>(strategy)->selectStrategy(strategyIndex);
        }
    }

    ROBOTSTRATEGYDLL_API void __cdecl ParkRobots(void* strategy) {
        if (strategy) {
            static_cast<UnifiedStrategy*>(strategy)->parkRobots();
        }
    }

    ROBOTSTRATEGYDLL_API void* __cdecl CreateStrategy(int type) {
        (void)type;
        std::cout << "CreateStrategy called" << std::endl;
        g_strategy = new UnifiedStrategy();
        return g_strategy;
    }

    ROBOTSTRATEGYDLL_API void __cdecl DestroyStrategy(void* strategy) {
        std::cout << "DestroyStrategy called" << std::endl;
        if (strategy) {
            delete static_cast<UnifiedStrategy*>(strategy);
            g_strategy = nullptr;
        }
    }

    ROBOTSTRATEGYDLL_API void __cdecl decide(void* strategy, const RobotPose* robots,
        const Point* oppRobots, const BallInfo* ball,
        WheelVelocity* velocities) {
        if (strategy && robots && oppRobots && ball && velocities) {
            static_cast<UnifiedStrategy*>(strategy)->decide(robots, oppRobots, *ball, velocities);
        }
    }

    ROBOTSTRATEGYDLL_API void __cdecl reset(void* strategy) {
        if (strategy) {
            static_cast<UnifiedStrategy*>(strategy)->reset();
        }
    }

    ROBOTSTRATEGYDLL_API void __cdecl setParameter(void* strategy, const char* key, double value) {
        if (strategy && key) {
            std::cout << "setParameter called: " << key << " = " << value << std::endl;
            UnifiedStrategy* pStrategy = static_cast<UnifiedStrategy*>(strategy);
            pStrategy->setParameter(std::string(key), value);
        }
    }

    ROBOTSTRATEGYDLL_API double __cdecl getParameter(void* strategy, const char* key) {
        if (!strategy || !key) return 0;

        UnifiedStrategy* pStrategy = static_cast<UnifiedStrategy*>(strategy);
        return pStrategy->getParameter(std::string(key));
    }

    ROBOTSTRATEGYDLL_API const char* __cdecl GetStrategyName(int type) {
        (void)type;
        return "Unified MFC Strategy";
    }

    ROBOTSTRATEGYDLL_API void __cdecl setOurGoalOnRight(void* strategy, bool onRight) {
        if (strategy) {
            static_cast<UnifiedStrategy*>(strategy)->setOurGoalOnRight(onRight);
        }
    }

    ROBOTSTRATEGYDLL_API void __cdecl setOurKickoff(void* strategy, bool isOurKickoff) {
        if (strategy) {
            static_cast<UnifiedStrategy*>(strategy)->setOurKickoff(isOurKickoff);
        }
    }

    ROBOTSTRATEGYDLL_API int __cdecl TestFunction() {
        return 42;
    }

    ROBOTSTRATEGYDLL_API void __cdecl InitializeStrategy(void* strategy,
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

    ROBOTSTRATEGYDLL_API int __cdecl GetCurrentStrategyMode(void* strategy) {
        (void)strategy;
        StrategyMode currentMode = StrategySelector::getInstance().getCurrentConfig().mode;
        return static_cast<int>(currentMode);
    }

    ROBOTSTRATEGYDLL_API void __cdecl SetStrategyMode(void* strategy, int mode) {
        if (!strategy) return;

        StrategySelector& selector = StrategySelector::getInstance();

        if (selector.isLocked()) {
            std::cout << "Strategy is locked, cannot switch mode!" << std::endl;
            return;
        }

        StrategyMode strategyMode;
        switch (mode) {
        case 0: strategyMode = StrategyMode::NORMAL; break;
        case 1: strategyMode = StrategyMode::DEFENSIVE; break;
        case 2: strategyMode = StrategyMode::AGGRESSIVE; break;
        case 3: strategyMode = StrategyMode::CONSERVATIVE; break;
        default: strategyMode = StrategyMode::NORMAL; break;
        }

        StrategyConfig config = selector.generateConfig(strategyMode);
        selector.applyConfig(static_cast<UnifiedStrategy*>(strategy), config);
    }

}  // extern "C"

// ========== 注意：不要在这里重复定义 createStrategy 和 destroyStrategy ==========
// 它们已经在 strategyfactory.h 中作为 inline 函数定义了
// 删除下面的重复定义：
/*
StrategyBase* createStrategy(StrategyType type) {
    return static_cast<StrategyBase*>(CreateStrategy(static_cast<int>(type)));
}

void destroyStrategy(StrategyBase* strategy) {
    DestroyStrategy(strategy);
}
*/