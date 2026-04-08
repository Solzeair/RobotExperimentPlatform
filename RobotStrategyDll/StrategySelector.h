// StrategySelector.h
// 策略选择器 - 根据比赛场景选择进攻或防守策略

#ifndef STRATEGYSELECTOR_H
#define STRATEGYSELECTOR_H

#include "UnifiedStrategy.h"
#include "StrategyBase.h"

// 策略模式枚举
enum class StrategyMode {
    MODE_AGGRESSIVE,    // 激进进攻模式
    MODE_BALANCED,      // 平衡模式（默认）
    MODE_DEFENSIVE,     // 保守防守模式
    MODE_COUNTER_ATTACK // 防守反击模式
};

// 比赛场景信息结构体
struct MatchContext {
    int ourScore;           // 我方比分
    int oppScore;           // 对方比分
    int remainingTime;      // 剩余时间（秒）
    bool isFirstHalf;       // 是否上半场
    bool isOurKickoff;      // 是否我方开球
    bool isHomeGame;        // 是否主场
    int ourRobotsCount;     // 我方机器人数量（5）
    int oppRobotsCount;     // 对方机器人数量（5）

    MatchContext() : ourScore(0), oppScore(0), remainingTime(300),
        isFirstHalf(true), isOurKickoff(true),
        isHomeGame(true), ourRobotsCount(5), oppRobotsCount(5) {
    }
};

// 策略配置
struct StrategyConfig {
    StrategyMode mode;              // 策略模式
    double attackAggression;        // 进攻侵略性 (0-1)
    double defenseDepth;            // 防守深度 (0-1)
    double pressingIntensity;       // 压迫强度 (0-1)
    int formationBase;              // 队形基准区域
    bool useOffsideTrap;            // 是否使用越位陷阱
    bool useZonalDefense;           // 是否使用区域防守

    StrategyConfig() : mode(StrategyMode::MODE_BALANCED),
        attackAggression(0.6), defenseDepth(0.5),
        pressingIntensity(0.5), formationBase(8),
        useOffsideTrap(false), useZonalDefense(true) {
    }
};

class StrategySelector {
public:
    static StrategySelector& getInstance() {
        static StrategySelector instance;
        return instance;
    }

    // 根据比赛场景选择策略模式（比赛开始前调用一次）
    StrategyMode selectMode(const MatchContext& context);

    // 根据选择的模式生成策略配置
    StrategyConfig generateConfig(StrategyMode mode);

    // 应用配置到策略实例
    void applyConfig(UnifiedStrategy* strategy, const StrategyConfig& config);

    // 获取当前策略配置
    const StrategyConfig& getCurrentConfig() const { return m_currentConfig; }

    // 设置策略是否已锁定（比赛开始后不能变）
    void setLocked(bool locked) { m_isLocked = locked; }
    bool isLocked() const { return m_isLocked; }

private:
    StrategySelector() : m_isLocked(false) {}
    ~StrategySelector() {}
    StrategySelector(const StrategySelector&) = delete;
    StrategySelector& operator=(const StrategySelector&) = delete;

    StrategyConfig m_currentConfig;
    bool m_isLocked;
};

#endif // STRATEGYSELECTOR_H