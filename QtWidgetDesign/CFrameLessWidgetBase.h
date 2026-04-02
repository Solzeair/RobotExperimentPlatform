#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class CFrameLessWidgetBase : public QWidget
{
    Q_OBJECT

public:
    explicit CFrameLessWidgetBase(QWidget* parent = nullptr);
    virtual ~CFrameLessWidgetBase();

    // 留给子窗口调用的接口
    void setWindowTitleText(const QString& text);
    QWidget* getCentralWidget() const { return m_pCentralWidget; }

protected:
    // 核心事件拦截：处理拖拽和边缘缩放
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    // 状态拦截：处理最大化/还原时的图标切换
    bool event(QEvent* event) override;

signals:
    // 如果子窗口想拦截关闭事件（比如弹窗确认），可以连接这个信号
    void sig_close();

private slots:
    void onTitleBarButtonClicked();

private:
    void initBaseUI();
    void updateMaximizeButtonState(bool isMaximized);

private:
    int m_nBorderWidth = 5;

    // --- 标题栏部件 ---
    QWidget* m_pTitleBarWidget = nullptr;
    QLabel* m_pLogo = nullptr;
    QLabel* m_pTitleTextLabel = nullptr;
    QPushButton* m_pSetBtn = nullptr;
    QPushButton* m_pMinBtn = nullptr;
    QPushButton* m_pMaxBtn = nullptr;
    QPushButton* m_pCloseBtn = nullptr;

    // --- 核心内容区（子类在此区域布局） ---
    QWidget* m_pCentralWidget = nullptr;
};