// StrategyInitializer.cpp
// 策略初始化器实现

#include "StrategyInitializer.h"
#include <iostream>
#include <fstream>
#include <sstream>

/**
 * 初始化策略
 * 流程：选择模式 -> 生成配置 -> 应用配置 -> 锁定策略
 */
bool StrategyInitializer::initializeStrategy(UnifiedStrategy* strategy, const MatchContext& context) {
    if (!strategy) {
        std::cerr << "Error: Strategy is null!" << std::endl;
        return false;
    }

    StrategySelector& selector = StrategySelector::getInstance();

    // 1. 根据比赛场景选择策略模式
    StrategyMode mode = selector.selectMode(context);

    // 2. 生成策略配置
    StrategyConfig config = selector.generateConfig(mode);

    // 3. 应用配置到策略实例
    selector.applyConfig(strategy, config);

    // 4. 锁定策略（比赛开始后不能变）
    selector.setLocked(true);

    // 5. 设置基础参数
    strategy->setOurGoalOnRight(!context.isFirstHalf);  // 下半场交换半场
    strategy->setOurKickoff(context.isOurKickoff);

    // 6. 输出策略信息（用于调试）
    printStrategyInfo(config);

    return true;
}

/**
 * 从配置文件初始化策略
 * 读取配置文件中的比赛上下文信息
 */
bool StrategyInitializer::initializeFromFile(UnifiedStrategy* strategy, const std::string& configFilePath) {
    if (!strategy) return false;

    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        std::cerr << "Warning: Cannot open config file: " << configFilePath
            << ", using default" << std::endl;
        MatchContext defaultContext = createDefaultContext();
        return initializeStrategy(strategy, defaultContext);
    }

    MatchContext context;
    std::string line;

    // 解析配置文件
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
            int val;
            iss >> key >> val;
            context.isFirstHalf = (val != 0);
        }
        else if (line.find("isOurKickoff") != std::string::npos) {
            int val;
            iss >> key >> val;
            context.isOurKickoff = (val != 0);
        }
    }

    file.close();
    return initializeStrategy(strategy, context);
}

/**
 * 创建默认的比赛上下文（用于测试）
 */
MatchContext StrategyInitializer::createDefaultContext() {
    MatchContext context;
    context.ourScore = 0;
    context.oppScore = 0;
    context.remainingTime = 300;  // 5分钟 = 300秒
    context.isFirstHalf = true;    // 上半场
    context.isOurKickoff = true;   // 我方开球
    context.ourRobotsCount = 5;    // 5个机器人
    context.oppRobotsCount = 5;
    return context;
}

/**
 * 打印策略初始化信息
 * 用于调试和确认策略配置
 */
void StrategyInitializer::printStrategyInfo(const StrategyConfig& config) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "   Strategy Initialized" << std::endl;
    std::cout << "========================================" << std::endl;

    std::string modeName;
    switch (config.mode) {
    case StrategyMode::AGGRESSIVE: modeName = "进攻模式"; break;
    case StrategyMode::NORMAL: modeName = "平衡模式"; break;
    case StrategyMode::DEFENSIVE: modeName = "防守模式"; break;
    case StrategyMode::CONSERVATIVE: modeName = "保守模式"; break;
    default: modeName = "未知模式"; break;
    }

    std::cout << "Strategy Mode: " << modeName << std::endl;
    std::cout << "Attack Aggression: " << config.attackAggression << " (0-1)" << std::endl;
    std::cout << "Defense Depth: " << config.defenseDepth << " (0-1)" << std::endl;
    std::cout << "Pressing Intensity: " << config.pressingIntensity << " (0-1)" << std::endl;
    std::cout << "========================================\n" << std::endl;
}