/*
* 5v5比赛对话框源文件
* 写作人 李青
* 功能 5v5比赛控制界面逻辑实现，包含开球类型、阵型布置、点球及战术选择功能响应。
* 策略通过插件形式实现，与标定采色接口保持一致
*/
#include "MatchDlg_5vs5.h"
#include "DisplayDlg.h"
#include "Debug.h"
#include "PluginManager.h"
#include <QMessageBox>
#include <QDebug>

MatchDlg_5vs5::MatchDlg_5vs5(QWidget *parent)
    : QWidget(parent)
    , m_attack(0)
    , m_BallLost(true)
    , m_checkinfo(false)
    , m_IdentifyOpp(true)
    , m_CorrectPatch(false)
    , m_return2pt(false)
    , m_dqsmd(1)
    , m_area(0)
    , m_kick(0)
    , m_dqdirect(1)
    , m_exchangerobot1(0)
    , m_exchangerobot2(0)
    , m_dan(0)
    , StrategyNum(0)
    , m_pDisplayDlg(nullptr)
    , m_strategyPlugin(nullptr)
    , m_isMatchRunning(false)
    , radioMiddleDirect(nullptr)
    , radioCenterGoalkeeper(nullptr)
    , m_danShuangGroup(nullptr)
    , m_kickTeamGroup(nullptr)
    , m_areaGroup(nullptr)
    , m_dqDirectGroup(nullptr)
    , m_goalkeeperGroup(nullptr)
    , btnPrepare(nullptr)
    , btnExchangeRole(nullptr)
    , lineEditNewCar(nullptr)
    , lineEditExchangeRole(nullptr)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initUI();
    // 通过插件管理器从指定路径加载策略插件
    PluginManager* pluginManager = PluginManager::getInstance();
    QString dllPath = QString("e:/bishe/策略/RobotStrategyDll.dll");
    pluginManager->loadPlugin(dllPath, PluginType::STRATEGY);
    m_strategyPlugin = dynamic_cast<StrategyPluginInterface*>(pluginManager->getPlugin(PluginType::STRATEGY));
    
    // 初始状态：禁用开始比赛按钮
    btnStartMatch->setEnabled(false);
}

MatchDlg_5vs5::~MatchDlg_5vs5()
{
    // 策略插件由插件管理器管理，不需要在此释放
}

void MatchDlg_5vs5::setDisplayDlg(DisplayDlg* displayDlg)
{
    m_pDisplayDlg = displayDlg;
    // 设置DisplayDlg指针给策略插件
    if (m_strategyPlugin) {
        m_strategyPlugin->setDisplayDlg(displayDlg);
    }
}

void MatchDlg_5vs5::applyMatchParameters()
{
    if (!m_strategyPlugin) {
        return;
    }

    // 阵型 (0=单后卫, 1=双后卫)
    m_strategyPlugin->setFormationType(m_dan);

    // 球门方向 (左半场=0, 右半场=1)
    m_strategyPlugin->setOurGoalOnRight(m_area == 1);

    // 开球方 (我方=1, 对方=0)
    m_strategyPlugin->setOurKickoff(m_attack == 1);

    // 点球模式 (m_dqdirect=点球方向, m_dqsmd=点球模式)
    m_strategyPlugin->setPenaltyKickMode(m_dqdirect, m_dqsmd);

    // 策略选择
    m_strategyPlugin->selectStrategy(StrategyNum);

    // 设置归位参数
    m_strategyPlugin->setParameter("return2pt", m_return2pt ? 1.0 : 0.0);
}

void MatchDlg_5vs5::initUI()
{
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 0, 20, 20);
    mainLayout->setSpacing(15);

    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->setSpacing(18);

    QLabel *titleLabel = new QLabel("5vs5比赛控制", this);
    titleLabel->setFont(font);
    controlLayout->addWidget(titleLabel);

    // 单双后卫
    QHBoxLayout *danShuangLayout = new QHBoxLayout();
    QLabel *danShuangLabel = new QLabel("单双后卫", this);
    danShuangLabel->setFont(font);
    radioDan = new QRadioButton("单后卫", this);
    radioDan->setChecked(true);
    radioDan->setFont(font);
    radioShuang = new QRadioButton("双后卫", this);
    radioShuang->setFont(font);
    // 添加到ButtonGroup实现单选互斥
    m_danShuangGroup = new QButtonGroup(this);
    m_danShuangGroup->addButton(radioDan, 0);
    m_danShuangGroup->addButton(radioShuang, 1);
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
    // 添加到ButtonGroup实现单选互斥
    m_kickTeamGroup = new QButtonGroup(this);
    m_kickTeamGroup->addButton(radioAttack, 0);
    m_kickTeamGroup->addButton(radioDefend, 1);
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
    // 添加到ButtonGroup实现单选互斥
    m_areaGroup = new QButtonGroup(this);
    m_areaGroup->addButton(radioLeftArea, 0);
    m_areaGroup->addButton(radioRightArea, 1);
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
    radioMiddleDirect = new QRadioButton("随机直冲", this);
    radioMiddleDirect->setFont(font);
    radioMiddleDirect->setChecked(true);
    radioRightDirect = new QRadioButton("右晃射门", this);
    radioRightDirect->setFont(font);
    m_dqDirectGroup = new QButtonGroup(this);
    m_dqDirectGroup->addButton(radioLeftDirect, 0);
    m_dqDirectGroup->addButton(radioMiddleDirect, 1);
    m_dqDirectGroup->addButton(radioRightDirect, 2);
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
    radioCenterGoalkeeper = new QRadioButton("中", this);
    radioCenterGoalkeeper->setFont(font);
    radioCenterGoalkeeper->setChecked(true);
    radioRightGoalkeeper = new QRadioButton("右", this);
    radioRightGoalkeeper->setFont(font);
    m_goalkeeperGroup = new QButtonGroup(this);
    m_goalkeeperGroup->addButton(radioLeftGoalkeeper, 0);
    m_goalkeeperGroup->addButton(radioCenterGoalkeeper, 1);
    m_goalkeeperGroup->addButton(radioRightGoalkeeper, 2);
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
    btnPrepare = new QPushButton("初始预备", this);
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
    lineEditNewCar = new QLineEdit(this);
    lineEditNewCar->setFont(font);
    lineEditNewCar->setText("1");
    lineEditNewCar->setFixedWidth(50);
    QLabel *exchangeRoleLabel = new QLabel("替换角色", this);
    exchangeRoleLabel->setFont(font);
    lineEditExchangeRole = new QLineEdit(this);
    lineEditExchangeRole->setFont(font);
    lineEditExchangeRole->setText("1");
    lineEditExchangeRole->setFixedWidth(50);
    btnExchangeRole = new QPushButton("替换", this);
    btnExchangeRole->setFont(font);
    btnExchangeRole->setFixedWidth(60);

    roleExchangeLayout->addWidget(roleExchangeLabel);
    roleExchangeLayout->addWidget(newCarLabel);
    roleExchangeLayout->addWidget(lineEditNewCar);
    roleExchangeLayout->addWidget(exchangeRoleLabel);
    roleExchangeLayout->addWidget(lineEditExchangeRole);
    roleExchangeLayout->addWidget(btnExchangeRole);
    controlLayout->addLayout(roleExchangeLayout);

    // 在 controlLayout 底部添加弹性空间，让内容在垂直方向上铺满
    controlLayout->addStretch();

    // 将控制区域添加到主布局，并设置 stretch 因子使其填满剩余空间
    mainLayout->addLayout(controlLayout, 1);  // stretch 因子为 1

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
    connect(radioMiddleDirect, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioRightDirect, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioLeftGoalkeeper, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioCenterGoalkeeper, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
    connect(radioRightGoalkeeper, SIGNAL(clicked()), this, SLOT(onRadioButtonClicked()));
}

void MatchDlg_5vs5::onButtonStart()
{
    if (!m_strategyPlugin) {
        QMessageBox::warning(this, "错误", "策略插件未加载");
        Debug::get()->print("错误：策略插件未加载");
        return;
    }

    // 读取界面参数
    m_attack = radioAttack->isChecked() ? 1 : 0;
    m_area = radioLeftArea->isChecked() ? 0 : 1;
    m_dan = radioDan->isChecked() ? 0 : 1;
    m_kick = radioNormalKick->isChecked() ? 0 : (radioPenaltyKick->isChecked() ? 1 : 0);

    if (radioLeftDirect->isChecked())
        m_dqdirect = 0;
    else if (radioRightDirect->isChecked())
        m_dqdirect = 2;
    else
        m_dqdirect = 1;

    // 初始化策略
    m_strategyPlugin->initialize(0);

    // 应用比赛参数到插件
    applyMatchParameters();

    m_isMatchRunning = true;
    qDebug() << "Match started!";
    QMessageBox::information(this, "比赛开始", "比赛已开始");
    Debug::get()->print("比赛开始：比赛已启动，机器人进入比赛状态");
    
    // 禁用开始比赛和初始预备按钮
    btnStartMatch->setEnabled(false);
    if (btnPrepare) {
        btnPrepare->setEnabled(false);
    }
    
    // 归位选项设为false
    m_return2pt = false;
}

void MatchDlg_5vs5::onButtonStop()
{
    m_isMatchRunning = false;
    qDebug() << "Match stopped!";
    Debug::get()->print("停止：比赛已停止，机器人进入待命状态");
    
    // 启用初始预备按钮
    if (btnPrepare) {
        btnPrepare->setEnabled(true);
    }
}

void MatchDlg_5vs5::onStrategyChanged(int index)
{
    StrategyNum = index;

    if (m_strategyPlugin) {
        m_strategyPlugin->selectStrategy(index);
    }

    qDebug() << "Strategy changed to:" << index;
    Debug::get()->print(QString("策略选择：已切换到%1号策略").arg(index + 1));
}

void MatchDlg_5vs5::onButtonPrepare()
{
    if (!m_strategyPlugin) {
        QMessageBox::warning(this, "错误", "策略插件未加载");
        Debug::get()->print("错误：策略插件未加载");
        return;
    }

    // 根据单双后卫选择策略
    if (radioDan->isChecked()) {
        Debug::get()->print("策略选择：单后卫策略");
    } else {
        Debug::get()->print("策略选择：双后卫策略");
    }

    // 更新界面参数
    m_attack = radioAttack->isChecked() ? 1 : 0;
    m_area = radioLeftArea->isChecked() ? 0 : 1;
    m_dan = radioDan->isChecked() ? 0 : 1;
    m_kick = radioNormalKick->isChecked() ? 0 : (radioPenaltyKick->isChecked() ? 1 : 0);
    
    if (radioLeftDirect->isChecked())
        m_dqdirect = 0;
    else if (radioRightDirect->isChecked())
        m_dqdirect = 2;
    else
        m_dqdirect = 1;

    // 应用比赛参数到插件
    applyMatchParameters();

    // 机器人归位
    m_strategyPlugin->parkRobots();

    if (m_pDisplayDlg) {
        m_pDisplayDlg->ShowInitGame();
    }

    btnStartMatch->setEnabled(true);
    Debug::get()->print("初始预备：机器人已归位，比赛准备就绪");
}

void MatchDlg_5vs5::onButtonExchangeRole()
{
    if (!lineEditNewCar || !lineEditExchangeRole) {
        Debug::get()->print("错误：找不到输入控件");
        return;
    }
    
    int newCar = lineEditNewCar->text().toInt();
    int exchangeRole = lineEditExchangeRole->text().toInt();
    
    // 验证输入
    if (newCar < 1 || newCar > 5 || exchangeRole < 1 || exchangeRole > 5) {
        QMessageBox::warning(this, "错误", "输入值无效，请输入1-5之间的数字");
        Debug::get()->print("错误：角色替换输入值无效");
        return;
    }
    
    // 执行角色替换
    m_exchangerobot1 = newCar;
    m_exchangerobot2 = exchangeRole;
    
    // 显示提示
    QString message = QString("角色替换：将新车 %1 替换为角色 %2").arg(newCar).arg(exchangeRole);
    Debug::get()->print(message);
    QMessageBox::information(this, "角色替换", message);
}
void MatchDlg_5vs5::onRadioButtonClicked()
{
    // 更新数据变量
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

    // 实时应用设置到策略
    applyMatchParameters();
}