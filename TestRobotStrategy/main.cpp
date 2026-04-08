// main.cpp
// 机器人策略DLL测试程序 - 加载策略DLL并提供参数调优GUI界面
// 功能：
//   1. 动态加载RobotStrategyDll.dll
//   2. 获取并调用策略接口函数
//   3. 创建参数调优对话框，支持运行时参数调整
//   4. 执行一次策略决策测试

#include <QApplication>
#include <QLibrary>
#include <QDebug>
#include <iostream>
#include <iomanip>
#include "ParameterDialog.h"

// ==================== 数据结构定义 ====================
// 注意：这些定义必须与DLL中的定义完全一致，确保内存布局兼容

/**
 * 二维点结构体
 * 与DLL中的GeometryUtils::Point保持一致
 */
struct Point {
    double x, y;  // 点的x坐标和y坐标
    Point(double x = 0, double y = 0) : x(x), y(y) {}
};

/**
 * 机器人位姿结构体
 * 与DLL中的GeometryUtils::RobotPose保持一致
 */
struct RobotPose {
    double x, y, theta;   // 机器人位置(x,y)和朝向角theta(弧度)
    double vx, vy;        // 机器人在x和y方向上的速度
    double vtheta;        // 机器人的角速度

    RobotPose(double x = 0, double y = 0, double theta = 0,
              double vx = 0, double vy = 0, double vtheta = 0)
        : x(x), y(y), theta(theta), vx(vx), vy(vy), vtheta(vtheta) {}
};

/**
 * 球信息结构体
 * 与DLL中的BallInfo保持一致
 */
struct BallInfo {
    Point pos;          // 球的当前位置
    double vel_x;       // 球在x方向的速度分量
    double vel_y;       // 球在y方向的速度分量
    double velocity;    // 球的合速度大小
    double angle;       // 球的运动方向角
    Point predictPos;   // 预测的未来位置

    BallInfo() : vel_x(0), vel_y(0), velocity(0), angle(0) {}
};

/**
 * 轮速结构体
 * 与DLL中的WheelVelocity保持一致
 */
struct WheelVelocity {
    double left;   // 左轮速度，正值向前
    double right;  // 右轮速度，正值向前

    WheelVelocity(double l = 0, double r = 0) : left(l), right(r) {}
};

// ==================== 函数指针类型定义 ====================

// 创建策略实例
// 参数: type - 策略类型(0=默认策略)
// 返回: 策略实例指针
typedef void* (*CreateStrategyFunc)(int);

// 销毁策略实例
// 参数: strategy - 策略实例指针
typedef void (*DestroyStrategyFunc)(void*);

// 决策函数 - 核心接口
// 参数: strategy - 策略实例指针
//       robots - 我方机器人位姿数组(5个)
//       oppRobots - 对方机器人位置数组(5个)
//       ball - 球的信息
//       velocities - 输出轮速数组(5个)
typedef void (*DecideFunc)(void*, const RobotPose*, const Point*,
                           const BallInfo*, WheelVelocity*);

// 重置策略状态
typedef void (*ResetFunc)(void*);

// 设置参数
// 参数: strategy - 策略实例指针
//       key - 参数名
//       value - 参数值
typedef void (*SetParameterFunc)(void*, const char*, double);

// 获取参数
// 参数: strategy - 策略实例指针
//       key - 参数名
// 返回: 参数值
typedef double (*GetParameterFunc)(void*, const char*);

// 获取策略名称
typedef const char* (*GetStrategyNameFunc)();

// 设置我方球门方向
// 参数: strategy - 策略实例指针
//       onRight - true:球门在右侧, false:球门在左侧
typedef void (*SetOurGoalOnRightFunc)(void*, bool);

// 设置我方开球状态
// 参数: strategy - 策略实例指针
//       isOurKickoff - true:我方开球, false:对方开球
typedef void (*SetOurKickoffFunc)(void*, bool);

// 保存配置到文件
// 参数: strategy - 策略实例指针
//       filename - 文件名
// 返回: true-成功, false-失败
typedef bool (*SaveConfigFunc)(void*, const char*);

// 从文件加载配置
// 参数: strategy - 策略实例指针
//       filename - 文件名
// 返回: true-成功, false-失败
typedef bool (*LoadConfigFunc)(void*, const char*);

/**
 * 主函数
 * 程序入口点，执行以下步骤：
 * 1. 加载策略DLL
 * 2. 解析DLL中的函数
 * 3. 创建策略实例
 * 4. 设置默认参数
 * 5. 创建并显示参数调优对话框
 * 6. 执行决策测试
 * 7. 等待用户交互
 * 8. 清理资源
 */
int main(int argc, char *argv[]) {
    // 创建Qt应用程序实例
    QApplication a(argc, argv);

    // 打印启动信息
    std::cout << "\n========================================" << std::endl;
    std::cout << "   Robot Strategy DLL Test with GUI" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // ==================== 加载DLL ====================
    // 注意：需要根据实际编译路径修改DLL路径
    QLibrary lib("C:/Users/2dou/Desktop/RobotStrategyDll/build/Desktop_Qt_6_10_2_MinGW_64_bit-Debug/debug/RobotStrategyDll.dll");

    if (!lib.load()) {
        std::cerr << "Failed to load DLL: " << lib.errorString().toStdString() << std::endl;
        return -1;
    }

    std::cout << "DLL loaded successfully!" << std::endl;

    // ==================== 解析函数 ====================
    // 获取DLL导出的函数指针
    auto createStrategy = reinterpret_cast<CreateStrategyFunc>(lib.resolve("CreateStrategy"));
    auto destroyStrategy = reinterpret_cast<DestroyStrategyFunc>(lib.resolve("DestroyStrategy"));
    auto decide = reinterpret_cast<DecideFunc>(lib.resolve("decide"));
    auto setParameter = reinterpret_cast<SetParameterFunc>(lib.resolve("setParameter"));
    auto getParameter = reinterpret_cast<GetParameterFunc>(lib.resolve("getParameter"));
    auto setOurGoalOnRight = reinterpret_cast<SetOurGoalOnRightFunc>(lib.resolve("setOurGoalOnRight"));
    auto setOurKickoff = reinterpret_cast<SetOurKickoffFunc>(lib.resolve("setOurKickoff"));
    auto saveConfig = reinterpret_cast<SaveConfigFunc>(lib.resolve("saveConfig"));
    auto loadConfig = reinterpret_cast<LoadConfigFunc>(lib.resolve("loadConfig"));

    // 检查关键函数是否解析成功
    if (!createStrategy || !destroyStrategy) {
        std::cerr << "Failed to resolve required functions!" << std::endl;
        return -1;
    }

    std::cout << "All functions resolved!" << std::endl;

    // ==================== 创建策略实例 ====================
    void* strategy = createStrategy(0);
    if (!strategy) {
        std::cerr << "Failed to create strategy!" << std::endl;
        return -1;
    }

    std::cout << "Strategy created!" << std::endl;

    // ==================== 配置策略 ====================
    // 设置我方球门在右侧
    if (setOurGoalOnRight) setOurGoalOnRight(strategy, true);
    // 设置对方开球
    if (setOurKickoff) setOurKickoff(strategy, false);

    // 设置默认控制参数
    if (setParameter) {
        setParameter(strategy, "max_speed", 75.0);   // 最大速度75cm/s
        setParameter(strategy, "kp_pos", 12.0);      // 位置比例系数
        setParameter(strategy, "kp_angle", 22.0);    // 角度比例系数
        std::cout << "Default parameters set" << std::endl;
    }

    // ==================== 创建参数对话框 ====================
    ParameterDialog dialog;

    // 设置DLL函数指针，供对话框调用
    if (setParameter) {
        dialog.setSetParameterFunc(strategy, setParameter);
    }
    if (getParameter) {
        dialog.setGetParameterFunc(strategy, getParameter);
    }
    if (saveConfig) {
        dialog.setSaveConfigFunc(strategy, saveConfig);
    }
    if (loadConfig) {
        dialog.setLoadConfigFunc(strategy, loadConfig);
    }
    dialog.setStrategy(strategy);

    // ==================== 准备测试数据 ====================
    // 5个机器人的初始位置
    // 机器人0: 中场偏右
    // 机器人1: 中场左下
    // 机器人2: 中场左上
    // 机器人3: 左中场
    // 机器人4: 守门员位置
    RobotPose robots[5] = {
        RobotPose(50, 90, 0),      // 机器人0 - 中场
        RobotPose(40, 70, 0),      // 机器人1 - 中下
        RobotPose(40, 110, 0),     // 机器人2 - 中上
        RobotPose(30, 50, 0),      // 机器人3 - 左中下
        RobotPose(8, 90, 1.57)     // 机器人4 - 守门员，面向右
    };

    // 5个对方机器人的位置
    Point oppRobots[5] = {
        Point(150, 90),   // 对手0 - 中场
        Point(160, 70),   // 对手1 - 中下
        Point(160, 110),  // 对手2 - 中上
        Point(170, 50),   // 对手3 - 右中下
        Point(170, 130)   // 对手4 - 右中上
    };

    // 球的信息：位于对方半场，向右运动
    BallInfo ball;
    ball.pos = Point(150, 90);   // 球在对方半场中路
    ball.vel_x = 10;             // 向右运动
    ball.vel_y = 0;              // 垂直方向无运动
    ball.velocity = 10;          // 速度大小10cm/s

    WheelVelocity velocities[5]; // 输出轮速数组

    // ==================== 执行一次决策测试 ====================
    if (decide) {
        std::cout << "\n执行初始决策测试..." << std::endl;
        decide(strategy, robots, oppRobots, &ball, velocities);
        std::cout << "决策结果:" << std::endl;
        for (int i = 0; i < 5; i++) {
            std::cout << "  机器人" << i << ": L=" << std::setw(6)
                << velocities[i].left << ", R=" << std::setw(6)
                << velocities[i].right << std::endl;
        }
    }

    // ==================== 显示参数调优对话框 ====================
    std::cout << "\n打开参数调优窗口..." << std::endl;

    // 设置对话框为独立窗口（非模态）
    dialog.setWindowFlags(Qt::Window);
    dialog.resize(800, 600);   // 设置窗口大小
    dialog.show();              // 显示对话框

    std::cout << "对话框已显示，等待用户交互..." << std::endl;

    // ==================== 运行Qt事件循环 ====================
    // 用户关闭对话框后程序继续执行
    int result = a.exec();

    // ==================== 清理资源 ====================
    destroyStrategy(strategy);  // 销毁策略实例
    lib.unload();               // 卸载DLL

    return result;
}
