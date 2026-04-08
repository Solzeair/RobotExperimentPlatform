// StrategyInitializer.h
// 策略初始化器 - 在比赛开始前初始化策略

#ifndef STRATEGYINITIALIZER_H
#define STRATEGYINITIALIZER_H

#include "UnifiedStrategy.h"
#include "StrategySelector.h"
#include <string>

class StrategyInitializer {
public:
    /**
     * 初始化策略（比赛开始前调用一次）
     * @param strategy 策略实例指针
     * @param context 比赛场景信息
     * @return 是否初始化成功
     */
    static bool initializeStrategy(UnifiedStrategy* strategy, const MatchContext& context);

    /**
     * 从UI配置文件初始化策略
     * @param strategy 策略实例指针
     * @param configFilePath 配置文件路径
     * @return 是否初始化成功
     */
    static bool initializeFromFile(UnifiedStrategy* strategy, const std::string& configFilePath);

    /**
     * 创建默认的MatchContext（用于测试）
     */
    static MatchContext createDefaultContext();

private:
    static void printStrategyInfo(const StrategyConfig& config);
};

#endif // STRATEGYINITIALIZER_H
