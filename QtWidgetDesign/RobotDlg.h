/*
* 调车对话框头文件
* 写作人 李青
* 功能 调车控制界面的类，包含机器人方向控制按钮、通信频率和车辆编号配置相关的属性及方法。
* 已完成 / 未完成
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

class RobotDlg : public QWidget
{
    Q_OBJECT

public:
    RobotDlg(QWidget* parent = nullptr);  //初始化调车面板及其状态
    ~RobotDlg();

private slots:
    void onButtonFront();                 //前进按钮点击
    void onButtonBack();                  //后退按钮点击
    void onButtonLeft();                  //向左转按钮点击
    void onButtonRight();                 //向右转按钮点击
    void onButtonStop();                  //停止运动按钮点击
    void onButtonChangeNum();             //修改小车编号按钮点击
    void onButtonChangeFreq();            //修改通信频率按钮点击

private:
    void initUI();                        //UI初始化

private:
    // --- 布局部件 ---
    QVBoxLayout* mainLayout;              //主垂直区域布局器

    // --- 控件部件 ---
    QLineEdit* editOldNum;                //显示旧车号的文本框
    QLineEdit* editNewNum;                //输入新车号的文本框
    QLineEdit* editNum;                   //输入待设置频率车号的文本框
    QLineEdit* editDeviceStatus;          //用于反馈当前通讯模块状态的文本框
    QPushButton* btnChangeNum;            //触发修改编号命令的功能按钮
    QPushButton* btnChangeFreq;           //触发修改频率命令的功能按钮
    QPushButton* btnFront;                //发出机器人向前运动指令的按钮
    QPushButton* btnBack;                 //发出机器人向后运动指令的按钮
    QPushButton* btnLeft;                 //发出机器人向左运动指令的按钮
    QPushButton* btnRight;                //发出机器人向右运动指令的按钮
    QPushButton* btnStop;                 //发出机器人终止运动指令的按钮
    QRadioButton* radio1_450;             //选择450通讯频道的单选按钮
    QRadioButton* radio1_460;             //选择460通讯频道的单选按钮
    QGroupBox* carNumGroup;               //包裹改号功能的区域
    QGroupBox* controlGroup;              //包裹遥控移动功能的区域
    QGroupBox* carFreqGroup;              //包裹车辆频段配置功能的区域
    QGroupBox* deviceGroup;               //包裹系统发射器配置功能的区域

    // --- 数据变量 ---
    int m_oldNum;                         //操作之前的已有小车编号数值
    int m_newNum;                         //准备配置的目标新小车编号数值
    int m_numSet;                         //当前等待下发频段参数的被控车编号
    bool m_carFre;                        //表示设备选择频带状态的逻辑标志
};