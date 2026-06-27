/*
* 5v5 比赛控制面板对话框
* 提供比赛规则、策略选择与比赛流程控制；
* 策略以插件形式接入，接口约定与标定采色一致
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

// 策略插件接口约定
#include "StrategyPluginInterface.h"

// 前向声明，避免头文件循环依赖
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

private:
    void initUI();

private:
    // --- UI 控件 ---
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
    QPushButton* btnPrepare;
    QPushButton* btnStartMatch;
    QPushButton* btnPauseMatch;
    QPushButton* btnStopMatch;
    QPushButton* btnResetPosition;
    QPushButton* btnStrategyParam;
    QComboBox* comboStrategy;

    // --- 互斥单选 ButtonGroup ---
    QButtonGroup* m_danShuangGroup;      // 单双后卫
    QButtonGroup* m_kickTeamGroup;       // 开球方
    QButtonGroup* m_areaGroup;           // 左右半场
    QButtonGroup* m_dqDirectGroup;       // 点球选择
    QButtonGroup* m_goalkeeperGroup;     // 守门选择

    // --- 比赛规则与状态参数 ---
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

    // --- 关联的显示对话框 ---
    DisplayDlg* m_pDisplayDlg;

    // 当前加载的策略插件实例
    StrategyPluginInterface* m_strategyPlugin;

    // 比赛运行状态标志
    bool m_isMatchRunning;
};