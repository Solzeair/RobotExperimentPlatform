// robotstrategydll.cpp
#include "robotstrategydll.h"
#include "StrategyFactory.h"
#include "StrategySelector.h"
#include "StrategyInitializer.h"
#include <iostream>

// 全局策略实例存储（如果需要）
static void* g_currentStrategy = nullptr;

extern "C" {

    ROBOTSTRATEGYDLL_EXPORT void* CreateStrategy(int type) {
        std::cout << "CreateStrategy called, type: " << type << std::endl;
        void* strategy = static_cast<void*>(createStrategy(static_cast<StrategyType>(type)));
        g_currentStrategy = strategy;
        return strategy;
    }

    ROBOTSTRATEGYDLL_EXPORT void DestroyStrategy(void* strategy) {
        std::cout << "DestroyStrategy called" << std::endl;
        if (strategy) {
            destroyStrategy(static_cast<StrategyBase*>(strategy));
            g_currentStrategy = nullptr;
        }
    }

    ROBOTSTRATEGYDLL_EXPORT void decide(void* strategy, const RobotPose* robots,
        const Point* oppRobots, const BallInfo* ball,
        WheelVelocity* velocities) {
        if (strategy && robots && oppRobots && ball && velocities) {
            static_cast<UnifiedStrategy*>(strategy)->decide(robots, oppRobots, *ball, velocities);
        }
    }

    ROBOTSTRATEGYDLL_EXPORT void reset(void* strategy) {
        if (strategy) {
            static_cast<UnifiedStrategy*>(strategy)->reset();
        }
    }

    ROBOTSTRATEGYDLL_EXPORT void setParameter(void* strategy, const char* key, double value) {
        if (strategy && key) {
            static_cast<UnifiedStrategy*>(strategy)->setParameter(std::string(key), value);
        }
    }

    ROBOTSTRATEGYDLL_EXPORT double getParameter(void* strategy, const char* key) {
        if (strategy && key) {
            std::string keyStr(key);
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

    ROBOTSTRATEGYDLL_EXPORT const char* GetStrategyName(int type) {
        (void)type;
        return "Unified MFC Strategy";
    }

    ROBOTSTRATEGYDLL_EXPORT void setOurGoalOnRight(void* strategy, bool onRight) {
        if (strategy) {
            static_cast<UnifiedStrategy*>(strategy)->setOurGoalOnRight(onRight);
        }
    }

    ROBOTSTRATEGYDLL_EXPORT void setOurKickoff(void* strategy, bool isOurKickoff) {
        if (strategy) {
            static_cast<UnifiedStrategy*>(strategy)->setOurKickoff(isOurKickoff);
        }
    }

    ROBOTSTRATEGYDLL_EXPORT int TestFunction() {
        return 42;
    }

    // ========== 策略初始化接口 ==========
    ROBOTSTRATEGYDLL_EXPORT void InitializeStrategy(void* strategy,
        int ourScore, int oppScore,
        int remainingTime,
        int isFirstHalf,
        int isOurKickoff) {
        if (!strategy) {
            std::cerr << "InitializeStrategy: strategy is null!" << std::endl;
            return;
        }

        std::cout << "InitializeStrategy called:" << std::endl;
        std::cout << "  ourScore: " << ourScore << ", oppScore: " << oppScore << std::endl;
        std::cout << "  remainingTime: " << remainingTime << "s" << std::endl;
        std::cout << "  isFirstHalf: " << (isFirstHalf ? "Yes" : "No") << std::endl;
        std::cout << "  isOurKickoff: " << (isOurKickoff ? "Yes" : "No") << std::endl;

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

    ROBOTSTRATEGYDLL_EXPORT int GetCurrentStrategyMode(void* strategy) {
        (void)strategy;
        return static_cast<int>(StrategySelector::getInstance().getCurrentConfig().mode);
    }

    ROBOTSTRATEGYDLL_EXPORT void SetStrategyMode(void* strategy, int mode) {
        if (!strategy) return;

        StrategySelector& selector = StrategySelector::getInstance();

        if (selector.isLocked()) {
            std::cout << "Strategy is locked, cannot switch mode!" << std::endl;
            return;
        }

        StrategyConfig config = selector.generateConfig(static_cast<StrategyMode>(mode));
        selector.applyConfig(static_cast<UnifiedStrategy*>(strategy), config);
        std::cout << "Strategy mode changed to: " << mode << std::endl;
    }

} // extern "C"