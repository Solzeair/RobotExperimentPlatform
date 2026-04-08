// StrategyInitializer.cpp
// 策略初始化器实现

#include "StrategyInitializer.h"
#include <iostream>
#include <fstream>
#include <sstream>

bool StrategyInitializer::initializeStrategy(UnifiedStrategy* strategy, const MatchContext& context) {
    if (!strategy) {
        std::cerr << "Error: Strategy is null!" << std::endl;
        return false;
    }

    StrategySelector& selector = StrategySelector::getInstance();

    // 1. 根据场景选择策略模式
    StrategyMode mode = selector.selectMode(context);

    // 2. 生成策略配置
    StrategyConfig config = selector.generateConfig(mode);

    // 3. 应用配置到策略
    selector.applyConfig(strategy, config);

    // 4. 锁定策略（比赛开始后不能变）
    selector.setLocked(true);

    // 5. 设置基础参数
    strategy->setOurGoalOnRight(!context.isFirstHalf);  // 下半场交换半场
    strategy->setOurKickoff(context.isOurKickoff);

    // 6. 输出策略信息
    printStrategyInfo(config);

    return true;
}

bool StrategyInitializer::initializeFromFile(UnifiedStrategy* strategy, const std::string& configFilePath) {
    if (!strategy) return false;

    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        std::cerr << "Warning: Cannot open config file, using default" << std::endl;
        MatchContext defaultContext = createDefaultContext();
        return initializeStrategy(strategy, defaultContext);
    }

    MatchContext context;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;

        if (line.find("ourScore") != std::string::npos) {
            iss >> key >> context.ourScore;
        }
        else if (line.find("oppScore") != std::string::npos) {
            iss >> key >> context.oppScore;
        }
        else if (line.find("remainingTime") != std::string::npos) {
            iss >> key >> context.remainingTime;
        }
        else if (line.find("isFirstHalf") != std::string::npos) {
            int val; iss >> key >> val; context.isFirstHalf = (val != 0);
        }
        else if (line.find("isOurKickoff") != std::string::npos) {
            int val; iss >> key >> val; context.isOurKickoff = (val != 0);
        }
    }

    file.close();
    return initializeStrategy(strategy, context);
}

MatchContext StrategyInitializer::createDefaultContext() {
    MatchContext context;
    context.ourScore = 0;
    context.oppScore = 0;
    context.remainingTime = 300;  // 5分钟
    context.isFirstHalf = true;
    context.isOurKickoff = true;
    context.isHomeGame = true;
    return context;
}

void StrategyInitializer::printStrategyInfo(const StrategyConfig& config) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "   Strategy Initialized" << std::endl;
    std::cout << "========================================" << std::endl;

    std::string modeName;
    switch (config.mode) {
    case StrategyMode::MODE_AGGRESSIVE: modeName = "激进进攻"; break;
    case StrategyMode::MODE_BALANCED: modeName = "平衡"; break;
    case StrategyMode::MODE_DEFENSIVE: modeName = "保守防守"; break;
    case StrategyMode::MODE_COUNTER_ATTACK: modeName = "防守反击"; break;
    }

    std::cout << "Mode: " << modeName << std::endl;
    std::cout << "Attack Aggression: " << config.attackAggression << std::endl;
    std::cout << "Defense Depth: " << config.defenseDepth << std::endl;
    std::cout << "Pressing Intensity: " << config.pressingIntensity << std::endl;
    std::cout << "Use Offside Trap: " << (config.useOffsideTrap ? "Yes" : "No") << std::endl;
    std::cout << "Use Zonal Defense: " << (config.useZonalDefense ? "Yes" : "No") << std::endl;
    std::cout << "========================================\n" << std::endl;
}