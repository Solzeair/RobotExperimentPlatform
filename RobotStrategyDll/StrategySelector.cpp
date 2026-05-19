// StrategySelector.cpp
// 策略选择器实现

#include "StrategySelector.h"
#include <iostream>
#include <algorithm>

/**
 * 根据比赛上下文选择策略模式
 * 决策逻辑：
 * - 上半场：开球时激进，否则平衡
 * - 下半场：根据比分差和时间剩余动态调整
 */
StrategyMode StrategySelector::selectMode(const MatchContext& context) {
    // 如果已经锁定，返回当前模式（比赛开始后不能改变）
    if (m_isLocked) {
        return m_currentConfig.mode;
    }

    // 输出调试信息
    std::cout << "\n========== Strategy Selection ==========" << std::endl;
    std::cout << "Score: " << context.ourScore << " - " << context.oppScore << std::endl;
    std::cout << "Time Left: " << context.remainingTime << "s" << std::endl;
    std::cout << "First Half: " << (context.isFirstHalf ? "Yes" : "No") << std::endl;
    std::cout << "Our Kickoff: " << (context.isOurKickoff ? "Yes" : "No") << std::endl;

    StrategyMode selectedMode;

    // ========== 上半场判断 ==========
    if (context.isFirstHalf) {
        // 上半场我方开球 -> 激进进攻，争取早期进球
        if (context.isOurKickoff) {
            selectedMode = StrategyMode::AGGRESSIVE;
            std::cout << ">>> First half, our kickoff -> AGGRESSIVE MODE" << std::endl;
        }
        // 上半场对方开球 -> 平衡模式，观察对方
        else {
            selectedMode = StrategyMode::NORMAL;
            std::cout << ">>> First half, opp kickoff -> NORMAL MODE" << std::endl;
        }
        return selectedMode;
    }

    // ========== 下半场判断 ==========
    int scoreDiff = context.ourScore - context.oppScore;  // 比分差
    int timeLeft = context.remainingTime;                 // 剩余时间

    // 领先2球及以上 -> 防守模式，保住胜果
    if (scoreDiff >= 2) {
        selectedMode = StrategyMode::DEFENSIVE;
        std::cout << ">>> Leading by 2+ goals -> DEFENSIVE MODE" << std::endl;
    }
    // 领先1球
    else if (scoreDiff == 1) {
        // 最后一分钟 -> 防守模式，全力防守
        if (timeLeft < 60) {
            selectedMode = StrategyMode::DEFENSIVE;
            std::cout << ">>> Leading by 1 goal, last minute -> DEFENSIVE MODE" << std::endl;
        }
        // 时间充裕 -> 平衡模式，稳妥为主
        else {
            selectedMode = StrategyMode::NORMAL;
            std::cout << ">>> Leading by 1 goal, time available -> NORMAL MODE" << std::endl;
        }
    }
    // 落后 -> 激进进攻，争取扳平/反超
    else if (scoreDiff <= -1) {
        selectedMode = StrategyMode::AGGRESSIVE;
        std::cout << ">>> Losing -> AGGRESSIVE MODE" << std::endl;
    }
    // 平局
    else {
        // 最后一分钟 -> 激进进攻，争取绝杀
        if (timeLeft < 60) {
            selectedMode = StrategyMode::AGGRESSIVE;
            std::cout << ">>> Tie, last minute -> AGGRESSIVE MODE" << std::endl;
        }
        // 时间充裕 -> 平衡模式
        else {
            selectedMode = StrategyMode::NORMAL;
            std::cout << ">>> Tie -> NORMAL MODE" << std::endl;
        }
    }

    std::cout << "========================================\n" << std::endl;
    return selectedMode;
}

/**
 * 根据策略模式生成详细配置
 * 为每种模式设置具体的参数值
 */
StrategyConfig StrategySelector::generateConfig(StrategyMode mode) {
    StrategyConfig config;
    config.mode = mode;

    switch (mode) {
    case StrategyMode::AGGRESSIVE:
        config.attackAggression = 0.85;      // 使用 attackAggression（不是 aggression）
        config.defenseDepth = 0.3;
        config.pressingIntensity = 0.8;      // 使用 pressingIntensity（不是 pressing）
        config.useOffsideTrap = true;
        config.useZonalDefense = false;
        break;

    case StrategyMode::NORMAL:
        config.attackAggression = 0.6;
        config.defenseDepth = 0.5;
        config.pressingIntensity = 0.5;
        config.useOffsideTrap = false;
        config.useZonalDefense = true;
        break;

    case StrategyMode::DEFENSIVE:
        config.attackAggression = 0.3;
        config.defenseDepth = 0.8;
        config.pressingIntensity = 0.3;
        config.useOffsideTrap = false;
        config.useZonalDefense = true;
        break;

    case StrategyMode::CONSERVATIVE:
        config.attackAggression = 0.4;
        config.defenseDepth = 0.6;
        config.pressingIntensity = 0.4;
        config.useOffsideTrap = false;
        config.useZonalDefense = true;
        break;
    }

    return config;
}

/**
 * 应用配置到策略实例
 * 将生成的配置参数设置到具体的策略对象中
 */
void StrategySelector::applyConfig(UnifiedStrategy* strategy, const StrategyConfig& config) {
    if (!strategy) {
        std::cerr << "Error: Cannot apply config to null strategy!" << std::endl;
        return;
    }

    std::cout << "Applying strategy config..." << std::endl;

    // 使用 attackAggression 
    strategy->setParameter("attack_aggression", config.attackAggression);

    // defenseDepth 字段名正确，保持不变
    strategy->setParameter("defense_depth", config.defenseDepth);

    // 使用 pressingIntensity 
    strategy->setParameter("pressing_intensity", config.pressingIntensity);

    // 应用越位陷阱设置
    strategy->setParameter("use_offside_trap", config.useOffsideTrap ? 1.0 : 0.0);

    // 应用区域防守设置
    strategy->setParameter("use_zonal_defense", config.useZonalDefense ? 1.0 : 0.0);

    // 保存当前配置
    m_currentConfig = config;

    // 输出调试信息
    std::cout << "Config applied: attackAggression=" << config.attackAggression
        << ", defenseDepth=" << config.defenseDepth
        << ", pressingIntensity=" << config.pressingIntensity
        << ", useOffsideTrap=" << (config.useOffsideTrap ? "true" : "false")
        << ", useZonalDefense=" << (config.useZonalDefense ? "true" : "false") << std::endl;
}