/*
* 插件管理器头文件
* 功能 管理标定和采色插件的加载、调用和释放
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
    
    // 加载插件
    bool loadPlugin(const QString& pluginPath, PluginType type);
    
    // 获取插件接口
    PluginInterface* getPlugin(PluginType type);
    
    // 创建插件界面
    QWidget* createPluginWidget(PluginType type, QWidget* parent = nullptr);
    
    // 设置DisplayDlg指针给插件
    void setDisplayDlgForPlugins(DisplayDlg* dlg);
    
    // 释放所有插件
    void releaseAllPlugins();
    
    // 检查插件是否已加载
    bool isPluginLoaded(PluginType type) const;

private:
    PluginManager();
    ~PluginManager();
    
    // 禁止拷贝和赋值
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    
    static PluginManager* m_instance;
    
    // 插件库映射
    QMap<PluginType, QLibrary*> m_libraries;
    
    // 插件接口映射
    QMap<PluginType, PluginInterface*> m_plugins;
};
