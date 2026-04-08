// StrategySelector.cpp
#include "StrategySelector.h"
#include <iostream>
#include <algorithm>

StrategyMode StrategySelector::selectMode(const MatchContext& context) {
    // 如果已经锁定，返回当前模式
    if (m_isLocked) {
        return m_currentConfig.mode;
    }

    std::cout << "\n========== Strategy Selection ==========" << std::endl;
    std::cout << "Score: " << context.ourScore << " - " << context.oppScore << std::endl;
    std::cout << "Time Left: " << context.remainingTime << "s" << std::endl;
    std::cout << "First Half: " << (context.isFirstHalf ? "Yes" : "No") << std::endl;
    std::cout << "Our Kickoff: " << (context.isOurKickoff ? "Yes" : "No") << std::endl;

    StrategyMode selectedMode;

    // ========== 上半场判断 ==========
    if (context.isFirstHalf) {
        if (context.isOurKickoff) {
            selectedMode = StrategyMode::MODE_AGGRESSIVE;
            std::cout << ">>> First half, our kickoff -> AGGRESSIVE MODE" << std::endl;
        }
        else {
            selectedMode = StrategyMode::MODE_BALANCED;
            std::cout << ">>> First half, opp kickoff -> BALANCED MODE" << std::endl;
        }
        return selectedMode;
    }

    // ========== 下半场判断 ==========
    int scoreDiff = context.ourScore - context.oppScore;
    int timeLeft = context.remainingTime;

    // 领先2球及以上
    if (scoreDiff >= 2) {
        selectedMode = StrategyMode::MODE_DEFENSIVE;
        std::cout << ">>> Leading by 2+ goals -> DEFENSIVE MODE" << std::endl;
    }
    // 领先1球
    else if (scoreDiff == 1) {
        if (timeLeft < 60) {
            selectedMode = StrategyMode::MODE_DEFENSIVE;
            std::cout << ">>> Leading by 1 goal, last minute -> DEFENSIVE MODE" << std::endl;
        }
        else {
            selectedMode = StrategyMode::MODE_BALANCED;
            std::cout << ">>> Leading by 1 goal, time充裕 -> BALANCED MODE" << std::endl;
        }
    }
    // 落后
    else if (scoreDiff <= -1) {
        selectedMode = StrategyMode::MODE_AGGRESSIVE;
        std::cout << ">>> Losing -> AGGRESSIVE MODE" << std::endl;
    }
    // 平局
    else {
        if (timeLeft < 60) {
            selectedMode = StrategyMode::MODE_AGGRESSIVE;
            std::cout << ">>> Tie, last minute -> AGGRESSIVE MODE" << std::endl;
        }
        else {
            selectedMode = StrategyMode::MODE_BALANCED;
            std::cout << ">>> Tie -> BALANCED MODE" << std::endl;
        }
    }

    std::cout << "========================================\n" << std::endl;
    return selectedMode;
}

StrategyConfig StrategySelector::generateConfig(StrategyMode mode) {
    StrategyConfig config;
    config.mode = mode;

    switch (mode) {
    case StrategyMode::MODE_AGGRESSIVE:
        config.attackAggression = 0.85;
        config.defenseDepth = 0.3;
        config.pressingIntensity = 0.8;
        config.formationBase = 12;
        config.useOffsideTrap = true;
        config.useZonalDefense = false;
        break;

    case StrategyMode::MODE_BALANCED:
        config.attackAggression = 0.6;
        config.defenseDepth = 0.5;
        config.pressingIntensity = 0.5;
        config.formationBase = 8;
        config.useOffsideTrap = false;
        config.useZonalDefense = true;
        break;

    case StrategyMode::MODE_DEFENSIVE:
        config.attackAggression = 0.3;
        config.defenseDepth = 0.8;
        config.pressingIntensity = 0.3;
        config.formationBase = 4;
        config.useOffsideTrap = false;
        config.useZonalDefense = true;
        break;

    case StrategyMode::MODE_COUNTER_ATTACK:
        config.attackAggression = 0.5;
        config.defenseDepth = 0.7;
        config.pressingIntensity = 0.4;
        config.formationBase = 6;
        config.useOffsideTrap = false;
        config.useZonalDefense = true;
        break;
    }

    return config;
}

void StrategySelector::applyConfig(UnifiedStrategy* strategy, const StrategyConfig& config) {
    if (!strategy) return;

    std::cout << "Applying strategy config..." << std::endl;

    // 应用战术参数
    strategy->setParameter("attack_aggression", config.attackAggression);
    strategy->setParameter("defense_depth", config.defenseDepth);
    strategy->setParameter("pressing_intensity", config.pressingIntensity);
    strategy->setParameter("use_offside_trap", config.useOffsideTrap ? 1.0 : 0.0);
    strategy->setParameter("use_zonal_defense", config.useZonalDefense ? 1.0 : 0.0);

    m_currentConfig = config;

    std::cout << "Config applied: aggression=" << config.attackAggression
        << ", defenseDepth=" << config.defenseDepth << std::endl;
}