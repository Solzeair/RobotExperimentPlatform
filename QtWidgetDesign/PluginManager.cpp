/*
* 插件管理器实现文件
* 功能 实现插件的加载、调用和释放逻辑
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
    // 如果已加载该类型的插件，先释放
    if (m_libraries.contains(type)) {
        releaseAllPlugins();
    }
    
    // 加载插件库
    QLibrary* library = new QLibrary(pluginPath);
    if (!library->load()) {
        qDebug() << "Failed to load plugin:" << library->errorString();
        delete library;
        return false;
    }
    
    // 解析插件创建函数
    CreatePluginFunc createFunc = reinterpret_cast<CreatePluginFunc>(library->resolve("createPlugin"));
    if (!createFunc) {
        qDebug() << "Failed to resolve createPlugin function";
        library->unload();
        delete library;
        return false;
    }
    
    // 创建插件实例
    PluginInterface* plugin = createFunc();
    if (!plugin) {
        qDebug() << "Failed to create plugin instance";
        library->unload();
        delete library;
        return false;
    }
    
    // 初始化插件
    if (!plugin->initialize()) {
        qDebug() << "Failed to initialize plugin";
        delete plugin;
        library->unload();
        delete library;
        return false;
    }
    
    // 保存插件和库
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
