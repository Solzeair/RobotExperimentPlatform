/*
* 插件接口定义文件
* 功能 定义标定和采色插件的统一接口
*/
#pragma once

#include <QWidget>
#include <QString>

class DisplayDlg;

// 插件类型枚举
enum class PluginType {
    DEMARCATE,   // 标定插件
    COLOR,       // 采色插件
    STRATEGY     // 策略插件
};

// 插件接口类
class PluginInterface
{
public:
    virtual ~PluginInterface() {}
    
    // 获取插件名称
    virtual QString getName() const = 0;
    
    // 获取插件类型
    virtual PluginType getType() const = 0;
    
    // 创建插件界面
    virtual QWidget* createWidget(QWidget* parent) = 0;
    
    // 设置DisplayDlg指针
    virtual void setDisplayDlg(DisplayDlg* dlg) = 0;
    
    // 初始化插件
    virtual bool initialize() = 0;
    
    // 释放插件资源
    virtual void release() = 0;
};

// 插件创建函数类型定义
typedef PluginInterface* (*CreatePluginFunc)();
typedef void (*DestroyPluginFunc)(PluginInterface*);

#define PLUGIN_INTERFACE_IID "com.xsyu.robot.PluginInterface/1.0"

Q_DECLARE_INTERFACE(PluginInterface, PLUGIN_INTERFACE_IID)