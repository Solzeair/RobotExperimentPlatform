/*
* 插件管理器实现：负责动态插件的加载、调用与释放，单例管理
*/
#include "PluginManager.h"
#include "DisplayDlg.h"
#include <QDebug>

PluginManager* PluginManager::m_instance = nullptr;

PluginManager* PluginManager::getInstance()
{
    if (!m_instance) {
        m_instance = new PluginManager();
    }
    return m_instance;
}

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
    releaseAllPlugins();
}

bool PluginManager::loadPlugin(const QString& pluginPath, PluginType type)
{
    // 同一类型仅保留一个插件，加载新插件前需清空已有实例，避免资源泄漏
    if (m_libraries.contains(type)) {
        releaseAllPlugins();
    }
    
    QLibrary* library = new QLibrary(pluginPath);
    if (!library->load()) {
        qDebug() << "Failed to load plugin:" << library->errorString();
        delete library;
        return false;
    }
    
    // 插件必须导出名为 createPlugin 的工厂函数，作为动态库与宿主的统一入口约定
    CreatePluginFunc createFunc = reinterpret_cast<CreatePluginFunc>(library->resolve("createPlugin"));
    if (!createFunc) {
        qDebug() << "Failed to resolve createPlugin function";
        library->unload();
        delete library;
        return false;
    }
    
    PluginInterface* plugin = createFunc();
    if (!plugin) {
        qDebug() << "Failed to create plugin instance";
        library->unload();
        delete library;
        return false;
    }
    
    if (!plugin->initialize()) {
        qDebug() << "Failed to initialize plugin";
        delete plugin;
        library->unload();
        delete library;
        return false;
    }
    
    // 同时持有插件实例与动态库句柄，确保释放时能成对卸载
    m_libraries[type] = library;
    m_plugins[type] = plugin;
    
    qDebug() << "Plugin loaded successfully:" << plugin->getName();
    return true;
}

PluginInterface* PluginManager::getPlugin(PluginType type)
{
    if (m_plugins.contains(type)) {
        return m_plugins[type];
    }
    return nullptr;
}

QWidget* PluginManager::createPluginWidget(PluginType type, QWidget* parent)
{
    PluginInterface* plugin = getPlugin(type);
    if (plugin) {
        return plugin->createWidget(parent);
    }
    return nullptr;
}

void PluginManager::setDisplayDlgForPlugins(DisplayDlg* dlg)
{
    for (auto plugin : m_plugins.values()) {
        plugin->setDisplayDlg(dlg);
    }
}

void PluginManager::releaseAllPlugins()
{
    for (auto plugin : m_plugins.values()) {
        plugin->release();
        delete plugin;
    }
    m_plugins.clear();
    
    for (auto library : m_libraries.values()) {
        library->unload();
        delete library;
    }
    m_libraries.clear();
}

bool PluginManager::isPluginLoaded(PluginType type) const
{
    return m_plugins.contains(type);
}
