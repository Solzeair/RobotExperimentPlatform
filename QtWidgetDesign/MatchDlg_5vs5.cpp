/*
* 5v5比赛对话框源文件
* 写作人 李青
* 功能 5v5比赛控制界面逻辑实现，包含开球类型、阵型布置、点球及战术选择功能响应。
* 未完成
*/
#include "MatchDlg_5vs5.h"
#include <QLibrary>
#include <QMessageBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <iostream>
#include <QDebug>

MatchDlg_5vs5::MatchDlg_5vs5(QWidget *parent)
    : QWidget(parent)
    , m_attack(0)
    , m_BallLost(true)
    , m_checkinfo(false)
    , m_IdentifyOpp(true)
    , m_return2pt(false)
    , m_dqsmd(1)
    , m_area(0)
    , m_kick(0)
    , m_dqdirect(1)
    , m_exchangerobot1(0)
    , m_exchangerobot2(0)
    , m_dan(0)
    // ========== 新增：初始化DLL相关变量 ==========
    , m_library(nullptr)
    , m_strategy(nullptr)
    , m_createStrategy(nullptr)
    , m_destroyStrategy(nullptr)
    , m_decide(nullptr)
    , m_setParameter(nullptr)
    , m_getParameter(nullptr)
    , m_initializeStrategy(nullptr)
    , m_setOurGoalOnRight(nullptr)
    , m_setOurKickoff(nullptr)
    , m_setMatchState(nullptr)
    , m_setFormationType(nullptr)
    , m_setKickoffType(nullptr)
    , m_setPenaltyKickMode(nullptr)
    , m_selectStrategy(nullptr)
    , m_parkRobotsFunc(nullptr)
    , m_startMatchFunc(nullptr)
    , m_stopMatchFunc(nullptr)
    , m_saveConfigFunc(nullptr)
    , m_loadConfigFunc(nullptr)
    , m_decisionTimer(nullptr)
    , m_isMatchRunning(false)
{
    // 设置大小策略为可伸缩
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    initUI();
    // ========== 新增：加载策略DLL ==========
    loadStrategyDLL("RobotStrategyDll.dll");

    // ========== 新增：初始化测试数据 ==========
    // 机器人初始位置
    m_testRobots[0] = RobotPose(50, 90, 0);
    m_testRobots[1] = RobotPose(40, 70, 0);
    m_testRobots[2] = RobotPose(40, 110, 0);
    m_testRobots[3] = RobotPose(30, 50, 0);
    m_testRobots[4] = RobotPose(8, 90, 1.57);

    // 对方机器人位置
    m_testOppRobots[0] = Point(150, 90);
    m_testOppRobots[1] = Point(160, 70);
    m_testOppRobots[2] = Point(160, 110);
    m_testOppRobots[3] = Point(170, 50);
    m_testOppRobots[4] = Point(170, 130);

    // 球的信息
    m_testBall.pos = Point(150, 90);
    m_testBall.vel_x = 10;
    m_testBall.vel_y = 0;
    m_testBall.velocity = 10;

    // 创建决策定时器
    m_decisionTimer = new QTimer(this);
    connect(m_decisionTimer, &QTimer::timeout, this, &MatchDlg_5vs5::onDecisionTimer);
}

MatchDlg_5vs5::~MatchDlg_5vs5()
{
    unloadStrategyDLL();
}
// ========== 新增：加载策略DLL ==========
bool MatchDlg_5vs5::loadStrategyDLL(const QString& dllPath)
{
    m_library = new QLibrary(dllPath, this);

    if (!m_library->load()) {
        qDebug() << "Failed to load DLL:" << m_library->errorString();
        return false;
    }
    // 解析函数指针
    m_createStrategy = (CreateStrategyFunc)m_library->resolve("CreateStrategy");
    m_destroyStrategy = (DestroyStrategyFunc)m_library->resolve("DestroyStrategy");
    m_decide = (DecideFunc)m_library->resolve("decide");
    m_setParameter = (SetParameterFunc)m_library->resolve("setParameter");
    m_getParameter = (GetParameterFunc)m_library->resolve("getParameter");
    m_initializeStrategy = (InitializeStrategyFunc)m_library->resolve("InitializeStrategy");
    m_setOurGoalOnRight = (SetOurGoalOnRightFunc)m_library->resolve("setOurGoalOnRight");
    m_setOurKickoff = (SetOurKickoffFunc)m_library->resolve("setOurKickoff");

    // 可选函数（如果DLL实现了）
    m_setMatchState = (SetMatchStateFunc)m_library->resolve("SetMatchState");
    m_setFormationType = (SetFormationTypeFunc)m_library->resolve("SetFormationType");
    m_setKickoffType = (SetKickoffTypeFunc)m_library->resolve("SetKickoffType");
    m_setPenaltyKickMode = (SetPenaltyKickModeFunc)m_library->resolve("SetPenaltyKickMode");
    m_selectStrategy = (SelectStrategyFunc)m_library->resolve("SelectStrategy");
    m_parkRobotsFunc = (ParkRobotsFunc)m_library->resolve("ParkRobots");
    m_saveConfigFunc = (SaveConfigFunc)m_library->resolve("saveConfig");
    m_loadConfigFunc = (LoadConfigFunc)m_library->resolve("loadConfig");

    if (!m_createStrategy || !m_destroyStrategy) {
        qDebug() << "Failed to resolve required functions!";
        return false;
    }

    // 创建策略实例
    m_strategy = m_createStrategy(0);
    if (!m_strategy) {
        qDebug() << "Failed to create strategy!";
        return false;
    }

    // 设置默认参数
    if (m_setParameter) {
        m_setParameter(m_strategy, "max_speed", 75.0);
        m_setParameter(m_strategy, "kp_pos", 12.0);
        m_setParameter(m_strategy, "kp_angle", 22.0);
    }

    qDebug() << "DLL loaded successfully!";
    return true;
}// ========== 新增：卸载DLL ==========
void MatchDlg_5vs5::unloadStrategyDLL()
{
    if (m_decisionTimer && m_decisionTimer->isActive()) {
        m_decisionTimer->stop();
    }

    if (m_destroyStrategy && m_strategy) {
        m_destroyStrategy(m_strategy);
        m_strategy = nullptr;
    }

    if (m_library) {
        m_library->unload();
        delete m_library;
        m_library = nullptr;
    }
}

void MatchDlg_5vs5::initUI()
{
    // 设置字体为楷体，12号，加粗
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // 标题 "比赛"
    QLabel *titleLabel = new QLabel("比赛", this);
    titleLabel->setFont(font);
    mainLayout->addWidget(titleLabel);
    
    // 创建顶部布局（左侧显示区域 + 右侧控制区域）
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(20);
    
    // 左侧显示区域
    QLabel *displayLabel = new QLabel("比赛状态显示区域", this);
    displayLabel->setStyleSheet("QLabel { background-color: #333333; border: 1px solid black; color: white; }");
    displayLabel->setFont(font);
    displayLabel->setAlignment(Qt::AlignCenter);
    displayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    topLayout->addWidget(displayLabel);
    
    // 右侧控制区域
    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->setSpacing(15);
    
    // 单双后卫
    QHBoxLayout *danShuangLayout = new QHBoxLayout();
    QLabel *danShuangLabel = new QLabel("单双后卫", this);
    danShuangLabel->setFont(font);
    radioDan = new QRadioButton("单后卫", this);
    radioDan->setChecked(true);
    radioDan->setFont(font);
    radioShuang = new QRadioButton("双后卫", this);
    radioShuang->setFont(font);
    danShuangLayout->addWidget(danShuangLabel);
    danShuangLayout->addWidget(radioDan);
    danShuangLayout->addWidget(radioShuang);
    controlLayout->addLayout(danShuangLayout);
    
    // 开球方
    QHBoxLayout *kickTeamLayout = new QHBoxLayout();
    QLabel *kickTeamLabel = new QLabel("开球方", this);
    kickTeamLabel->setFont(font);
    radioAttack = new QRadioButton("我方", this);
    radioAttack->setChecked(true);
    radioAttack->setFont(font);
    radioDefend = new QRadioButton("对方", this);
    radioDefend->setFont(font);
    kickTeamLayout->addWidget(kickTeamLabel);
    kickTeamLayout->addWidget(radioAttack);
    kickTeamLayout->addWidget(radioDefend);
    controlLayout->addLayout(kickTeamLayout);
    
    // 左右半场
    QHBoxLayout *areaLayout = new QHBoxLayout();
    QLabel *areaLabel = new QLabel("左右半场", this);
    areaLabel->setFont(font);
    radioLeftArea = new QRadioButton("左半场", this);
    radioLeftArea->setChecked(true);
    radioLeftArea->setFont(font);
    radioRightArea = new QRadioButton("右半场", this);
    radioRightArea->setFont(font);
    areaLayout->addWidget(areaLabel);
    areaLayout->addWidget(radioLeftArea);
    areaLayout->addWidget(radioRightArea);
    controlLayout->addLayout(areaLayout);
    
    // 开球方式
    QGroupBox *kickGroupBox = new QGroupBox("开球方式", this);
    kickGroupBox->setFont(font);
    QGridLayout *kickGridLayout = new QGridLayout(kickGroupBox);
    kickGridLayout->setContentsMargins(20, 20, 20, 20);
    kickGridLayout->setSpacing(10);
    
    radioNormalKick = new QRadioButton("普通", kickGroupBox);
    radioNormalKick->setChecked(true);
    radioNormalKick->setFont(font);
    radioPenaltyKick = new QRadioButton("点球", kickGroupBox);
    radioPenaltyKick->setFont(font);
    QRadioButton *radioGoalKick = new QRadioButton("门球", kickGroupBox);
    radioGoalKick->setFont(font);
    QRadioButton *radioFreeKick = new QRadioButton("任意球", kickGroupBox);
    radioFreeKick->setFont(font);
    QRadioButton *radioFreeBall = new QRadioButton("争球", kickGroupBox);
    radioFreeBall->setFont(font);
    QRadioButton *radioShouqiu = new QRadioButton("收车", kickGroupBox);
    radioShouqiu->setFont(font);
    QRadioButton *radioLostCarTest = new QRadioButton("标定测试", kickGroupBox);
    radioLostCarTest->setFont(font);
    QRadioButton *radioTest = new QRadioButton("性能测试", kickGroupBox);
    radioTest->setFont(font);
    
    kickGridLayout->addWidget(radioNormalKick, 0, 0);
    kickGridLayout->addWidget(radioPenaltyKick, 0, 1);
    kickGridLayout->addWidget(radioGoalKick, 0, 2);
    kickGridLayout->addWidget(radioFreeKick, 1, 0);
    kickGridLayout->addWidget(radioFreeBall, 1, 1);
    kickGridLayout->addWidget(radioShouqiu, 1, 2);
    kickGridLayout->addWidget(radioLostCarTest, 2, 0);
    kickGridLayout->addWidget(radioTest, 2, 1);
    
    controlLayout->addWidget(kickGroupBox);
    
    // 点球选择
    QHBoxLayout *dqSelectLayout = new QHBoxLayout();
    QLabel *dqSelectLabel = new QLabel("点球选择", this);
    dqSelectLabel->setFont(font);
    radioLeftDirect = new QRadioButton("左晃射门", this);
    radioLeftDirect->setFont(font);
    QRadioButton *radioMiddleDirect = new QRadioButton("随机直冲", this);
    radioMiddleDirect->setFont(font);
    radioMiddleDirect->setChecked(true);
    radioRightDirect = new QRadioButton("右晃射门", this);
    radioRightDirect->setFont(font);
    dqSelectLayout->addWidget(dqSelectLabel);
    dqSelectLayout->addWidget(radioLeftDirect);
    dqSelectLayout->addWidget(radioMiddleDirect);
    dqSelectLayout->addWidget(radioRightDirect);
    controlLayout->addLayout(dqSelectLayout);
    
    // 守门选择
    QHBoxLayout *goalkeeperLayout = new QHBoxLayout();
    QLabel *goalkeeperLabel = new QLabel("守门选择", this);
    goalkeeperLabel->setFont(font);
    radioLeftGoalkeeper = new QRadioButton("左", this);
    radioLeftGoalkeeper->setFont(font);
    QRadioButton *radioCenterGoalkeeper = new QRadioButton("中", this);
    radioCenterGoalkeeper->setFont(font);
    radioCenterGoalkeeper->setChecked(true);
    radioRightGoalkeeper = new QRadioButton("右", this);
    radioRightGoalkeeper->setFont(font);
    goalkeeperLayout->addWidget(goalkeeperLabel);
    goalkeeperLayout->addWidget(radioLeftGoalkeeper);
    goalkeeperLayout->addWidget(radioCenterGoalkeeper);
    goalkeeperLayout->addWidget(radioRightGoalkeeper);
    controlLayout->addLayout(goalkeeperLayout);
    
    // 细节处理
    QHBoxLayout *detailsLayout = new QHBoxLayout();
    QLabel *detailsLabel = new QLabel("细节处理", this);
    detailsLabel->setFont(font);
    QCheckBox *checkColorError = new QCheckBox("色标纠错", this);
    checkColorError->setFont(font);
    checkColorError->setChecked(true);
    QCheckBox *checkIdentifyOpponent = new QCheckBox("辨识对方", this);
    checkIdentifyOpponent->setFont(font);
    checkIdentifyOpponent->setChecked(true);
    detailsLayout->addWidget(detailsLabel);
    detailsLayout->addWidget(checkColorError);
    detailsLayout->addWidget(checkIdentifyOpponent);
    controlLayout->addLayout(detailsLayout);
    
    // 细节处理第二行
    QHBoxLayout *detailsRow2Layout = new QHBoxLayout();
    QCheckBox *checkRobotCheck = new QCheckBox("RobotCheck", this);
    checkRobotCheck->setFont(font);
    QCheckBox *checkReturn = new QCheckBox("归位", this);
    checkReturn->setFont(font);
    detailsRow2Layout->addWidget(checkRobotCheck);
    detailsRow2Layout->addWidget(checkReturn);
    controlLayout->addLayout(detailsRow2Layout);
    
    // 比赛控制按钮
    QHBoxLayout *matchControlLayout = new QHBoxLayout();
    QPushButton *btnPrepare = new QPushButton("初始预备", this);
    btnPrepare->setFont(font);
    btnStartMatch = new QPushButton("开始比赛", this);
    btnStartMatch->setFont(font);
    btnStopMatch = new QPushButton("停止", this);
    btnStopMatch->setFont(font);
    matchControlLayout->addWidget(btnPrepare);
    matchControlLayout->addWidget(btnStartMatch);
    matchControlLayout->addWidget(btnStopMatch);
    controlLayout->addLayout(matchControlLayout);
    
    // 策略选择
    QHBoxLayout *strategyLayout = new QHBoxLayout();
    QLabel *strategyLabel = new QLabel("策略", this);
    strategyLabel->setFont(font);
    comboStrategy = new QComboBox(this);
    comboStrategy->addItem("1号策略");
    comboStrategy->addItem("2号策略");
    comboStrategy->addItem("3号策略");
    comboStrategy->addItem("4号策略");
    comboStrategy->setFont(font);
    strategyLayout->addWidget(strategyLabel);
    strategyLayout->addWidget(comboStrategy);
    controlLayout->addLayout(strategyLayout);
    
    // 角色替换
    QHBoxLayout *roleExchangeLayout = new QHBoxLayout();
    QLabel *roleExchangeLabel = new QLabel("角色替换", this);
    roleExchangeLabel->setFont(font);
    QLabel *newCarLabel = new QLabel("新车", this);
    newCarLabel->setFont(font);
    QLineEdit *lineEditNewCar = new QLineEdit(this);
    lineEditNewCar->setFont(font);
    lineEditNewCar->setText("0");
    lineEditNewCar->setFixedWidth(50);
    QLabel *exchangeRoleLabel = new QLabel("替换角色", this);
    exchangeRoleLabel->setFont(font);
    QLineEdit *lineEditExchangeRole = new QLineEdit(this);
    lineEditExchangeRole->setFont(font);
    lineEditExchangeRole->setText("0");
    lineEditExchangeRole->setFixedWidth(50);
    QPushButton *btnExchangeRole = new QPushButton("替换", this);
    btnExchangeRole->setFont(font);
    btnExchangeRole->setFixedWidth(60);
    
    roleExchangeLayout->addWidget(roleExchangeLabel);
    roleExchangeLayout->addWidget(newCarLabel);
    roleExchangeLayout->addWidget(lineEditNewCar);
    roleExchangeLayout->addWidget(exchangeRoleLabel);
    roleExchangeLayout->addWidget(lineEditExchangeRole);
    roleExchangeLayout->addWidget(btnExchangeRole);
    controlLayout->addLayout(roleExchangeLayout);
    
    topLayout->addLayout(controlLayout);
    mainLayout->addLayout(topLayout);
    
    // 输出区域
    QLabel *outputLabel = new QLabel(this);
    outputLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid black; font-family: Consolas; font-size: 10pt; }");
    outputLabel->setFont(font);
    outputLabel->setText("输出区域:");
    outputLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    outputLabel->setFixedHeight(100);
    mainLayout->addWidget(outputLabel);
    
    // 连接信号槽
    connect(btnStartMatch, SIGNAL(clicked()), this, SLOT(onButtonStart()));
    connect(btnStopMatch, SIGNAL(clicked()), this, SLOT(onButtonStop()));
    connect(comboStrategy, SIGNAL(currentIndexChanged(int)), this, SLOT(onStrategyChanged(int)));
    connect(btnPrepare, SIGNAL(clicked()), this, SLOT(onButtonPrepare()));
    connect(btnExchangeRole, SIGNAL(clicked()), this, SLOT(onButtonExchangeRole()));
    
    // 连接单选按钮
    connect(radioDan, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioShuang, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioAttack, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioDefend, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioLeftArea, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioRightArea, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioNormalKick, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioPenaltyKick, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioLeftDirect, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioRightDirect, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioLeftGoalkeeper, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioRightGoalkeeper, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
}

void MatchDlg_5vs5::onButtonStart()
{
    if (!m_strategy) {
        QMessageBox::warning(this, "错误", "策略DLL未加载");
        return;
    }

    // 初始化比赛场景
    if (m_initializeStrategy) {
        int isFirstHalf = (m_area == 0) ? 1 : 0;  // 左半场=上半场
        int isOurKickoff = (m_attack == 1) ? 1 : 0;

        m_initializeStrategy(m_strategy, 0, 0, 300, isFirstHalf, isOurKickoff);
    }

    // 设置球门方向
    if (m_setOurGoalOnRight) {
        m_setOurGoalOnRight(m_strategy, (m_area == 1));  // 右半场时球门在右
    }

    // 设置开球方
    if (m_setOurKickoff) {
        m_setOurKickoff(m_strategy, (m_attack == 1));
    }

    // 设置开球方式
    if (m_setKickoffType) {
        m_setKickoffType(m_strategy, m_kick);
    }

    // 设置阵型（单双后卫）
    if (m_setFormationType) {
        m_setFormationType(m_strategy, m_dan);  // 0=单后卫, 1=双后卫
    }

    // 设置点球模式
    if (m_setPenaltyKickMode) {
        m_setPenaltyKickMode(m_strategy, m_dqdirect, m_dqsmd);
    }

    // 设置策略
    if (m_selectStrategy) {
        m_selectStrategy(m_strategy, StrategyNum);
    }

    // 开始决策循环
    m_isMatchRunning = true;
    m_decisionTimer->start(33);  // 约30帧/秒

    qDebug() << "Match started!";
} // TODO: 开始比赛

void MatchDlg_5vs5::onButtonStop()
{
    m_isMatchRunning = false;
    if (m_decisionTimer) {
        m_decisionTimer->stop();
    }

    if (m_stopMatchFunc && m_strategy) {
        m_stopMatchFunc(m_strategy);
    }

    qDebug() << "Match stopped!";
    // TODO: 结束比赛
}

void MatchDlg_5vs5::onStrategyChanged(int index)
{
    StrategyNum = index;

    if (m_selectStrategy && m_strategy) {
        m_selectStrategy(m_strategy, index);
    }

    qDebug() << "Strategy changed to:" << index;
    // TODO: 策略选择变化
}

void MatchDlg_5vs5::onButtonPrepare()
{  // TODO: 初始预备逻辑
    // 可以调用 m_parkRobots 让机器人归位
    if (m_parkRobotsFunc && m_strategy) {
        m_parkRobotsFunc(m_strategy);
    }

    // TODO: 初始预备
}

void MatchDlg_5vs5::onButtonExchangeRole()
{ // TODO: 角色替换逻辑
    qDebug() << "Exchange role: robot" << m_exchangerobot1 << "with" << m_exchangerobot2;
    // TODO: 角色替换
}

void MatchDlg_5vs5::onRadioButtonClicked()
{
    // TODO: 处理单选按钮点击
    if (radioDan->isChecked())
        m_dan = 0;
    else if (radioShuang->isChecked())
        m_dan = 1;
    
    if (radioAttack->isChecked())
        m_attack = 1;
    else if (radioDefend->isChecked())
        m_attack = 0;
    
    if (radioLeftArea->isChecked())
        m_area = 0;
    else if (radioRightArea->isChecked())
        m_area = 1;
    
    if (radioNormalKick->isChecked())
        m_kick = 0;
    else if (radioPenaltyKick->isChecked())
        m_kick = 1;
    
    if (radioLeftDirect->isChecked())
        m_dqdirect = 0;
    else if (radioRightDirect->isChecked())
        m_dqdirect = 2;
    else
        m_dqdirect = 1;
    // ========== 新增：实时应用设置到策略 ==========
    // 阵型
    if (m_setFormationType && m_strategy) {
        m_setFormationType(m_strategy, m_dan);
    }

    // 球门方向
    if (m_setOurGoalOnRight && m_strategy) {
        m_setOurGoalOnRight(m_strategy, (m_area == 1));
    }

    // 开球方
    if (m_setOurKickoff && m_strategy) {
        m_setOurKickoff(m_strategy, (m_attack == 1));
    }

    // 开球方式
    if (m_setKickoffType && m_strategy) {
        m_setKickoffType(m_strategy, m_kick);
    }

    // 点球模式
    if (m_setPenaltyKickMode && m_strategy) {
        m_setPenaltyKickMode(m_strategy, m_dqdirect, m_dqsmd);
    }
}
// ========== 新增：决策定时器 ==========
void MatchDlg_5vs5::onDecisionTimer()
{
    if (!m_isMatchRunning || !m_decide || !m_strategy) return;

    // 调用策略决策
    m_decide(m_strategy, m_testRobots, m_testOppRobots, &m_testBall, m_testVelocities);

    // 输出轮速（调试用）
    static int frameCount = 0;
    if (frameCount++ % 30 == 0) {  // 每30帧输出一次
        qDebug() << "Decision output - Frame:" << frameCount;
        for (int i = 0; i < 5; i++) {
            qDebug() << "  Robot" << i << ": L=" << m_testVelocities[i].left
                << ", R=" << m_testVelocities[i].right;
        }
    }

    // TODO: 后续对接通信接口，将 m_testVelocities 发送给机器人
}
