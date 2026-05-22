// StrategyContext.h
// 策略上下文统一类型定义文件

#ifndef STRATEGYCONTEXT_H
#define STRATEGYCONTEXT_H

/**
 * 比赛上下文信息结构体
 * 包含比赛开始前的所有关键信息，用于策略初始化
 */
struct MatchContext {
    int ourScore;           // 我方得分
    int oppScore;           // 对方得分
    int remainingTime;      // 剩余时间（秒）
    bool isFirstHalf;       // 是否上半场（true=上半场，false=下半场）
    bool isOurKickoff;      // 是否我方开球（true=我方开球，false=对方开球）
    int ourRobotsCount;     // 我方机器人数量（通常为5）
    int oppRobotsCount;     // 对方机器人数量（通常为5）

    /**
     * 默认构造函数
     * 提供合理的默认值
     */
    MatchContext()
        : ourScore(0)
        , oppScore(0)
        , remainingTime(0)
        , isFirstHalf(true)
        , isOurKickoff(false)
        , ourRobotsCount(5)
        , oppRobotsCount(5) {
    }
};

/**
 * 策略模式枚举
 * 定义策略的不同行为模式
 */
enum class StrategyMode {
    NORMAL = 0,      // 正常模式 - 平衡攻守，适合大多数情况
    DEFENSIVE = 1,   // 防守模式 - 侧重防守，减少冒险，保护领先优势
    AGGRESSIVE = 2,  // 进攻模式 - 积极进攻，高风险高回报，适合落后时使用
    CONSERVATIVE = 3 // 保守模式 - 稳妥为主，控制节奏，消耗时间
};

/**
 * 策略配置结构体
 * 包含策略的具体参数值，用于控制机器人的行为
 * 注意：字段名称必须与 UnifiedStrategy 类中的成员变量名匹配
 */
struct StrategyConfig {
    StrategyMode mode;              // 策略模式（决定整体行为倾向）
    double attackAggression;        // 进攻侵略性 (0-1) - 越高越激进
    double defenseDepth;            // 防守深度 (0-1) - 越高防守越深
    double pressingIntensity;       // 压迫强度 (0-1) - 越高压迫越强
    bool useOffsideTrap;            // 是否使用越位陷阱
    bool useZonalDefense;           // 是否使用区域防守

    /**
     * 默认构造函数
     * 默认使用正常模式，中等参数
     */
    StrategyConfig()
        : mode(StrategyMode::NORMAL)
        , attackAggression(0.6)      // 默认进攻侵略性 60%
        , defenseDepth(0.5)          // 默认防守深度 50%
        , pressingIntensity(0.5)     // 默认压迫强度 50%
        , useOffsideTrap(false)      // 默认不使用越位陷阱
        , useZonalDefense(true) {    // 默认使用区域防守
    }

    /**
     * 完整参数构造函数
     * @param m 策略模式
     * @param attack 进攻侵略性 (0-1)
     * @param defense 防守深度 (0-1)
     * @param pressing 压迫强度 (0-1)
     * @param offsideTrap 是否使用越位陷阱
     * @param zonalDefense 是否使用区域防守
     */
    StrategyConfig(StrategyMode m, double attack, double defense, double pressing,
        bool offsideTrap = false, bool zonalDefense = true)
        : mode(m)
        , attackAggression(attack)
        , defenseDepth(defense)
        , pressingIntensity(pressing)
        , useOffsideTrap(offsideTrap)
        , useZonalDefense(zonalDefense) {
    }
};

#endif // STRATEGYCONTEXT_H