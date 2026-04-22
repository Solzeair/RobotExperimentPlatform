/*
* 采色对话框头文件
* 写作人 李青
* 功能 采色界面的类定义，包含HSI阈值调节滑块、颜色测试按钮和色环显示等属性和方法
* 未完成
*/
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QPainter>
#include <QColor>
#include <cmath>
#include <QTimer>
#include <QElapsedTimer>

class ColorDlg : public QWidget
{
    Q_OBJECT

public:
    ColorDlg(QWidget *parent = nullptr); // 初始化采色界面
    ~ColorDlg();
    static ColorDlg* getInstance(); // 获取单例实例
    const int(*getHSIThreshold())[6] { return HSIThreshold; } // 获取HSI阈值

private slots:
    void onButtonColorTest();             // 颜色测试按钮
    void onButtonRunTest();               // 动态测试按钮
    void onButtonStopTest();              // 单帧图像按钮
    void onButtonSave();                  // 保存按钮
    void onButtonLoad();                  // 加载按钮
    void onSegCheckBoxStateChanged(int);  // 图像分割复选框状态改变
    void onScrollBarChanged();            // 各项滚动条值改变

private:
    void initUI();                        // UI初始化
    void UpdateHSIThreshold();            // 更新HSI阈值
    void drawHSIRing();                   // 绘制HSI颜色环

private:
    // --- 布局部件 ---
    QVBoxLayout* mainLayout;              // 主垂直布局容器指针

    // --- 控件部件 ---
    QLabel* localLabel;                   // 局部放大区域显示标签
    QLabel* HSIdisplayLabel;              // HSI颜色环显示标签
    QPushButton* stopButton;              // 单帧图像按钮
    QPushButton* colorTestButton;         // 颜色测试按钮
    QPushButton* runTestButton;           // 动态测试按钮
    QPushButton* colorLoadButton;         // 加载按钮
    QPushButton* colorSaveButton;         // 保存按钮
    QCheckBox* segCheckBox;               // 图像分割复选框

    // 滚动条
    QScrollBar* scrollBarHMin;            // 色调下限调节滚动条
    QScrollBar* scrollBarSMin;            // 饱和度下限调节滚动条
    QScrollBar* scrollBarIMin;            // 亮度下限调节滚动条

    // --- 数据变量 ---
    bool m_SelectRect;                    // 是否处于矩形选择状态
    int m_H_High;                         // 色调上限
    int m_H_Low;                          // 色调下限
    int m_S_High;                         // 饱和度上限
    int m_S_Low;                          // 饱和度下限
    int m_I_High;                         // 亮度上限
    int m_I_Low;                          // 亮度下限
    int m_object;                         // 当前操作对象的标识
    bool m_ImageSeg;                      // 是否开启图像分割功能
    bool m_isSaved;                       // 参数是否已保存
    int HSIThreshold[8][6];               // 各颜色HSI阈值
    QVector<QPoint> m_vecColorSet;        // 采样颜色点集合
    int yi[255];                          // 特定映射数值
    

};