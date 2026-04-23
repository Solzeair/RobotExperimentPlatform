/*
* 调车对话框源文件
* 写作人 李青
* 功能 调车面板的构建与基础交互响应，含车体位置标识显示、指令下发遥测操作等功能模块实现。
* 已完成
*/
#include "RobotDlg.h"
#include "Debug.h"

// 定义USE_USB340宏，使用真实的设备函数
// #define USE_USB340

// 尝试包含USB340HID61_DEF.h，如果失败则使用模拟函数
#ifdef USE_USB340
#include "USB340HID61_DEF.h"
#else
// 模拟USB340设备初始化
bool InitUSB340()
{
    Debug::get()->print(L"模拟初始化USB340设备");
    return true;
}

// 模拟检测设备是否存在
bool CheckIfExist()
{
    Debug::get()->print(L"模拟检测设备是否存在");
    return true; // 假设设备存在
}

// 模拟设置频率
bool SetFre(int fre, bool op)
{
    Debug::get()->print(L"模拟设置频率");
    return true;
}

// 模拟修改车频率
bool ChangeCarFre(unsigned char CarNum, int NewCarFre, bool ChangeCarFreOp)
{
    Debug::get()->print(L"模拟修改车频率");
    return true;
}

// 模拟修改车号
bool ChangeCarNum(unsigned char OldNum, unsigned char NewNum)
{
    Debug::get()->print(L"模拟修改车号");
    return true;
}

// 模拟组装车的速度
bool BuildCarSpeed(unsigned char CarNum, int Left, int Right)
{
    Debug::get()->print(L"模拟组装车的速度");
    return true;
}

// 模拟发送所有车的速度
bool SendAll(int num)
{
    Debug::get()->print(L"模拟发送所有车的速度");
    return true;
}

// 模拟发送单辆车的速度
bool SendOneCar(int num)
{
    Debug::get()->print(L"模拟发送单辆车的速度");
    return true;
}
#endif

// 使用450频率
#define USE_FRE_450

RobotDlg::RobotDlg(QWidget *parent)
    : QWidget(parent)
    , m_oldNum(0)
    , m_newNum(0)
    , m_numSet(0)
    , m_carFre(true)
    , m_selectedFreq(450)
{
    // 初始化USB340设备
    InitUSB340();
    
    // 设置初始频率
#ifdef USE_FRE_450 
    m_carFre = true;
    SetFre(450, false);
#else
    m_carFre = false;
    SetFre(460, false);
#endif
    
    // 设置大小策略为可伸缩
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    initUI();
}

RobotDlg::~RobotDlg()
{}

void RobotDlg::initUI()
{
    // 设置字体为楷体，12号，加粗
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 0, 20, 20);
    mainLayout->setSpacing(15);
    
    // 创建顶部布局（左侧显示区域 + 右侧控制区域）
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(15);
    
    // 左侧显示区域
    QLabel *displayLabel = new QLabel(this);
    displayLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid black; }");
    displayLabel->setFont(font);
    displayLabel->setFixedSize(640, 480);
    
    // 右侧控制区域
    QVBoxLayout *controlLayout = new QVBoxLayout();
    // 设置控件之间间隔为12像素，让内容排列更松散
    controlLayout->setSpacing(12);
    
    // 标题 "调车"
    QLabel *titleLabel = new QLabel("调车", this);
    titleLabel->setFont(font);
    controlLayout->addWidget(titleLabel);
    controlLayout->setAlignment(titleLabel, Qt::AlignTop);
    
    // 车号设置
    QGroupBox *carNumGroup = new QGroupBox("车号设置", this);
    carNumGroup->setFont(font);
    
    QVBoxLayout *carNumLayout = new QVBoxLayout(carNumGroup);
    carNumLayout->setContentsMargins(10, 10, 10, 10);
    carNumLayout->setSpacing(10);
    
    QHBoxLayout *oldNumLayout = new QHBoxLayout();
    QLabel *oldNumLabel = new QLabel("车号", carNumGroup);
    oldNumLabel->setFont(font);
    editOldNum = new QLineEdit(carNumGroup);
    editOldNum->setFixedWidth(80);
    editOldNum->setAlignment(Qt::AlignCenter);
    editOldNum->setFont(font);
    editOldNum->setText("0");
    oldNumLayout->addWidget(oldNumLabel);
    oldNumLayout->addWidget(editOldNum);
    carNumLayout->addLayout(oldNumLayout);
    
    QHBoxLayout *newNumLayout = new QHBoxLayout();
    QLabel *newNumLabel = new QLabel("新车号", carNumGroup);
    newNumLabel->setFont(font);
    editNewNum = new QLineEdit(carNumGroup);
    editNewNum->setFixedWidth(80);
    editNewNum->setAlignment(Qt::AlignCenter);
    editNewNum->setFont(font);
    editNewNum->setText("0");
    newNumLayout->addWidget(newNumLabel);
    newNumLayout->addWidget(editNewNum);
    carNumLayout->addLayout(newNumLayout);
    
    btnChangeNum = new QPushButton("改车号", carNumGroup);
    btnChangeNum->setFont(font);
    carNumLayout->addWidget(btnChangeNum, 0, Qt::AlignCenter);
    
    controlLayout->addWidget(carNumGroup);
    controlLayout->addSpacing(38); // 添加1厘米的间隔
    
    // 控制
    QGroupBox *controlGroup = new QGroupBox("控制", this);
    controlGroup->setFont(font);
    
    QVBoxLayout *controlButtonsLayout = new QVBoxLayout(controlGroup);
    controlButtonsLayout->setContentsMargins(10, 10, 10, 10);
    controlButtonsLayout->setSpacing(5);
    
    btnFront = new QPushButton("前进", controlGroup);
    btnFront->setFont(font);
    controlButtonsLayout->addWidget(btnFront, 0, Qt::AlignCenter);
    
    QHBoxLayout *midButtonsLayout = new QHBoxLayout();
    btnLeft = new QPushButton("向左", controlGroup);
    btnLeft->setFont(font);
    btnStop = new QPushButton("停止", controlGroup);
    btnStop->setFont(font);
    btnRight = new QPushButton("向右", controlGroup);
    btnRight->setFont(font);
    midButtonsLayout->addWidget(btnLeft);
    midButtonsLayout->addWidget(btnStop);
    midButtonsLayout->addWidget(btnRight);
    controlButtonsLayout->addLayout(midButtonsLayout);
    
    btnBack = new QPushButton("后退", controlGroup);
    btnBack->setFont(font);
    controlButtonsLayout->addWidget(btnBack, 0, Qt::AlignCenter);
    
    controlLayout->addWidget(controlGroup);
    controlLayout->addSpacing(38); // 添加1厘米的间隔
    
    // 小车频率设置
    QGroupBox *carFreqGroup = new QGroupBox("小车频率设置", this);
    carFreqGroup->setFont(font);
    
    QHBoxLayout *carFreqLayout = new QHBoxLayout(carFreqGroup);
    carFreqLayout->setContentsMargins(10, 10, 10, 10);
    carFreqLayout->setSpacing(10);
    
    QLabel *carNumLabel = new QLabel("车号", carFreqGroup);
    carNumLabel->setFont(font);
    editNum = new QLineEdit(carFreqGroup);
    editNum->setFixedWidth(60);
    editNum->setAlignment(Qt::AlignCenter);
    editNum->setFont(font);
    editNum->setText("0");
    radio1_450 = new QRadioButton("450", carFreqGroup);
    radio1_450->setChecked(true);
    radio1_450->setFont(font);
    radio1_460 = new QRadioButton("460", carFreqGroup);
    radio1_460->setFont(font);
    btnChangeFreq = new QPushButton("修改", carFreqGroup);
    btnChangeFreq->setFont(font);
    
    carFreqLayout->addWidget(carNumLabel);
    carFreqLayout->addWidget(editNum);
    carFreqLayout->addWidget(radio1_450);
    carFreqLayout->addWidget(radio1_460);
    carFreqLayout->addWidget(btnChangeFreq);
    
    controlLayout->addWidget(carFreqGroup);
    controlLayout->addSpacing(38); // 添加1厘米的间隔
    
    // 发射器设置
    QGroupBox *deviceGroup = new QGroupBox("发射器设置", this);
    deviceGroup->setFont(font);
    
    QHBoxLayout *deviceLayout = new QHBoxLayout(deviceGroup);
    deviceLayout->setContentsMargins(10, 10, 10, 10);
    deviceLayout->setSpacing(10);
    
    QVBoxLayout *freqButtonsLayout = new QVBoxLayout();
    btn450 = new QPushButton("450", deviceGroup);
    btn450->setFont(font);
    btn450->setCheckable(true);
    btn450->setChecked(true);
    btn450->setStyleSheet("QPushButton { background-color: #00FFFF; } QPushButton:checked { background-color: #00FFFF; border: 2px solid #0000FF; }");
    btn460 = new QPushButton("460", deviceGroup);
    btn460->setFont(font);
    btn460->setCheckable(true);
    btn460->setStyleSheet("QPushButton { background-color: #00FFFF; } QPushButton:checked { background-color: #00FFFF; border: 2px solid #0000FF; }");
    btnConfirmFreq = new QPushButton("确定", deviceGroup);
    btnConfirmFreq->setFont(font);
    freqButtonsLayout->addWidget(btn450);
    freqButtonsLayout->addWidget(btn460);
    freqButtonsLayout->addWidget(btnConfirmFreq);
    
    QVBoxLayout *statusLayout = new QVBoxLayout();
    QLabel *deviceStatusLabel = new QLabel("发射器状态：", deviceGroup);
    deviceStatusLabel->setFont(font);
    editDeviceStatus = new QLineEdit(deviceGroup);
    editDeviceStatus->setFixedWidth(100);
    editDeviceStatus->setReadOnly(true);
    editDeviceStatus->setStyleSheet("QLineEdit { border: 1px solid #000000; background-color: #FFFFFF; }");
    editDeviceStatus->setFont(font);
    statusLayout->addWidget(deviceStatusLabel);
    statusLayout->addWidget(editDeviceStatus);
    
    deviceLayout->addLayout(freqButtonsLayout);
    deviceLayout->addLayout(statusLayout);
    
    controlLayout->addWidget(deviceGroup);
    
    topLayout->addLayout(controlLayout);
    mainLayout->addLayout(topLayout);
    
    // 初始化定时器
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    // timer->start(TIME_SPACE); // 暂时注释掉，需要时取消注释
    
    // 连接信号槽
    connect(btnFront, SIGNAL(clicked()), this, SLOT(onButtonFront()));
    connect(btnBack, SIGNAL(clicked()), this, SLOT(onButtonBack()));
    connect(btnLeft, SIGNAL(clicked()), this, SLOT(onButtonLeft()));
    connect(btnRight, SIGNAL(clicked()), this, SLOT(onButtonRight()));
    connect(btnStop, SIGNAL(clicked()), this, SLOT(onButtonStop()));
    connect(btnChangeNum, SIGNAL(clicked()), this, SLOT(onButtonChangeNum()));
    connect(btnChangeFreq, SIGNAL(clicked()), this, SLOT(onButtonChangeFreq()));
    connect(btn450, SIGNAL(clicked()), this, SLOT(onButton450()));
    connect(btn460, SIGNAL(clicked()), this, SLOT(onButton460()));
    connect(btnConfirmFreq, SIGNAL(clicked()), this, SLOT(onButtonConfirmFreq()));
    connect(radio1_450, SIGNAL(clicked()), this, SLOT(onRadio1450()));
    connect(radio1_460, SIGNAL(clicked()), this, SLOT(onRadio1460()));
    
    // 设置单选按钮状态
    radio1_450->setChecked(true);
    if(m_carFre)
        radio1_450->setChecked(true);
    else
        radio1_460->setChecked(true);
}

void RobotDlg::onButtonFront()
{
    m_oldNum = editOldNum->text().toInt();
    BuildCarSpeed(m_oldNum, speed, speed);
    if (m_oldNum == 0)
        SendAll(11);
    else
        SendOneCar(m_oldNum);
    Debug::get()->print(L"发送前进命令");
}

void RobotDlg::onButtonBack()
{
    m_oldNum = editOldNum->text().toInt();
    BuildCarSpeed(m_oldNum, -speed, -speed);
    if (m_oldNum == 0)
        SendAll(11);
    else
        SendOneCar(m_oldNum);
    Debug::get()->print(L"发送后退命令");
}

void RobotDlg::onButtonLeft()
{
    m_oldNum = editOldNum->text().toInt();
    BuildCarSpeed(m_oldNum, -speed, speed);
    if (m_oldNum == 0)
        SendAll(11);
    else
        SendOneCar(m_oldNum);
    Debug::get()->print(L"发送左转命令");
}

void RobotDlg::onButtonRight()
{
    m_oldNum = editOldNum->text().toInt();
    BuildCarSpeed(m_oldNum, speed, -speed);
    if (m_oldNum == 0)
        SendAll(11);
    else
        SendOneCar(m_oldNum);
    Debug::get()->print(L"发送右转命令");
}

void RobotDlg::onButtonStop()
{
    m_oldNum = editOldNum->text().toInt();
    BuildCarSpeed(m_oldNum, 0, 0);
    if (m_oldNum == 0)
        SendAll(11);
    else
        SendOneCar(m_oldNum);
    Debug::get()->print(L"发送停止命令");
}

void RobotDlg::onButtonChangeNum()
{
    m_oldNum = editOldNum->text().toInt();
    m_newNum = editNewNum->text().toInt();
    ChangeCarNum(m_oldNum, m_newNum);
    // 交换编号显示
    editOldNum->setText(QString::number(m_newNum));
    editNewNum->setText(QString::number(m_oldNum));
    // 交换变量值
    std::swap(m_oldNum, m_newNum);
    Debug::get()->print(L"发送更改编号命令");
}

void RobotDlg::onButtonChangeFreq()
{
    m_numSet = editNum->text().toInt();
    m_carFre = radio1_450->isChecked();
    InitUSB340();
    if (m_carFre)
    {
        ChangeCarFre(m_numSet, 450, true);
        Debug::get()->print(L"发送更改频率为450的命令");
    }
    else
    {
        ChangeCarFre(m_numSet, 460, true);
        Debug::get()->print(L"发送更改频率为460的命令");
    }
}

void RobotDlg::onButton450()
{
    // 记录选择的频率
    m_selectedFreq = 450;
    // 更新按钮状态
    btn450->setChecked(true);
    btn460->setChecked(false);
    Debug::get()->print(L"选择发射器频率为 450，点击确定后生效");
}

void RobotDlg::onButton460()
{
    // 记录选择的频率
    m_selectedFreq = 460;
    // 更新按钮状态
    btn450->setChecked(false);
    btn460->setChecked(true);
    Debug::get()->print(L"选择发射器频率为 460，点击确定后生效");
}

void RobotDlg::onButtonConfirmFreq()
{
    // 确认并应用选择的频率
    if (m_selectedFreq == 450)
    {
        SetFre(450, false);
        m_carFre = true;
        editDeviceStatus->setText("450");
        Debug::get()->print(L"发射器频率已确认为 450");
    }
    else if (m_selectedFreq == 460)
    {
        SetFre(460, false);
        m_carFre = false;
        editDeviceStatus->setText("460");
        Debug::get()->print(L"发射器频率已确认为 460");
    }
}

void RobotDlg::onTimer()
{
    // 检查设备是否存在
    if (CheckIfExist())
    {
        editDeviceStatus->setText("设备已连接");
    }
    else
    {
        editDeviceStatus->setText("设备已断开");
        Debug::get()->print(L"警告!设备连接已断开!");
    }
}

void RobotDlg::onRadio1450()
{
    m_carFre = true;
}

void RobotDlg::onRadio1460()
{
    m_carFre = false;
}
