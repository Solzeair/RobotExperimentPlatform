/*
* 策略插件接口定义文件
* 功能 定义策略插件的统一接口，与标定采色插件保持一致
*/
#pragma once

#include "PluginInterface.h"
#include <QString>

// 前向声明
class DisplayDlg;

// 数据结构定义（与原策略接口保持一致）
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

// 策略插件接口类 - 继承通用插件接口
class StrategyPluginInterface : public PluginInterface
{
public:
    virtual ~StrategyPluginInterface() {}
    
    // ======== 继承自 PluginInterface ========
    // 获取插件名称（已在基类声明）
    // 获取插件类型（已在基类声明）
    // 创建插件界面（已在基类声明）
    // 设置DisplayDlg指针（已在基类声明）
    // 初始化插件（已在基类声明，无参数版本）
    // 释放插件资源（已在基类声明）
    
    // ======== 策略插件特有方法 ========
    
    // 带参数的初始化方法（策略插件专用）
    virtual void initialize(int param) = 0;
    
    // 设置参数
    virtual void setParameter(const char* name, double value) = 0;
    
    // 获取参数
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
    
    // 保存配置
    virtual bool saveConfig(const char* path) = 0;
    
    // 加载配置
    virtual bool loadConfig(const char* path) = 0;
};

#define STRATEGY_PLUGIN_INTERFACE_IID "com.xsyu.robot.StrategyPluginInterface/1.0"

Q_DECLARE_INTERFACE(StrategyPluginInterface, STRATEGY_PLUGIN_INTERFACE_IID)