// StrategySelector.h
// 策略选择器 - 根据比赛场景自动选择最优策略


#ifndef STRATEGYSELECTOR_H
#define STRATEGYSELECTOR_H

#include "UnifiedStrategy.h"  // 包含策略类定义
#include "StrategyContext.h"  // 包含类型定义（MatchContext, StrategyMode, StrategyConfig）

/**
 * 策略选择器类（单例模式）
 * 负责根据比赛上下文自动选择策略模式并生成配置
 */
class StrategySelector {
public:
    /**
     * 获取单例实例
     * @return StrategySelector引用
     */
    static StrategySelector& getInstance() {
        static StrategySelector instance;  // 线程安全的静态局部变量
        return instance;
    }

    /**
     * 根据比赛上下文选择策略模式
     * @param context 比赛上下文信息
     * @return 选中的策略模式
     */
    StrategyMode selectMode(const MatchContext& context);

    /**
     * 根据策略模式生成详细配置
     * @param mode 策略模式
     * @return 策略配置结构体
     */
    StrategyConfig generateConfig(StrategyMode mode);

    /**
     * 应用配置到策略实例
     * @param strategy 策略实例指针
     * @param config 策略配置
     */
    void applyConfig(UnifiedStrategy* strategy, const StrategyConfig& config);

    /**
     * 获取当前策略配置
     * @return 当前配置的常量引用
     */
    const StrategyConfig& getCurrentConfig() const { return m_currentConfig; }

    /**
     * 设置策略是否已锁定（比赛开始后不能变）
     * @param locked true=锁定，false=未锁定
     */
    void setLocked(bool locked) { m_isLocked = locked; }

    /**
     * 检查策略是否已锁定
     * @return true=已锁定，false=未锁定
     */
    bool isLocked() const { return m_isLocked; }

private:
    // 私有构造函数（单例模式）
    StrategySelector() : m_isLocked(false) {}

    // 禁止拷贝和赋值
    StrategySelector(const StrategySelector&) = delete;
    StrategySelector& operator=(const StrategySelector&) = delete;

    StrategyConfig m_currentConfig;  // 当前生效的策略配置
    bool m_isLocked;                  // 锁定标志（比赛开始后锁定，不能切换）
};

#endif // STRATEGYSELECTOR_H