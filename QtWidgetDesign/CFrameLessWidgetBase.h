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
    // 默认没有父对象，explicit防止隐式类型转换
    explicit CFrameLessWidgetBase(QWidget* parent = nullptr); 
    virtual ~CFrameLessWidgetBase();

    // 留给子窗口调用的接口
    // 修改标题栏显示的文本内容
    void setWindowTitleText(const QString& text); 
    // 子类自定义容器的 Widget 类
    void setCentralWidget(QWidget* widget);

protected:
    // 拦截窗口关闭事件，实现弹窗二次确认
    void closeEvent(QCloseEvent* event);
    // 状态变化事件：监听窗口最大化/还原状态以切换按钮图标
    void changeEvent(QEvent* event) override;
    // 核心事件拦截：处理拖拽和边缘缩放
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

    // --- 核心内容区（子类在此区域布局） --- 
    QWidget* m_pCentralWidget = nullptr;    // 声明中心内容区容器的指针，留给子类作为父控件挂载业务UI，初始化为空

private slots:
    // 统一处理标题栏所有按钮（最小化/最大化/关闭）点击事件的槽函数
    void onTitleBarButtonClicked();

private:
    // 声明UI初始化函数，用于构建自定义标题栏和中心容器的布局
    void initBaseUI();
    // 声明UI更新函数，根据窗口是否最大化来切换最大化/还原按钮的图标
    void updateMaximizeButtonState(bool isMaximized);

private:
    // --- 标题栏部件 ---
    QWidget* m_pTitleBarWidget = nullptr;   // 声明标题栏整体容器部件的指针，并初始化为空
    QLabel* m_pLogo = nullptr;              // 声明用于显示Logo的标签控件指针，并初始化为空
    QLabel* m_pTitleTextLabel = nullptr;    // 声明用于显示窗口标题文本的标签控件指针，并初始化为空
    QPushButton* m_pSetBtn = nullptr;       // 声明设置按钮控件指针，并初始化为空
    QPushButton* m_pMinBtn = nullptr;       // 声明最小化按钮控件指针，并初始化为空
    QPushButton* m_pMaxBtn = nullptr;       // 声明最大化/向下还原按钮控件指针，并初始化为空
    QPushButton* m_pCloseBtn = nullptr;     // 声明关闭按钮控件指针，并初始化为空

};