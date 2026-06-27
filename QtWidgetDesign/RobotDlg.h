/*
* 调车对话框头文件
* 功能：调车控制界面，包含机器人方向控制、通信频率与车辆编号配置相关的属性及方法。
*/

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QGroupBox>
#include <QTimer>

// 定时器标识与周期常量，用于周期性读取通讯模块状态
const int ID_TIMER_SHOW_FRE = 1002; // 频率轮询定时器ID
const int TIME_SPACE = 1000;        // 轮询周期，单位毫秒

class RobotDlg : public QWidget
{
    Q_OBJECT

public:
    RobotDlg(QWidget* parent = nullptr);  // 构造并初始化调车面板
    ~RobotDlg();

protected:
    void closeEvent(QCloseEvent* event) override;  // 关闭时清理状态并通知父窗口

private slots:
    // 方向控制槽：分别下发前进/后退/左转/右转/停止运动指令
    void onButtonFront();
    void onButtonBack();
    void onButtonLeft();
    void onButtonRight();
    void onButtonStop();
    // 编号与频率配置槽
    void onButtonChangeNum();             // 下发修改被控车编号命令
    void onButtonChangeFreq();            // 下发修改被控车通信频率命令
    void onRadio1450();                   // 被控车频率选为450
    void onRadio1460();                   // 被控车频率选为460
    void onButton450();                   // 发射器频率选为450
    void onButton460();                   // 发射器频率选为460
    void onButtonConfirmFreq();           // 确认并应用发射器频率设置
    void onTimer();                       // 定时轮询通讯模块状态

private:
    void initUI();                        // 构建并组装各功能分组控件

private:
    // --- 布局 ---
    QVBoxLayout* mainLayout;              // 对话框主垂直布局，承载各功能分组

    // --- 控件 ---
    QLineEdit* editOldNum;                // 展示修改前的小车编号
    QLineEdit* editNewNum;                // 输入要配置的新小车编号
    QLineEdit* editNum;                   // 输入待下发频段参数的被控车编号
    QLineEdit* editDeviceStatus;          // 反馈当前通讯模块连接状态
    QPushButton* btnChangeNum;            // 触发下发修改编号命令
    QPushButton* btnChangeFreq;           // 触发下发修改频率命令
    QPushButton* btnFront;                // 下发前进运动指令
    QPushButton* btnBack;                 // 下发后退运动指令
    QPushButton* btnLeft;                 // 下发左转运动指令
    QPushButton* btnRight;                // 下发右转运动指令
    QPushButton* btnStop;                 // 下发停止运动指令
    QRadioButton* radio1_450;             // 被控车频率选择450
    QRadioButton* radio1_460;             // 被控车频率选择460
    QPushButton* btn450;                  // 发射器频率选择450
    QPushButton* btn460;                  // 发射器频率选择460
    QPushButton* btnConfirmFreq;          // 确认发射器频率设置
    QGroupBox* carNumGroup;               // 小车编号配置区
    QGroupBox* controlGroup;              // 遥控运动控制区
    QGroupBox* carFreqGroup;              // 被控车频段配置区
    QGroupBox* deviceGroup;               // 发射器频率配置区
    QTimer* timer;                        // 通讯状态轮询定时器

    // --- 运行时状态 ---
    int m_oldNum;                         // 修改前的原小车编号
    int m_newNum;                         // 待配置的目标小车编号
    int m_numSet;                         // 当前等待下发频段参数的被控车编号
    bool m_carFre;                        // 被控车频段选择标志（区分450/460）
    int m_selectedFreq;                   // 发射器频率选择状态（450或460）
    const int speed = 20;                 // 运动指令下发的固定速度档位
    
};