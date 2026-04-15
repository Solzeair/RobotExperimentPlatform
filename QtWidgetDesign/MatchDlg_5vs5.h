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
#include <QDialog>
#include <QTimer>
#include <QLibrary>
// ==================== 数据结构定义（与DLL保持一致）====================
struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
};

struct RobotPose {
    double x, y, theta;
    double vx, vy;
    double vtheta;
    RobotPose(double x = 0, double y = 0, double theta = 0,
        double vx = 0, double vy = 0, double vtheta = 0)
        : x(x), y(y), theta(theta), vx(vx), vy(vy), vtheta(vtheta) {
    }
};

struct BallInfo {
    Point pos;
    double vel_x, vel_y;
    double velocity;
    double angle;
    Point predictPos;
    BallInfo() : vel_x(0), vel_y(0), velocity(0), angle(0) {}
};

struct WheelVelocity {
    double left, right;
    WheelVelocity(double l = 0, double r = 0) : left(l), right(r) {}
};

// ==================== 函数指针类型定义 ====================
typedef void* (*CreateStrategyFunc)(int);
typedef void (*DestroyStrategyFunc)(void*);
typedef void (*DecideFunc)(void*, const RobotPose*, const Point*, const BallInfo*, WheelVelocity*);
typedef void (*SetParameterFunc)(void*, const char*, double);
typedef double (*GetParameterFunc)(void*, const char*);
typedef void (*InitializeStrategyFunc)(void*, int, int, int, int, int);
typedef void (*SetOurGoalOnRightFunc)(void*, bool);
typedef void (*SetOurKickoffFunc)(void*, bool);
typedef void (*SetMatchStateFunc)(void*, int);
typedef void (*SetFormationTypeFunc)(void*, int);
typedef void (*SetPenaltyKickModeFunc)(void*, int, int);
typedef void (*SelectStrategyFunc)(void*, int);
typedef void (*ParkRobotsFunc)(void*);
typedef void (*StartMatchFunc)(void*);
typedef void (*StopMatchFunc)(void*);
typedef void (*SetKickoffTypeFunc)(void*, int);
typedef bool (*SaveConfigFunc)(void*, const char*);
typedef bool (*LoadConfigFunc)(void*, const char*);



class MatchDlg_5vs5 : public QWidget
{
    Q_OBJECT

public:
    MatchDlg_5vs5(QWidget* parent = nullptr); //初始化比赛控制界面
    ~MatchDlg_5vs5();
    // ========== DLL管理接口 ==========
    bool loadStrategyDLL(const QString& dllPath);
    void unloadStrategyDLL();
    void* getStrategy() const { return m_strategy; }

private slots:
    void onButtonStart();                     //开始比赛按钮点击
    void onButtonStop();                      //停止比赛按钮点击
    void onStrategyChanged(int index);        //策略下拉框索引改变
    void onButtonPrepare();                   //初始预备按钮点击
    void onButtonExchangeRole();              //替换角色按钮点击
    void onRadioButtonClicked();              //各类设定单选按钮点击
    void onDecisionTimer();                          // DLL对接槽函数 决策定时器


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
    // ========== DLL相关 ==========
    QLibrary* m_library;
    void* m_strategy;

    // 函数指针
    CreateStrategyFunc m_createStrategy;
    DestroyStrategyFunc m_destroyStrategy;
    DecideFunc m_decide;
    SetParameterFunc m_setParameter;
    GetParameterFunc m_getParameter;
    SetOurGoalOnRightFunc m_setOurGoalOnRight;
    SetOurKickoffFunc m_setOurKickoff;
    InitializeStrategyFunc m_initializeStrategy;
    SetMatchStateFunc m_setMatchState;
    SetFormationTypeFunc m_setFormationType;
    SetKickoffTypeFunc m_setKickoffType;
    SetPenaltyKickModeFunc m_setPenaltyKickMode;
    SelectStrategyFunc m_selectStrategy;
    ParkRobotsFunc m_parkRobotsFunc;       
    StartMatchFunc m_startMatchFunc;      
    StopMatchFunc m_stopMatchFunc;      
    SaveConfigFunc m_saveConfigFunc;      
    LoadConfigFunc m_loadConfigFunc;    

    // 决策定时器
    QTimer* m_decisionTimer;

    // 比赛状态
    bool m_isMatchRunning;
    // ========== 测试数据（后续对接视觉系统后删除）==========
    RobotPose m_testRobots[5];
    Point m_testOppRobots[5];
    BallInfo m_testBall;
    WheelVelocity m_testVelocities[5];
};