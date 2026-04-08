#include "RobotDlg.h"

RobotDlg::RobotDlg(QWidget *parent)
    : QWidget(parent)
    , m_oldNum(0)
    , m_newNum(0)
    , m_numSet(0)
    , m_carFre(true)
{
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
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // 标题 "调车"
    QLabel *titleLabel = new QLabel("调车", this);
    titleLabel->setFont(font);
    mainLayout->addWidget(titleLabel);
    
    // 创建顶部布局（左侧显示区域 + 右侧控制区域）
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(20);
    
    // 左侧显示区域
    QLabel *displayLabel = new QLabel(this);
    displayLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid black; }");
    displayLabel->setFont(font);
    displayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 机器人编号显示区域 - 使用网格布局
    QGridLayout *robotGridLayout = new QGridLayout(displayLabel);
    robotGridLayout->setContentsMargins(30, 30, 30, 30);
    robotGridLayout->setSpacing(30);
    
    // 1号机器人
    QLabel *robot1Label = new QLabel(displayLabel);
    robot1Label->setFixedSize(140, 140);
    robot1Label->setStyleSheet("QLabel { background-color: #333333; border: 1px solid #000000; }");
    
    // 左侧黑色
    QLabel *robot1LeftColorLabel = new QLabel(robot1Label);
    robot1LeftColorLabel->setGeometry(0, 0, 40, 140);
    robot1LeftColorLabel->setStyleSheet("QLabel { background-color: #000000; }");
    
    // 中间黄色
    QLabel *robot1NumLabel = new QLabel("1", robot1Label);
    robot1NumLabel->setGeometry(40, 0, 70, 140);
    robot1NumLabel->setStyleSheet("QLabel { background-color: #FFFF00; color: #000000; font-size: 56px; font-weight: bold; text-align: center; }");
    robot1NumLabel->setAlignment(Qt::AlignCenter);
    
    // 右侧上面黑色，下面紫色
    QLabel *robot1RightTopColorLabel = new QLabel(robot1Label);
    robot1RightTopColorLabel->setGeometry(110, 0, 30, 70);
    robot1RightTopColorLabel->setStyleSheet("QLabel { background-color: #000000; border-bottom: 1px solid white; }");
    
    QLabel *robot1RightBottomColorLabel = new QLabel(robot1Label);
    robot1RightBottomColorLabel->setGeometry(110, 70, 30, 70);
    robot1RightBottomColorLabel->setStyleSheet("QLabel { background-color: #FF00FF; }");
    
    // 2号机器人
    QLabel *robot2Label = new QLabel(displayLabel);
    robot2Label->setFixedSize(140, 140);
    robot2Label->setStyleSheet("QLabel { background-color: #333333; border: 1px solid #000000; }");
    
    // 左侧黑色
    QLabel *robot2LeftColorLabel = new QLabel(robot2Label);
    robot2LeftColorLabel->setGeometry(0, 0, 40, 140);
    robot2LeftColorLabel->setStyleSheet("QLabel { background-color: #000000; }");
    
    // 中间黄色
    QLabel *robot2NumLabel = new QLabel("2", robot2Label);
    robot2NumLabel->setGeometry(40, 0, 70, 140);
    robot2NumLabel->setStyleSheet("QLabel { background-color: #FFFF00; color: #000000; font-size: 56px; font-weight: bold; text-align: center; }");
    robot2NumLabel->setAlignment(Qt::AlignCenter);
    
    // 右侧上面紫色，下面黑色
    QLabel *robot2RightTopColorLabel = new QLabel(robot2Label);
    robot2RightTopColorLabel->setGeometry(110, 0, 30, 70);
    robot2RightTopColorLabel->setStyleSheet("QLabel { background-color: #FF00FF; border-bottom: 1px solid white; }");
    
    QLabel *robot2RightBottomColorLabel = new QLabel(robot2Label);
    robot2RightBottomColorLabel->setGeometry(110, 70, 30, 70);
    robot2RightBottomColorLabel->setStyleSheet("QLabel { background-color: #000000; }");
    
    // 3号机器人
    QLabel *robot3Label = new QLabel(displayLabel);
    robot3Label->setFixedSize(140, 140);
    robot3Label->setStyleSheet("QLabel { background-color: #333333; border: 1px solid #000000; }");
    
    // 左侧黑色
    QLabel *robot3LeftColorLabel = new QLabel(robot3Label);
    robot3LeftColorLabel->setGeometry(0, 0, 40, 140);
    robot3LeftColorLabel->setStyleSheet("QLabel { background-color: #000000; }");
    
    // 中间黄色
    QLabel *robot3NumLabel = new QLabel("3", robot3Label);
    robot3NumLabel->setGeometry(40, 0, 70, 140);
    robot3NumLabel->setStyleSheet("QLabel { background-color: #FFFF00; color: #000000; font-size: 56px; font-weight: bold; text-align: center; }");
    robot3NumLabel->setAlignment(Qt::AlignCenter);
    
    // 右侧全部紫色，添加分界线
    QLabel *robot3RightTopColorLabel = new QLabel(robot3Label);
    robot3RightTopColorLabel->setGeometry(110, 0, 30, 70);
    robot3RightTopColorLabel->setStyleSheet("QLabel { background-color: #FF00FF; border-bottom: 1px solid white; }");
    
    QLabel *robot3RightBottomColorLabel = new QLabel(robot3Label);
    robot3RightBottomColorLabel->setGeometry(110, 70, 30, 70);
    robot3RightBottomColorLabel->setStyleSheet("QLabel { background-color: #FF00FF; }");
    
    // 4号机器人
    QLabel *robot4Label = new QLabel(displayLabel);
    robot4Label->setFixedSize(140, 140);
    robot4Label->setStyleSheet("QLabel { background-color: #333333; border: 1px solid #000000; }");
    
    // 左侧黑色
    QLabel *robot4LeftColorLabel = new QLabel(robot4Label);
    robot4LeftColorLabel->setGeometry(0, 0, 40, 140);
    robot4LeftColorLabel->setStyleSheet("QLabel { background-color: #000000; }");
    
    // 中间黄色
    QLabel *robot4NumLabel = new QLabel("4", robot4Label);
    robot4NumLabel->setGeometry(40, 0, 70, 140);
    robot4NumLabel->setStyleSheet("QLabel { background-color: #FFFF00; color: #000000; font-size: 56px; font-weight: bold; text-align: center; }");
    robot4NumLabel->setAlignment(Qt::AlignCenter);
    
    // 右侧上面黑色，下面绿色
    QLabel *robot4RightTopColorLabel = new QLabel(robot4Label);
    robot4RightTopColorLabel->setGeometry(110, 0, 30, 70);
    robot4RightTopColorLabel->setStyleSheet("QLabel { background-color: #000000; border-bottom: 1px solid white; }");
    
    QLabel *robot4RightBottomColorLabel = new QLabel(robot4Label);
    robot4RightBottomColorLabel->setGeometry(110, 70, 30, 70);
    robot4RightBottomColorLabel->setStyleSheet("QLabel { background-color: #00FF00; }");
    
    // 5号机器人
    QLabel *robot5Label = new QLabel(displayLabel);
    robot5Label->setFixedSize(140, 140);
    robot5Label->setStyleSheet("QLabel { background-color: #333333; border: 1px solid #000000; }");
    
    // 左侧黑色
    QLabel *robot5LeftColorLabel = new QLabel(robot5Label);
    robot5LeftColorLabel->setGeometry(0, 0, 40, 140);
    robot5LeftColorLabel->setStyleSheet("QLabel { background-color: #000000; }");
    
    // 中间黄色
    QLabel *robot5NumLabel = new QLabel("5", robot5Label);
    robot5NumLabel->setGeometry(40, 0, 70, 140);
    robot5NumLabel->setStyleSheet("QLabel { background-color: #FFFF00; color: #000000; font-size: 56px; font-weight: bold; text-align: center; }");
    robot5NumLabel->setAlignment(Qt::AlignCenter);
    
    // 右侧上面绿色，下面黑色
    QLabel *robot5RightTopColorLabel = new QLabel(robot5Label);
    robot5RightTopColorLabel->setGeometry(110, 0, 30, 70);
    robot5RightTopColorLabel->setStyleSheet("QLabel { background-color: #00FF00; border-bottom: 1px solid white; }");
    
    QLabel *robot5RightBottomColorLabel = new QLabel(robot5Label);
    robot5RightBottomColorLabel->setGeometry(110, 70, 30, 70);
    robot5RightBottomColorLabel->setStyleSheet("QLabel { background-color: #000000; }");
    
    // 添加机器人到网格布局
    robotGridLayout->addWidget(robot1Label, 0, 0);
    robotGridLayout->addWidget(robot2Label, 0, 1);
    robotGridLayout->addWidget(robot3Label, 0, 2);
    robotGridLayout->addWidget(robot4Label, 1, 0);
    robotGridLayout->addWidget(robot5Label, 1, 1);
    
    topLayout->addWidget(displayLabel);
    
    // 右侧控制区域
    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->setSpacing(15);
    
    // 车号设置
    QGroupBox *carNumGroup = new QGroupBox("车号设置", this);
    carNumGroup->setFont(font);
    
    QVBoxLayout *carNumLayout = new QVBoxLayout(carNumGroup);
    carNumLayout->setContentsMargins(20, 20, 20, 20);
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
    
    // 控制
    QGroupBox *controlGroup = new QGroupBox("控制", this);
    controlGroup->setFont(font);
    
    QVBoxLayout *controlButtonsLayout = new QVBoxLayout(controlGroup);
    controlButtonsLayout->setContentsMargins(20, 20, 20, 20);
    controlButtonsLayout->setSpacing(10);
    
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
    
    // 小车频率设置
    QGroupBox *carFreqGroup = new QGroupBox("小车频率设置", this);
    carFreqGroup->setFont(font);
    
    QHBoxLayout *carFreqLayout = new QHBoxLayout(carFreqGroup);
    carFreqLayout->setContentsMargins(20, 20, 20, 20);
    carFreqLayout->setSpacing(15);
    
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
    
    // 发射器设置
    QGroupBox *deviceGroup = new QGroupBox("发射器设置", this);
    deviceGroup->setFont(font);
    
    QHBoxLayout *deviceLayout = new QHBoxLayout(deviceGroup);
    deviceLayout->setContentsMargins(20, 20, 20, 20);
    deviceLayout->setSpacing(15);
    
    QVBoxLayout *freqButtonsLayout = new QVBoxLayout();
    QPushButton *btn450 = new QPushButton("450", deviceGroup);
    btn450->setFont(font);
    btn450->setStyleSheet("QPushButton { background-color: #00FFFF; }");
    QPushButton *btn460 = new QPushButton("460", deviceGroup);
    btn460->setFont(font);
    freqButtonsLayout->addWidget(btn450);
    freqButtonsLayout->addWidget(btn460);
    
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
    
    // 输出区域
    QLabel *outputLabel = new QLabel(this);
    outputLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid black; font-family: Consolas; font-size: 10pt; }");
    outputLabel->setFont(font);
    outputLabel->setText("输出区域:");
    outputLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    outputLabel->setFixedHeight(100);
    mainLayout->addWidget(outputLabel);
    
    // 连接信号槽
    connect(btnFront, SIGNAL(clicked()), this, SLOT(onButtonFront()));
    connect(btnBack, SIGNAL(clicked()), this, SLOT(onButtonBack()));
    connect(btnLeft, SIGNAL(clicked()), this, SLOT(onButtonLeft()));
    connect(btnRight, SIGNAL(clicked()), this, SLOT(onButtonRight()));
    connect(btnStop, SIGNAL(clicked()), this, SLOT(onButtonStop()));
    connect(btnChangeNum, SIGNAL(clicked()), this, SLOT(onButtonChangeNum()));
    connect(btnChangeFreq, SIGNAL(clicked()), this, SLOT(onButtonChangeFreq()));
}

void RobotDlg::onButtonFront()
{
    m_oldNum = editOldNum->text().toInt();
    // TODO: 发送前进命令
}

void RobotDlg::onButtonBack()
{
    m_oldNum = editOldNum->text().toInt();
    // TODO: 发送后退命令
}

void RobotDlg::onButtonLeft()
{
    m_oldNum = editOldNum->text().toInt();
    // TODO: 发送左转命令
}

void RobotDlg::onButtonRight()
{
    m_oldNum = editOldNum->text().toInt();
    // TODO: 发送右转命令
}

void RobotDlg::onButtonStop()
{
    m_oldNum = editOldNum->text().toInt();
    // TODO: 发送停止命令
}

void RobotDlg::onButtonChangeNum()
{
    m_oldNum = editOldNum->text().toInt();
    m_newNum = editNewNum->text().toInt();
    // TODO: 发送更改编号命令
    // 交换编号显示
    editOldNum->setText(QString::number(m_newNum));
    editNewNum->setText(QString::number(m_oldNum));
}

void RobotDlg::onButtonChangeFreq()
{
    m_numSet = editNum->text().toInt();
    m_carFre = radio1_450->isChecked();
    // TODO: 发送更改频率命令
}
