// StrategyInitializer.h
// 策略初始化器 - 在比赛开始前初始化策略

#ifndef STRATEGYINITIALIZER_H
#define STRATEGYINITIALIZER_H

#include "UnifiedStrategy.h"   // 包含策略类
#include "StrategySelector.h"  // 包含策略选择器
#include "StrategyContext.h"   // 包含类型定义（已包含MatchContext）
#include <string>

/**
 * 策略初始化器类（静态类）
 * 负责在比赛开始前初始化策略系统
 */
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
     * 从配置文件初始化策略
     * @param strategy 策略实例指针
     * @param configFilePath 配置文件路径
     * @return 是否初始化成功
     */
    static bool initializeFromFile(UnifiedStrategy* strategy, const std::string& configFilePath);

    /**
     * 创建默认的MatchContext（用于测试）
     * @return 默认的比赛上下文
     */
    static MatchContext createDefaultContext();

private:
    /**
     * 打印策略初始化信息（调试用）
     * @param config 策略配置
     */
    static void printStrategyInfo(const StrategyConfig& config);
};

#endif // STRATEGYINITIALIZER_H