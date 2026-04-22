/*
* 5v5比赛对话框头文件
* 写作人 李青
* 功能 5v5比赛控制面板的类，包含各种比赛规则单选框、策略选择和比赛状态控制方法。
* 未完成
*/
#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QElapsedTimer>

class MatchDlg_5vs5 : public QWidget
{
    Q_OBJECT

public:
    MatchDlg_5vs5(QWidget* parent = nullptr); //初始化比赛控制界面
    ~MatchDlg_5vs5();

private slots:
    void onButtonStart();                     //开始比赛按钮点击
    void onButtonStop();                      //停止比赛按钮点击
    void onStrategyChanged(int index);        //策略下拉框索引改变
    void onButtonPrepare();                   //初始预备按钮点击
    void onButtonExchangeRole();              //替换角色按钮点击
    void onRadioButtonClicked();              //各类设定单选按钮点击

private:
    void initUI();                            //UI初始化
    void Initialize();                        //比赛内部数据初始化
    void StartGame();                         //执行开始比赛内部逻辑
    void StopGame();                          //执行终止比赛内部逻辑

private:
    // --- 控件部件 ---
    QRadioButton* radioAttack;              // 开球方(我方)单选按钮
    QRadioButton* radioDefend;              // 开球方(对方)单选按钮
    QRadioButton* radioLeftArea;            // 场地(左半场)单选按钮
    QRadioButton* radioRightArea;           // 场地(右半场)单选按钮
    QRadioButton* radioNormalKick;          // 开球方式(普通)单选按钮
    QRadioButton* radioPenaltyKick;         // 开球方式(点球)单选按钮
    QRadioButton* radioLeftDirect;          // 点球方式(左晃)单选按钮
    QRadioButton* radioRightDirect;         // 点球方式(右晃)单选按钮
    QRadioButton* radioDan;                 // 后卫数量(单后卫)单选按钮
    QRadioButton* radioShuang;              // 后卫数量(双后卫)单选按钮
    QRadioButton* radioLeftGoalkeeper;      // 守门位置(左)单选按钮
    QRadioButton* radioRightGoalkeeper;     // 守门位置(右)单选按钮
    QPushButton* btnStartMatch;             // 开始比赛控制按钮
    QPushButton* btnPauseMatch;             // 暂停比赛控制按钮
    QPushButton* btnStopMatch;              // 停止比赛控制按钮
    QPushButton* btnResetPosition;          // 位置重置控制按钮
    QPushButton* btnExchangeRobot;          // 机器人交换操作按钮
    QComboBox* comboStrategy;               // 战术策略选择下拉框

    // --- 数据变量 ---
    int m_attack;                           // 是否作为进攻方标志
    bool m_BallLost;                        // 当前是否处于丢球状态标志
    bool m_checkinfo;                       // 是否开启信息校验标志
    bool m_IdentifyOpp;                     // 是否开启对方识别标志
    bool m_CorrectPatch;                    // 是否开启色块纠错模式标志
    bool m_return2pt;                       // 是否执行归位操作标志
    int m_dqsmd;                            // 特定点球状态参数值
    int m_area;                             // 当前队伍所在半场值
    int m_kick;                             // 当前定位球发球类型值
    int m_dqdirect;                         // 点球发球预期方向值
    int m_exchangerobot1;                   // 用于角色替换的主力车号参数
    int m_exchangerobot2;                   // 用于角色替换的替补车号参数
    int m_dan;                              // 当前采取的后卫阵型(单双)值

    int StrategyNum;                        // 当前选定的战术策略索引
    

};
