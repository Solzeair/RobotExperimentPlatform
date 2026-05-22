/*
* 5v5比赛对话框头文件
* 写作人 李青
* 功能 5v5比赛控制面板的类，包含各种比赛规则单选框、策略选择和比赛状态控制方法。
* 策略通过插件形式实现，与标定采色接口保持一致
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
#include <QButtonGroup>
#include <QProcess>
#include <QFile>

// 包含策略插件接口
#include "StrategyPluginInterface.h"

// 前向声明
class DisplayDlg;

class MatchDlg_5vs5 : public QWidget
{
    Q_OBJECT

public:
    MatchDlg_5vs5(QWidget* parent = nullptr);
    ~MatchDlg_5vs5();
    void setDisplayDlg(DisplayDlg* displayDlg);

    // ========== 策略插件接口 ==========
    StrategyPluginInterface* getStrategyPlugin() const { return m_strategyPlugin; }

    // ========== 参数设置接口 ==========
    void applyMatchParameters();

private slots:
    void onButtonStart();
    void onButtonStop();
    void onStrategyChanged(int index);
    void onButtonPrepare();
    void onButtonStrategyParam();
    void onRadioButtonClicked();
    void onStrategyTimer();  // 策略定时器槽函数
    void onUpdateFPS();      // 帧率显示更新（可选）
private:
    void sendVelocitiesToRobots(const WheelVelocity velocities[5]);  // 发送轮速给机器人
    void updateDisplayDlgData();  // 从 DisplayDlg 获取识别数据


private:
    void initUI();
    // DLL 函数指针类型定义
    typedef void* (*CreateStrategyFunc)();
    typedef void (*DestroyStrategyFunc)(void*);
    typedef void (*InitializeStrategyFunc)(void*, int);
    typedef void (*SetFormationTypeFunc)(void*, int);
    typedef void (*SetOurGoalOnRightFunc)(void*, int);
    typedef void (*SetOurKickoffFunc)(void*, int);
    typedef void (*SetPenaltyKickModeFunc)(void*, int, int);
    typedef void (*SelectStrategyFunc)(void*, int);
    typedef void (*SetParameterFunc)(void*, const char*, double);
    typedef void (*GetParameterFunc)(void*, const char*);
    typedef void (*ParkRobotsFunc)(void*);
    typedef void (*SetKickoffTypeFunc)(void*, int);  // 开球方式
    typedef void (*DecideFunc)(void*, const RobotPose*, const Point*, const BallInfo*, WheelVelocity*);

    // 策略句柄和函数指针
    void* m_strategyHandle;
    DecideFunc m_decide;  // 决策函数指针
    InitializeStrategyFunc m_initStrategy;
    SetFormationTypeFunc m_setFormation;
    SetOurGoalOnRightFunc m_setOurGoalOnRight;
    SetOurKickoffFunc m_setOurKickoff;
    SetPenaltyKickModeFunc m_setPenaltyKickMode;
    SelectStrategyFunc m_selectStrategy;
    SetParameterFunc m_setParameter;
    ParkRobotsFunc m_parkRobots;
    SetKickoffTypeFunc m_setKickoffType;

private:
    // --- 控件部件 ---
    QRadioButton* radioAttack;
    QRadioButton* radioDefend;
    QRadioButton* radioLeftArea;
    QRadioButton* radioRightArea;
    QRadioButton* radioNormalKick;
    QRadioButton* radioPenaltyKick;
    QRadioButton* radioLeftDirect;
    QRadioButton* radioMiddleDirect;
    QRadioButton* radioRightDirect;
    QRadioButton* radioDan;
    QRadioButton* radioShuang;
    QRadioButton* radioLeftGoalkeeper;
    QRadioButton* radioCenterGoalkeeper;
    QRadioButton* radioRightGoalkeeper;
    QRadioButton* radioGoalKick;
    QRadioButton* radioFreeKick;
    QRadioButton* radioFreeBall;
    QRadioButton* radioShouqiu;
    QPushButton* btnPrepare;
    QPushButton* btnStartMatch;
    QPushButton* btnPauseMatch;
    QPushButton* btnStopMatch;
    QPushButton* btnResetPosition;
    QPushButton* btnStrategyParam;
    QComboBox* comboStrategy;

    // --- ButtonGroup for exclusive selection ---
    QButtonGroup* m_danShuangGroup;      // 单双后卫
    QButtonGroup* m_kickTeamGroup;       // 开球方
    QButtonGroup* m_areaGroup;           // 左右半场
    QButtonGroup* m_dqDirectGroup;       // 点球选择
    QButtonGroup* m_goalkeeperGroup;     // 守门选择

    QTimer* m_strategyTimer;      // 策略执行定时器
    QTimer* m_fpsTimer;           // 帧率显示定时器（可选）

    // 缓存当前帧的识别数据
    RobotPose m_cachedRobots[5];
    Point m_cachedOppRobots[5];
    BallInfo m_cachedBall;
    bool m_hasValidData;           // 是否有有效数据
    // --- 比赛参数变量 ---
    int m_attack;
    bool m_BallLost;
    bool m_checkinfo;
    bool m_IdentifyOpp;
    bool m_CorrectPatch;
    bool m_return2pt;
    int m_dqsmd;
    int m_area;
    int m_kick;
    int m_dqdirect;
    int m_exchangerobot1;
    int m_exchangerobot2;
    int m_dan;
    int StrategyNum;

    // --- 显示对话框指针 ---
    DisplayDlg* m_pDisplayDlg;

    // ========== 策略插件接口 ==========
    StrategyPluginInterface* m_strategyPlugin;

    // 比赛状态
    bool m_isMatchRunning;
};