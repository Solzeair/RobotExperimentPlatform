/*
* 场地标定对话框头文件
* 写作人 李青
* 功能 标定界面的类，包含标定操作控制按钮、进度条及场地边界点等属性。
* 已完成 / 未完成
*/
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QMessageBox>
#include <QThread>
#include <QCoreApplication>

class DemarcateDlg : public QWidget
{
    Q_OBJECT

public:
    DemarcateDlg(QWidget *parent = nullptr); // 初始化标定界面和边界点
    ~DemarcateDlg();

private slots:
    void onButtonSet();         // 开始标定按钮
    void onButtonResetOne();    // 撤销一步按钮
    void onButtonReset();       // 重新标定按钮
    void onButtonLoad();        // 加载标定按钮
    void onButtonSave();        // 保存标定按钮
    void onButtonFlush();       // 刷新图像按钮
    void onButtonShowRes();     // 查看标定结果按钮

private:
    void initUI();              // UI初始化

private:
    // --- 布局部件 ---
    QVBoxLayout* mainLayout;                            // 主垂直布局容器

    // --- 控件部件 ---
    QLabel* resultLabel;                                // 标定结果显示标签
    QProgressBar* progressBar;                          // 标定进度条控件
    QPushButton* btnSet;                                // 开始标定按钮
    QPushButton* btnResetOne;                           // 撤销单步标定操作按钮
    QPushButton* btnReset;                              // 重置整个标定操作按钮
    QPushButton* btnLoad;                               // 加载外部标定数据按钮
    QPushButton* btnSave;                               // 保存当前标定数据按钮
    QPushButton* btnFlush;                              // 刷新当前画面按钮
    QPushButton* btnShowRes;                            // 显示最终标定结果按钮

    // --- 数据变量 ---
    bool m_isSaved;                                     // 标定参数是否已保存
    bool m_needResetDC;                                 // 是否需要重置设备上下文(DC)的状态
    QVector<QPoint> m_points;                           // 用户手动标定点集合

    // 场地边界点
    QPoint point[13];                                   // 预定义场地固定边界坐标
};