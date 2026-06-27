/*
* 插件接口定义文件
* 定义标定、采色、策略等插件的统一契约，
* 使宿主程序可在不感知具体实现的情况下动态加载并管理各功能插件。
*/
#pragma once

#include <QWidget>
#include <QString>

class DisplayDlg;

// 插件类型枚举，用于宿主按类别路由加载与界面管理
enum class PluginType {
    DEMARCATE,   // 标定插件
    COLOR,       // 采色插件
    STRATEGY     // 策略插件
};

// 插件接口类，所有插件须实现该抽象接口，由宿主通过虚函数统一调度生命周期与界面创建
class PluginInterface
{
public:
    virtual ~PluginInterface() {}
    
    // 返回插件唯一标识名称，供宿主区分与日志记录
    virtual QString getName() const = 0;
    
    // 返回插件类别，宿主据此决定加载时机与界面归属
    virtual PluginType getType() const = 0;
    
    // 按需创建并返回插件主界面，所有权由插件内部管理
    virtual QWidget* createWidget(QWidget* parent) = 0;
    
    // 注入宿主 DisplayDlg 指针，供插件回调宿主的显示与交互能力
    virtual void setDisplayDlg(DisplayDlg* dlg) = 0;
    
    // 初始化插件运行所需的资源，返回 false 表示加载失败
    virtual bool initialize() = 0;
    
    // 卸载时释放插件占用的资源，确保无残留引用
    virtual void release() = 0;
};

// 动态库导出的插件创建/销毁函数签名，宿主通过这些符号在运行时解析并加载插件
typedef PluginInterface* (*CreatePluginFunc)();
typedef void (*DestroyPluginFunc)(PluginInterface*);

#define PLUGIN_INTERFACE_IID "com.xsyu.robot.PluginInterface/1.0"

Q_DECLARE_INTERFACE(PluginInterface, PLUGIN_INTERFACE_IID)