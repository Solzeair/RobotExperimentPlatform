/*
* 插件管理器
* 统一管理标定与采色插件的全生命周期：加载、接口调用与释放
*/
#pragma once

#include <QLibrary>
#include <QMap>
#include <memory>
#include "PluginInterface.h"

class PluginManager
{
public:
    static PluginManager* getInstance();
    
    // 动态加载指定路径的插件库并解析其导出接口
    bool loadPlugin(const QString& pluginPath, PluginType type);
    
    // 返回已加载插件的接口指针，供外部调用其功能
    PluginInterface* getPlugin(PluginType type);
    
    // 由插件创建其界面控件，parent 用于纳入 Qt 父子对象管理
    QWidget* createPluginWidget(PluginType type, QWidget* parent = nullptr);
    
    // 将显示对话框注入插件，使插件可向主界面回传数据
    void setDisplayDlgForPlugins(DisplayDlg* dlg);
    
    // 释放所有已加载插件占用的资源
    void releaseAllPlugins();
    
    // 判断某类型插件是否已成功加载
    bool isPluginLoaded(PluginType type) const;

private:
    PluginManager();
    ~PluginManager();
    
    // 禁止拷贝与赋值，保证单例唯一性
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    
    static PluginManager* m_instance;
    
    // 插件类型到动态库句柄的映射，用于后续释放
    QMap<PluginType, QLibrary*> m_libraries;
    
    // 插件类型到接口指针的映射，统一暴露给调用方
    QMap<PluginType, PluginInterface*> m_plugins;
};
