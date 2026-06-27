/*
* 调车对话框源文件
* 调车面板的构建与基础交互响应，含车体位置标识显示、指令下发遥测操作等功能模块实现。
*/
#include "RobotDlg.h"
#include "Debug.h"
#include "USB340ProxyClient.h"
#include <QCloseEvent>

// 通过宏选择工作频率，当前默认 450
#define USE_FRE_450

RobotDlg::RobotDlg(QWidget *parent)
    : QWidget(parent)
    , m_oldNum(0)
    , m_newNum(0)
    , m_numSet(0)
    , m_carFre(true)
    , m_selectedFreq(450)
{
    // 初始化USB340硬件代理，失败仅告警不阻断界面启动
    USB340ProxyClient* proxy = USB340ProxyClient::getInstance();
    if (!proxy->init()) {
        Debug::get()->print(L"USB340代理初始化失败");
    }
    
    // 根据宏切换发射器初始频率并同步车体频率标志
#ifdef USE_FRE_450 
    m_carFre = true;
    proxy->setFre(450, false);
#else
    m_carFre = false;
    proxy->setFre(460, false);
#endif
    
    // 面板随父容器伸缩，适配不同窗口尺寸
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    initUI();
}

RobotDlg::~RobotDlg()
{}

void RobotDlg::initUI()
{
    // 全局字体统一为楷体加粗，保证面板视觉一致
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);
    
    // 主布局统一定义外边距与行距，与其它界面风格对齐
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 0, 20, 20);
    mainLayout->setSpacing(15);
    
    // 控制区域聚合各功能分组，独立布局以便统一管理间距
    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->setSpacing(12);
    
    QLabel *titleLabel = new QLabel("调车", this);
    titleLabel->setFont(font);
    controlLayout->addWidget(titleLabel);
    controlLayout->setAlignment(titleLabel, Qt::AlignTop);
    
    // 车号设置分组：原车号录入、新车号录入与改号触发
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
    controlLayout->addSpacing(38); // 分组之间预留约1cm视觉间隔
    
    // 运动控制分组：上下前后与停止按钮，构成方向操纵十字布局
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
    controlLayout->addSpacing(38); // 分组之间预留约1cm视觉间隔
    
    // 小车频率设置分组：按车号切换 450/460 载波频率
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
    controlLayout->addSpacing(38); // 分组之间预留约1cm视觉间隔
    
    // 发射器设置分组：选择发射器载波频率并显示连接状态
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

    // 控制区域整体并入主布局
    mainLayout->addLayout(controlLayout);

    // 定时轮询设备连接状态；默认未启动，需要时取消下方死代码注释
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    // timer->start(TIME_SPACE); // 暂时注释掉，需要时取消注释

    // 绑定各按钮与频率单选的信号槽至对应处理槽
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

    // 按当前车体频率标志同步发射器按钮选中态
    btn450->setChecked(true);
    if(m_carFre)
        btn450->setChecked(true);
    else
        btn460->setChecked(true);

    // 末尾弹性占位，使内容顶部对齐、垂直方向自适应拉伸
    mainLayout->addStretch();
}

void RobotDlg::onButtonFront()
{
    // 车号0表示广播：下发全队(11辆)同速前进，否则仅驱动指定单车
    USB340ProxyClient* proxy = USB340ProxyClient::getInstance();
    m_oldNum = editOldNum->text().toInt();
    proxy->buildCarSpeed(m_oldNum, speed, speed, 100);
    if (m_oldNum == 0)
        proxy->sendAll(11);
    else
        proxy->sendOneCar(m_oldNum);
    Debug::get()->print(L"发送前进命令");
}

void RobotDlg::onButtonBack()
{
    // 两侧轮速取负实现后退；车号0为广播
    USB340ProxyClient* proxy = USB340ProxyClient::getInstance();
    m_oldNum = editOldNum->text().toInt();
    proxy->buildCarSpeed(m_oldNum, -speed, -speed, 100);
    if (m_oldNum == 0)
        proxy->sendAll(11);
    else
        proxy->sendOneCar(m_oldNum);
    Debug::get()->print(L"发送后退命令");
}
void RobotDlg::onButtonLeft()
{
    // 左右轮反向驱动实现原地左转
    USB340ProxyClient* proxy = USB340ProxyClient::getInstance();
    m_oldNum = editOldNum->text().toInt();
    proxy->buildCarSpeed(m_oldNum, -speed, speed, 100);
    if (m_oldNum == 0)
        proxy->sendAll(11);
    else
        proxy->sendOneCar(m_oldNum);
    Debug::get()->print(L"发送左转命令");
}

void RobotDlg::onButtonRight()
{
    // 左右轮反向驱动实现原地右转
    USB340ProxyClient* proxy = USB340ProxyClient::getInstance();
    m_oldNum = editOldNum->text().toInt();
    proxy->buildCarSpeed(m_oldNum, speed, -speed, 100);
    if (m_oldNum == 0)
        proxy->sendAll(11);
    else
        proxy->sendOneCar(m_oldNum);
    Debug::get()->print(L"发送右转命令");
}

void RobotDlg::onButtonStop()
{
    // 两侧轮速归零实现急停
    USB340ProxyClient* proxy = USB340ProxyClient::getInstance();
    m_oldNum = editOldNum->text().toInt();
    proxy->buildCarSpeed(m_oldNum, 0, 0, 100);
    if (m_oldNum == 0)
        proxy->sendAll(11);
    else
        proxy->sendOneCar(m_oldNum);
    Debug::get()->print(L"发送停止命令");
}

void RobotDlg::onButtonChangeNum()
{
    USB340ProxyClient* proxy = USB340ProxyClient::getInstance();
    m_oldNum = editOldNum->text().toInt();
    m_newNum = editNewNum->text().toInt();
    Debug::get()->print(QString("改车号: %1 -> %2").arg(m_oldNum).arg(m_newNum));
    bool success = proxy->changeCarNum(m_oldNum, m_newNum);
    if (!success) {
        Debug::get()->print(QString("改车号命令发送失败"));
    }
    // 改号成功后回填输入框并交换内部编号，便于连续改号操作
    editOldNum->setText(QString::number(m_newNum));
    editNewNum->setText(QString::number(m_oldNum));
    std::swap(m_oldNum, m_newNum);
}

void RobotDlg::onButtonChangeFreq()
{
    // 修改指定车号的载波频率，调用前重新init确保代理处于就绪态
    USB340ProxyClient* proxy = USB340ProxyClient::getInstance();
    m_numSet = editNum->text().toInt();
    m_carFre = radio1_450->isChecked();
    proxy->init();
    if (m_carFre)
    {
        proxy->changeCarFre(m_numSet, true);
        Debug::get()->print(L"发送更改频率为450的命令");
    }
    else
    {
        proxy->changeCarFre(m_numSet, false);
        Debug::get()->print(L"发送更改频率为460的命令");
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

void RobotDlg::onButton450()
{
    // 仅暂存选中频率并刷新按钮选中态，实际切换待点击确定
    m_selectedFreq = 450;
    btn450->setChecked(true);
    btn460->setChecked(false);
    Debug::get()->print(L"选择发射器频率为 450，点击确定后生效");
}

void RobotDlg::onButton460()
{
    // 仅暂存选中频率并刷新按钮选中态，实际切换待点击确定
    m_selectedFreq = 460;
    btn450->setChecked(false);
    btn460->setChecked(true);
    Debug::get()->print(L"选择发射器频率为 460，点击确定后生效");
}

void RobotDlg::onButtonConfirmFreq()
{
    // 确认后真正下发发射器频率并同步车体频率标志与状态显示
    USB340ProxyClient* proxy = USB340ProxyClient::getInstance();
    if (m_selectedFreq == 450)
    {
        proxy->setFre(450, false);
        m_carFre = true;
        editDeviceStatus->setText("450");
        Debug::get()->print(L"发射器频率已确认为 450");
    }
    else if (m_selectedFreq == 460)
    {
        proxy->setFre(460, false);
        m_carFre = false;
        editDeviceStatus->setText("460");
        Debug::get()->print(L"发射器频率已确认为 460");
    }
}

void RobotDlg::onTimer()
{
    USB340ProxyClient* proxy = USB340ProxyClient::getInstance();
    if (proxy->checkIfExist())
    {
        editDeviceStatus->setText("设备已连接");
    }
    else
    {
        editDeviceStatus->setText("设备已断开");
        Debug::get()->print(L"警告!设备连接已断开!");
    }
}

void RobotDlg::closeEvent(QCloseEvent* event)
{
    // 关闭前停止定时器，避免对话框销毁后槽函数触发空指针访问
    if (timer)
    {
        timer->stop();
    }
    event->accept();
}
