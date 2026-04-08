#include "MatchDlg_5vs5.h"

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
{
    // 设置大小策略为可伸缩
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    initUI();
}

MatchDlg_5vs5::~MatchDlg_5vs5()
{}

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
    // TODO: 开始比赛
}

void MatchDlg_5vs5::onButtonStop()
{
    // TODO: 结束比赛
}

void MatchDlg_5vs5::onStrategyChanged(int index)
{
    // TODO: 策略选择变化
}

void MatchDlg_5vs5::onButtonPrepare()
{
    // TODO: 初始预备
}

void MatchDlg_5vs5::onButtonExchangeRole()
{
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
}
