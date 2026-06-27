/*
* 策略插件接口定义
* 定义宿主与策略插件间的统一契约，结构对标标定采色插件，支持策略以可插拔方式接入
*/
#pragma once

#include "PluginInterface.h"
#include <QString>

class DisplayDlg;

// 策略交互所用的基础数据结构，字段含义与原策略接口对齐以保证跨插件兼容
struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
};

struct RobotPose {
    double x, y, theta;
    double vx, vy;
    double vtheta;
    RobotPose(double x = 0, double y = 0, double theta = 0,
        double vx = 0, double vy = 0, double vtheta = 0)
        : x(x), y(y), theta(theta), vx(vx), vy(vy), vtheta(vtheta) {}
};

struct BallInfo {
    Point pos;
    double vel_x, vel_y;
    double velocity;
    double angle;
    Point predictPos;
    BallInfo() : vel_x(0), vel_y(0), velocity(0), angle(0) {}
};

struct WheelVelocity {
    double left, right;
    WheelVelocity(double l = 0, double r = 0) : left(l), right(r) {}
};

// 策略插件契约：在通用插件接口基础上扩展比赛场景配置与策略调度方法
class StrategyPluginInterface : public PluginInterface
{
public:
    virtual ~StrategyPluginInterface() {}

    // ======== 继承自 PluginInterface ========
    // 名称/类型查询、界面创建、DisplayDlg 绑定、初始化、资源释放等通用能力由基类提供

    // ======== 策略插件特有方法 ========

    // 比赛开始前按参数完成策略内部初始化
    virtual void initialize(int param) = 0;

    // 运行期读写可调参数，使策略行为可在不重编译的前提下动态调整
    virtual void setParameter(const char* name, double value) = 0;

    virtual double getParameter(const char* name) = 0;

    // 设置阵型类型
    virtual void setFormationType(int type) = 0;

    // 设置球门方向
    virtual void setOurGoalOnRight(bool isRight) = 0;

    // 设置开球方
    virtual void setOurKickoff(bool isOurKickoff) = 0;

    // 设置开球类型
    virtual void setKickoffType(int type) = 0;

    // 设置点球模式
    virtual void setPenaltyKickMode(int direct, int mode) = 0;

    // 选择策略
    virtual void selectStrategy(int strategyNum) = 0;

    // 机器人归位
    virtual void parkRobots() = 0;

    // 配置持久化，保存/恢复策略运行所需的参数与阵型设定
    virtual bool saveConfig(const char* path) = 0;

    virtual bool loadConfig(const char* path) = 0;
};

#define STRATEGY_PLUGIN_INTERFACE_IID "com.xsyu.robot.StrategyPluginInterface/1.0"

Q_DECLARE_INTERFACE(StrategyPluginInterface, STRATEGY_PLUGIN_INTERFACE_IID)