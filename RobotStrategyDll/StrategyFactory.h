// StrategyFactory.h
// 策略工厂 - 创建和管理策略实例

#ifndef STRATEGYFACTORY_H
#define STRATEGYFACTORY_H

#include "robotstrategyDll_global.h"
#include "StrategyBase.h"

// 策略类型枚举
enum class StrategyType {
    STRATEGY_ONE,
    STRATEGY_TWO,
    STRATEGY_THREE
};

#ifdef __cplusplus
extern "C" {
#endif

// ========== C接口（供DLL导出）==========
ROBOTSTRATEGYDLL_EXPORT void* CreateStrategy(int type);
ROBOTSTRATEGYDLL_EXPORT void DestroyStrategy(void* strategy);
ROBOTSTRATEGYDLL_EXPORT int TestFunction(void);
ROBOTSTRATEGYDLL_EXPORT const char* GetStrategyName(int type);
ROBOTSTRATEGYDLL_EXPORT void SetStrategyParameter(void* strategy, const char* key, double value);

#ifdef __cplusplus
}
#endif

// ========== C++接口 ==========
ROBOTSTRATEGYDLL_EXPORT StrategyBase* createStrategy(StrategyType type);
ROBOTSTRATEGYDLL_EXPORT void destroyStrategy(StrategyBase* strategy);

#endif
