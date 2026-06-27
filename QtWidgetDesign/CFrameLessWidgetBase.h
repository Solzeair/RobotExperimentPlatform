#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox> 
#include <QWindowStateChangeEvent>

class CFrameLessWidgetBase : public QWidget
{
    Q_OBJECT

public:
    // explicit 防止隐式构造；默认无父对象，作为独立顶层窗口
    explicit CFrameLessWidgetBase(QWidget* parent = nullptr); 
    virtual ~CFrameLessWidgetBase();

    // 暴露给子类的定制接口
    // 设置标题栏显示文本
    void setWindowTitleText(const QString& text); 
    // 替换中心内容区，供子类挂载业务 UI
    void setCentralWidget(QWidget* widget);

protected:
    // 关闭前弹出二次确认，避免误关闭
    void closeEvent(QCloseEvent* event);
    // 监听最大化/还原以同步切换标题栏按钮图标
    void changeEvent(QEvent* event) override;
    // 在原生消息层处理标题栏拖拽与边缘缩放，实现无原生边框窗口的可拖拽缩放
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

    // --- 核心内容区：子类在此区域布局 ---
    QWidget* m_pCentralWidget = nullptr;    // 中心内容区容器，子类以此为父控件挂载业务 UI

private slots:
    // 统一接收标题栏各按钮点击，按来源分发处理
    void onTitleBarButtonClicked();

private:
    // 构建自定义标题栏与中心容器布局
    void initBaseUI();
    // 依窗口状态切换最大化/还原按钮图标
    void updateMaximizeButtonState(bool isMaximized);

private:
    // --- 标题栏部件 ---
    QWidget* m_pTitleBarWidget = nullptr;   // 标题栏容器
    QLabel* m_pLogo = nullptr;              // Logo 标签
    QLabel* m_pTitleTextLabel = nullptr;    // 标题文本标签
    QPushButton* m_pSetBtn = nullptr;       // 设置按钮
    QPushButton* m_pMinBtn = nullptr;       // 最小化按钮
    QPushButton* m_pMaxBtn = nullptr;       // 最大化/还原按钮
    QPushButton* m_pCloseBtn = nullptr;     // 关闭按钮

};