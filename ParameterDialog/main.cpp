// main.cpp
// 机器人策略DLL测试程序 - 加载策略DLL并提供参数调优GUI界面

#include <QApplication>
#include <QLibrary>
#include <QDebug>
#include <iostream>
#include <iomanip>
#include "ParameterDialog.h"
#include <windows.h>
// ==================== 数据结构定义（必须与DLL完全一致）====================

// 二维点结构体
struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
};

// 机器人位姿结构体
struct RobotPose {
    double x, y, theta;
    double vx, vy;
    double vtheta;
    RobotPose(double x = 0, double y = 0, double theta = 0,
        double vx = 0, double vy = 0, double vtheta = 0)
        : x(x), y(y), theta(theta), vx(vx), vy(vy), vtheta(vtheta) {
    }
};

// 球信息结构体
struct BallInfo {
    Point pos;
    double vel_x;
    double vel_y;
    double velocity;
    double angle;
    Point predictPos;
    BallInfo() : vel_x(0), vel_y(0), velocity(0), angle(0) {}
};

// 轮速结构体
struct WheelVelocity {
    double left;
    double right;
    WheelVelocity(double l = 0, double r = 0) : left(l), right(r) {}
};

// ==================== 函数指针类型定义 ====================

typedef void* (*CreateStrategyFunc)(int);
typedef void (*DestroyStrategyFunc)(void*);
typedef void (*DecideFunc)(void*, const RobotPose*, const Point*, const BallInfo*, WheelVelocity*);
typedef void (*ResetFunc)(void*);
typedef void (*SetParameterFunc)(void*, const char*, double);
typedef double (*GetParameterFunc)(void*, const char*);
typedef const char* (*GetStrategyNameFunc)();
typedef void (*SetOurGoalOnRightFunc)(void*, bool);
typedef void (*SetOurKickoffFunc)(void*, bool);
typedef bool (*SaveConfigFunc)(void*, const char*);
typedef bool (*LoadConfigFunc)(void*, const char*);
typedef void (*InitializeStrategyFunc)(void*, int, int, int, int, int);
typedef int (*GetCurrentStrategyModeFunc)(void*);

// 全局函数指针
static CreateStrategyFunc pCreateStrategy = nullptr;
static DestroyStrategyFunc pDestroyStrategy = nullptr;
static DecideFunc pDecide = nullptr;
static SetParameterFunc pSetParameter = nullptr;
static GetParameterFunc pGetParameter = nullptr;
static SetOurGoalOnRightFunc pSetOurGoalOnRight = nullptr;
static SetOurKickoffFunc pSetOurKickoff = nullptr;
static SaveConfigFunc pSaveConfig = nullptr;
static LoadConfigFunc pLoadConfig = nullptr;
static InitializeStrategyFunc pInitializeStrategy = nullptr;

// 全局策略句柄
static void* g_strategy = nullptr;

// ==================== DLL加载和函数解析 ====================

bool loadDLLFunctions(QLibrary& lib) {
    pCreateStrategy = reinterpret_cast<CreateStrategyFunc>(lib.resolve("CreateStrategy"));
    pDestroyStrategy = reinterpret_cast<DestroyStrategyFunc>(lib.resolve("DestroyStrategy"));
    pDecide = reinterpret_cast<DecideFunc>(lib.resolve("decide"));
    pSetParameter = reinterpret_cast<SetParameterFunc>(lib.resolve("setParameter"));
    pGetParameter = reinterpret_cast<GetParameterFunc>(lib.resolve("getParameter"));
    pSetOurGoalOnRight = reinterpret_cast<SetOurGoalOnRightFunc>(lib.resolve("setOurGoalOnRight"));
    pSetOurKickoff = reinterpret_cast<SetOurKickoffFunc>(lib.resolve("setOurKickoff"));
    pSaveConfig = reinterpret_cast<SaveConfigFunc>(lib.resolve("saveConfig"));
    pLoadConfig = reinterpret_cast<LoadConfigFunc>(lib.resolve("loadConfig"));
    pInitializeStrategy = reinterpret_cast<InitializeStrategyFunc>(lib.resolve("InitializeStrategy"));

    if (!pCreateStrategy || !pDestroyStrategy) {
        std::cerr << "Failed to resolve required functions!" << std::endl;
        return false;
    }

    std::cout << "All functions resolved!" << std::endl;
    return true;
}

// ==================== 初始化比赛策略 ====================

void initMatchStrategy(int ourScore, int oppScore, int remainingTime,
    int isFirstHalf, int isOurKickoff) {
    if (pInitializeStrategy && g_strategy) {
        std::cout << "\n========== Initializing Strategy ==========" << std::endl;
        std::cout << "Score: " << ourScore << " - " << oppScore << std::endl;
        std::cout << "Time Left: " << remainingTime << "s" << std::endl;
        std::cout << "First Half: " << (isFirstHalf ? "Yes" : "No") << std::endl;
        std::cout << "Our Kickoff: " << (isOurKickoff ? "Yes" : "No") << std::endl;

        pInitializeStrategy(g_strategy, ourScore, oppScore, remainingTime,
            isFirstHalf, isOurKickoff);
        std::cout << "===========================================\n" << std::endl;
    }
    else {
        std::cout << "Warning: InitializeStrategy not available, using default" << std::endl;
        // 使用默认参数
        if (pSetOurGoalOnRight) pSetOurGoalOnRight(g_strategy, true);
        if (pSetOurKickoff) pSetOurKickoff(g_strategy, isOurKickoff != 0);
    }
}
/**
 * 主函数
 */
int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8); // 强制控制台用UTF-8编码
    SetConsoleCP(CP_UTF8);       // 同时解决输入乱码
    QApplication a(argc, argv);

    std::cout << "\n========================================" << std::endl;
    std::cout << "   Robot Strategy DLL Test with GUI" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // ==================== 加载DLL ====================
    QLibrary lib("RobotStrategyDll.dll");

    if (!lib.load()) {
        std::cerr << "Failed to load DLL: " << lib.errorString().toLocal8Bit().constData() << std::endl;
        return -1;
    }

    std::cout << "DLL loaded successfully!" << std::endl;

    // ==================== 解析函数 ====================
    if (!loadDLLFunctions(lib)) {
        return -1;
    }

    // ==================== 创建策略实例 ====================
    g_strategy = pCreateStrategy(0);
    if (!g_strategy) {
        std::cerr << "Failed to create strategy!" << std::endl;
        return -1;
    }
    std::cout << "Strategy created!" << std::endl;

    // ==================== 根据比赛场景初始化策略 ====================
    // 比赛开始前调用一次，根据场景选择进攻/防守模式
    initMatchStrategy(
        0,      // 我方比分
        0,      // 对方比分
        300,    // 剩余时间（秒），5分钟
        1,      // 上半场（1=上半场，0=下半场）
        1       // 我方开球（1=我方开球，0=对方开球）
    );

    // ==================== 设置默认控制参数 ====================
    if (pSetParameter) {
        pSetParameter(g_strategy, "max_speed", 75.0);
        pSetParameter(g_strategy, "kp_pos", 12.0);
        pSetParameter(g_strategy, "kp_angle", 22.0);
        std::cout << "Default parameters set" << std::endl;
    }
    if (pGetParameter && g_strategy) {
        // 从策略读取当前值，更新到对话框的参数列表
        double attack = pGetParameter(g_strategy, "attack_aggression");
        double defense = pGetParameter(g_strategy, "defense_depth");
        double pressing = pGetParameter(g_strategy, "pressing_intensity");
        std::cout << "Current strategy values: attack=" << attack
            << ", defense=" << defense
            << ", pressing=" << pressing << std::endl;
    }

    // ==================== 创建参数对话框 ====================
    ParameterDialog dialog;
    if (pSetParameter) {
        dialog.setSetParameterFunc(g_strategy, pSetParameter);
    }
    if (pGetParameter) {
        dialog.setGetParameterFunc(g_strategy, pGetParameter);
    }
    if (pSaveConfig) {
        dialog.setSaveConfigFunc(g_strategy, pSaveConfig);
    }
    if (pLoadConfig) {
        dialog.setLoadConfigFunc(g_strategy, pLoadConfig);
    }
    dialog.setStrategy(g_strategy);

    // ==================== 准备测试数据 ====================
    RobotPose robots[5] = {
        RobotPose(50, 90, 0),
        RobotPose(40, 70, 0),
        RobotPose(40, 110, 0),
        RobotPose(30, 50, 0),
        RobotPose(8, 90, 1.57)
    };

    Point oppRobots[5] = {
        Point(150, 90), Point(160, 70), Point(160, 110),
        Point(170, 50), Point(170, 130)
    };

    BallInfo ball;
    ball.pos = Point(150, 90);
    ball.vel_x = 10;
    ball.vel_y = 0;
    ball.velocity = 10;

    WheelVelocity velocities[5];

    // ==================== 执行决策测试 ====================
    if (pDecide) {
        std::cout << "\n执行初始决策测试..." << std::endl;
        pDecide(g_strategy, robots, oppRobots, &ball, velocities);
        std::cout << "决策结果:" << std::endl;
        for (int i = 0; i < 5; i++) {
            std::cout << "  机器人" << i << ": L=" << std::setw(6)
                << velocities[i].left << ", R=" << std::setw(6)
                << velocities[i].right << std::endl;
        }
    }

    // ==================== 显示参数调优对话框 ====================
    std::cout << "\n打开参数调优窗口..." << std::endl;
    dialog.setWindowFlags(Qt::Window);
    dialog.resize(800, 600);
    dialog.show();

    std::cout << "对话框已显示，等待用户交互..." << std::endl;

    // ==================== 运行Qt事件循环 ====================
    int result = a.exec();

    // ==================== 清理资源 ====================
    pDestroyStrategy(g_strategy);
    lib.unload();

    return result;
}