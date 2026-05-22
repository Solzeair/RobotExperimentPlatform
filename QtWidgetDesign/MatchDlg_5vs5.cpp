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
#include <QLibrary> 
#include <QFile>         
#include <QThread>
#include <QCoreApplication>
#include <algorithm>  // 用于 std::max, std::min
#include "USB340ProxyClient.h" 

MatchDlg_5vs5::MatchDlg_5vs5(QWidget* parent)
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
    , btnStrategyParam(nullptr)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initUI();

    // ========== 直接加载策略 DLL ==========
    QString dllPath = "C:/Users/2dou/source/repos/STRATEGY/x64/Debug/RobotStrategyDll.dll";

    if (!QFile::exists(dllPath)) {
        Debug::get()->print("策略DLL不存在: " + dllPath);
        QMessageBox::warning(this, "错误", "策略DLL不存在\n" + dllPath);
        btnStartMatch->setEnabled(false);
        return;
    }

    Debug::get()->print("尝试加载: " + dllPath);

    QLibrary lib(dllPath);
    if (!lib.load()) {
        Debug::get()->print("加载 DLL 失败: " + lib.errorString());
        btnStartMatch->setEnabled(false);
        return;
    }

    // 解析函数指针
    CreateStrategyFunc createStrategy = (CreateStrategyFunc)lib.resolve("CreateStrategy");
    InitializeStrategyFunc initStrategy = (InitializeStrategyFunc)lib.resolve("InitializeStrategy");
    SetFormationTypeFunc setFormation = (SetFormationTypeFunc)lib.resolve("SetFormationType");
    SetOurGoalOnRightFunc setOurGoalOnRight = (SetOurGoalOnRightFunc)lib.resolve("setOurGoalOnRight");
    SetOurKickoffFunc setOurKickoff = (SetOurKickoffFunc)lib.resolve("setOurKickoff");
    SetPenaltyKickModeFunc setPenaltyKickMode = (SetPenaltyKickModeFunc)lib.resolve("SetPenaltyKickMode");
    SelectStrategyFunc selectStrategy = (SelectStrategyFunc)lib.resolve("SelectStrategy");
    SetParameterFunc setParameter = (SetParameterFunc)lib.resolve("setParameter");
    ParkRobotsFunc parkRobots = (ParkRobotsFunc)lib.resolve("ParkRobots");
    SetKickoffTypeFunc setKickoffType = (SetKickoffTypeFunc)lib.resolve("SetKickoffType");
    DecideFunc decide = (DecideFunc)lib.resolve("decide");

    m_decide = decide;

    if (!createStrategy || !initStrategy) {
        Debug::get()->print("解析函数失败");
        btnStartMatch->setEnabled(false);
        return;
    }

    // 创建策略实例
    m_strategyHandle = createStrategy();
    if (!m_strategyHandle) {
        Debug::get()->print("创建策略实例失败");
        btnStartMatch->setEnabled(false);
        return;
    }

    // 保存函数指针
    m_initStrategy = initStrategy;
    m_setFormation = setFormation;
    m_setOurGoalOnRight = setOurGoalOnRight;
    m_setOurKickoff = setOurKickoff;
    m_setPenaltyKickMode = setPenaltyKickMode;
    m_selectStrategy = selectStrategy;
    m_setParameter = setParameter;
    m_parkRobots = parkRobots;
    m_setKickoffType = setKickoffType;

    // 初始化策略
    m_initStrategy(m_strategyHandle, 0);

    Debug::get()->print("策略加载成功！");
    // ========== 初始化策略执行定时器 ==========
    m_strategyTimer = new QTimer(this);
    connect(m_strategyTimer, &QTimer::timeout, this, &MatchDlg_5vs5::onStrategyTimer);
    // 注意：定时器在开始比赛后才启动，不在构造函数中 start

    // 帧率显示定时器（可选）
    m_fpsTimer = new QTimer(this);
    connect(m_fpsTimer, &QTimer::timeout, this, &MatchDlg_5vs5::onUpdateFPS);
    m_fpsTimer->start(500);  // 每500ms更新一次帧率显示

    m_hasValidData = false;

    // 初始状态：禁用开始比赛按钮
    btnStartMatch->setEnabled(false);
}MatchDlg_5vs5::~MatchDlg_5vs5()
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
    if (!m_strategyHandle) {  // 检查策略句柄
        return;
    }
    // 阵型 (0=单后卫, 1=双后卫)
    if (m_setFormation) m_setFormation(m_strategyHandle, m_dan);

    // 球门方向 (左半场=0, 右半场=1)
    if (m_setOurGoalOnRight) m_setOurGoalOnRight(m_strategyHandle, m_area == 1 ? 1 : 0);

    // 开球方 (我方=1, 对方=0)
    if (m_setOurKickoff) m_setOurKickoff(m_strategyHandle, m_attack);

    // 点球模式
    if (m_setPenaltyKickMode) m_setPenaltyKickMode(m_strategyHandle, m_dqdirect, m_dqsmd);

    //开球方式 
    if (m_setKickoffType) {
        m_setKickoffType(m_strategyHandle, m_kick);
    }

    // 策略选择
    if (m_selectStrategy) m_selectStrategy(m_strategyHandle, StrategyNum);
    // 设置归位参数
    if (m_setParameter) {
        m_setParameter(m_strategyHandle, "return2pt", m_return2pt ? 1.0 : 0.0);
    }
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
    radioGoalKick = new QRadioButton("门球", kickGroupBox);
    radioGoalKick->setFont(font);
    radioFreeKick = new QRadioButton("任意球", kickGroupBox);
    radioFreeKick->setFont(font);
    radioFreeBall = new QRadioButton("争球", kickGroupBox);
    radioFreeBall->setFont(font);
    radioShouqiu = new QRadioButton("收车", kickGroupBox);
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
    comboStrategy->addItem("3号策略");
    comboStrategy->addItem("4号策略");
    comboStrategy->setFont(font);
    strategyLayout->addWidget(strategyLabel);
    strategyLayout->addWidget(comboStrategy);
    controlLayout->addLayout(strategyLayout);

    // 策略参数修改
    QHBoxLayout *strategyParamLayout = new QHBoxLayout();
    QLabel *strategyParamLabel = new QLabel("策略参数修改", this);
    strategyParamLabel->setFont(font);
    btnStrategyParam = new QPushButton("打开参数配置", this);
    btnStrategyParam->setFont(font);
    btnStrategyParam->setFixedWidth(120);

    strategyParamLayout->addWidget(strategyParamLabel);
    strategyParamLayout->addWidget(btnStrategyParam);
    strategyParamLayout->addStretch();
    controlLayout->addLayout(strategyParamLayout);

    // 在 controlLayout 底部添加弹性空间，让内容在垂直方向上铺满
    controlLayout->addStretch();

    // 将控制区域添加到主布局，并设置 stretch 因子使其填满剩余空间
    mainLayout->addLayout(controlLayout, 1);  // stretch 因子为 1

    // 连接信号槽
    connect(btnStartMatch, SIGNAL(clicked()), this, SLOT(onButtonStart()));
    connect(btnStopMatch, SIGNAL(clicked()), this, SLOT(onButtonStop()));
    connect(comboStrategy, SIGNAL(currentIndexChanged(int)), this, SLOT(onStrategyChanged(int)));
    connect(btnPrepare, SIGNAL(clicked()), this, SLOT(onButtonPrepare()));
    connect(btnStrategyParam, SIGNAL(clicked()), this, SLOT(onButtonStrategyParam()));

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
    if (!m_strategyHandle || !m_initStrategy) {
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
    m_initStrategy(m_strategyHandle, 0);

    // 应用比赛参数到插件
    applyMatchParameters();

    m_isMatchRunning = true;
    qDebug() << "Match started!";
    QMessageBox::information(this, "比赛开始", "比赛已开始");
    Debug::get()->print("比赛开始：比赛已启动，机器人进入比赛状态");
    // 启动策略执行定时器（约30fps，33ms一帧）
    if (m_strategyTimer) {
        m_strategyTimer->start(33);  // 33ms = 约30帧/秒
        Debug::get()->print("策略循环已启动，频率约30Hz");
    }
    
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
    // 停止策略定时器
    if (m_strategyTimer) {
        m_strategyTimer->stop();
        Debug::get()->print("策略循环已停止");
    }

    // 发送停止命令给所有机器人
    USB340ProxyClient* proxy = USB340ProxyClient::getInstance();
    for (int i = 1; i <= 5; i++) {
        proxy->buildCarSpeed(i, 0, 0, 100);
        proxy->sendOneCar(i);
    }
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

    if (m_selectStrategy && m_strategyHandle) {
        m_selectStrategy(m_strategyHandle, index);
    }

    qDebug() << "Strategy changed to:" << index;
    Debug::get()->print(QString("策略选择：已切换到%1号策略").arg(index + 1));
}

void MatchDlg_5vs5::onButtonPrepare()
{
    // 机器人归位
    if (!m_strategyHandle || !m_parkRobots) {
        QMessageBox::warning(this, "错误", "策略插件未加载");
        Debug::get()->print("错误：策略插件未加载");
        return;
    }
    // 机器人归位
    m_parkRobots(m_strategyHandle);
    // 根据单双后卫选择策略
    if (radioDan->isChecked()) {
        Debug::get()->print("策略选择：单后卫策略");
    }
    else {
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
    // 等待 DisplayDlg 识别数据就绪
    if (m_pDisplayDlg) {
        m_pDisplayDlg->ShowInitGame();
        // 等待识别数据就绪
        for (int i = 0; i < 5; i++) {
            updateDisplayDlgData();
            QCoreApplication::processEvents();
            QThread::msleep(50);
            if (m_hasValidData) break;
        }
        // 先获取几帧数据，让识别稳定
        for (int i = 0; i < 5; i++) {
            updateDisplayDlgData();
            QCoreApplication::processEvents();  // 处理事件队列
            QThread::msleep(50);  // 等待50ms
            if (m_hasValidData) break;
        }
 
        if (!m_hasValidData) {
            Debug::get()->print("警告：识别数据未就绪，请检查标定和采色设置");
        }
        else {
            Debug::get()->print("识别数据已就绪");
        }
    }
    btnStartMatch->setEnabled(true);
    Debug::get()->print("初始预备：机器人已归位，比赛准备就绪");
}

void MatchDlg_5vs5::onButtonStrategyParam()
{
    QString exePath = "C:/Users/2dou/source/repos/STRATEGY/x64/Debug/ParameterDialog.exe";

    if (!QFile::exists(exePath)) {
        QMessageBox::warning(this, "错误", "策略参数配置程序不存在\n路径：" + exePath);
        Debug::get()->print("错误：策略参数配置程序不存在");
        return;
    }

    QProcess* process = new QProcess(this);
    bool started = process->startDetached(exePath);

    if (started) {
        Debug::get()->print("策略参数配置程序已启动");
    }
    else {
        QMessageBox::warning(this, "错误", "无法启动策略参数配置程序");
        Debug::get()->print("错误：无法启动策略参数配置程序");
    }
}
void MatchDlg_5vs5::onRadioButtonClicked()
{
    // 单双后卫
    if (radioDan->isChecked())
        m_dan = 0;
    else if (radioShuang->isChecked())
        m_dan = 1;

    // 开球方
    if (radioAttack->isChecked())
        m_attack = 1;
    else if (radioDefend->isChecked())
        m_attack = 0;

    // 左右半场
    if (radioLeftArea->isChecked())
        m_area = 0;
    else if (radioRightArea->isChecked())
        m_area = 1;

    // 开球方式（补全所有选项）
    if (radioNormalKick->isChecked())
        m_kick = 0;
    else if (radioPenaltyKick->isChecked())
        m_kick = 1;
    else if (radioGoalKick && radioGoalKick->isChecked())  // 门球
        m_kick = 2;
    else if (radioFreeKick && radioFreeKick->isChecked())  // 任意球
        m_kick = 3;
    else if (radioFreeBall && radioFreeBall->isChecked())  // 争球
        m_kick = 4;
    else if (radioShouqiu && radioShouqiu->isChecked())    // 收车
        m_kick = 5;

    // 点球方向
    if (radioLeftDirect->isChecked())
        m_dqdirect = 0;
    else if (radioRightDirect->isChecked())
        m_dqdirect = 2;
    else
        m_dqdirect = 1;

    // 实时应用设置到策略
    applyMatchParameters();
} 
void MatchDlg_5vs5::updateDisplayDlgData()
{
    if (!m_pDisplayDlg) return;

    // 获取己方机器人数据
    for (int i = 0; i < 5; i++) {
        m_cachedRobots[i].x = m_pDisplayDlg->getRobotX(i);
        m_cachedRobots[i].y = m_pDisplayDlg->getRobotY(i);
        m_cachedRobots[i].theta = m_pDisplayDlg->getRobotTheta(i);
        m_cachedRobots[i].vx = 0;   // 速度信息需要从历史计算，暂时设0
        m_cachedRobots[i].vy = 0;
        m_cachedRobots[i].vtheta = 0;

        // 获取对方机器人数据
        m_cachedOppRobots[i].x = m_pDisplayDlg->getOppRobotX(i);
        m_cachedOppRobots[i].y = m_pDisplayDlg->getOppRobotY(i);
    }

    // 获取球的数据
    m_cachedBall.pos.x = m_pDisplayDlg->getBallX();
    m_cachedBall.pos.y = m_pDisplayDlg->getBallY();
    m_cachedBall.vel_x = 0;   // 速度需要从历史计算，暂时设0
    m_cachedBall.vel_y = 0;
    m_cachedBall.velocity = 0;
    m_cachedBall.angle = 0;

    // 检查数据有效性
    m_hasValidData = m_pDisplayDlg->isDataReady();
}
void MatchDlg_5vs5::sendVelocitiesToRobots(const WheelVelocity velocities[5])
{// 先注释掉，只打印日志
    for (int i = 0; i < 5; i++) {
        Debug::get()->print(QString("Robot %1: L=%2, R=%3")
            .arg(i + 1)
            .arg(velocities[i].left, 0, 'f', 1)
            .arg(velocities[i].right, 0, 'f', 1));
    }
    USB340ProxyClient* proxy = USB340ProxyClient::getInstance();

   for (int i = 0; i < 5; i++) {
       int carNum = i + 1;  // 车号从1开始
        // 轮速单位：cm/s，这里直接使用策略计算的值
        int leftSpeed = (int)velocities[i].left;
        int rightSpeed = (int)velocities[i].right;

        // 限制速度范围
       leftSpeed = std::max(-100, std::min(100, leftSpeed));
       rightSpeed = std::max(-100, std::min(100, rightSpeed));

       proxy->buildCarSpeed(carNum, leftSpeed, rightSpeed, 100);
        proxy->sendOneCar(carNum);
    }
}
void MatchDlg_5vs5::onStrategyTimer()
{
    // 检查策略是否运行中
    if (!m_isMatchRunning) return;

    // 检查 DLL 函数是否可用
    if (!m_strategyHandle || !m_decide) return;

    // 从 DisplayDlg 获取最新的识别数据
    updateDisplayDlgData();

    // 如果数据无效，跳过这一帧（等待识别稳定）
    if (!m_hasValidData) {
        // Debug::get()->print("等待识别数据...");
        return;
    }

    // 准备轮速输出数组
    WheelVelocity velocities[5];

    // 调用 DLL 的决策函数
    m_decide(m_strategyHandle,
        m_cachedRobots,      // 己方机器人位姿
        m_cachedOppRobots,   // 对方机器人位置
        &m_cachedBall,       // 球的信息
        velocities);         // 输出轮速

    // 将计算出的轮速发送给机器人
    sendVelocitiesToRobots(velocities);

    // 可选：输出调试信息
    static int frameCount = 0;
    if (++frameCount % 30 == 0) {  // 每30帧输出一次
        Debug::get()->print(QString("策略执行中 - 球位置: (%1, %2)")
            .arg(m_cachedBall.pos.x, 0, 'f', 1)
            .arg(m_cachedBall.pos.y, 0, 'f', 1));
    }
}
void MatchDlg_5vs5::onUpdateFPS()
{
    if (m_pDisplayDlg) {
        // 可以在这里更新界面上的帧率显示
        // 或者什么都不做，只是保持定时器运行
    }
}