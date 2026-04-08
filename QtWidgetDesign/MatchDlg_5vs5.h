#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

class MatchDlg_5vs5 : public QWidget
{
    Q_OBJECT

public:
    MatchDlg_5vs5(QWidget *parent = nullptr);
    ~MatchDlg_5vs5();

private slots:
    void onButtonStart();
    void onButtonStop();
    void onStrategyChanged(int index);
    void onButtonPrepare();
    void onButtonExchangeRole();
    void onRadioButtonClicked();

private:
    void initUI();
    void Initialize();
    void StartGame();
    void StopGame();

private:
    // 控件
    QRadioButton *radioAttack;
    QRadioButton *radioDefend;
    QRadioButton *radioLeftArea;
    QRadioButton *radioRightArea;
    QRadioButton *radioNormalKick;
    QRadioButton *radioPenaltyKick;
    QRadioButton *radioLeftDirect;
    QRadioButton *radioRightDirect;
    QRadioButton *radioDan;
    QRadioButton *radioShuang;
    QRadioButton *radioLeftGoalkeeper;
    QRadioButton *radioRightGoalkeeper;
    QPushButton *btnStartMatch;
    QPushButton *btnPauseMatch;
    QPushButton *btnStopMatch;
    QPushButton *btnResetPosition;
    QPushButton *btnExchangeRobot;
    QComboBox *comboStrategy;
    
    // 数据
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
};
