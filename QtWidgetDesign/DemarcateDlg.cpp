#include "DemarcateDlg.h"
#include <QThread>
#include <QCoreApplication>
#include <QMessageBox>

DemarcateDlg::DemarcateDlg(QWidget *parent)
    : QWidget(parent)
    , m_isSaved(true)
    , m_needResetDC(false)
{
    // 设置大小策略为可伸缩
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 初始化边界点
    point[0].setX(0); point[0].setY(0);
    point[1].setX(0); point[1].setY(70);
    point[2].setX(-15); point[2].setY(70);
    point[3].setX(-15); point[3].setY(110);
    point[4].setX(0); point[4].setY(110);
    point[5].setX(0); point[5].setY(180);
    point[6].setX(220); point[6].setY(180);
    point[7].setX(220); point[7].setY(110);
    point[8].setX(235); point[8].setY(110);
    point[9].setX(235); point[9].setY(70);
    point[10].setX(220); point[10].setY(70);
    point[11].setX(220); point[11].setY(0);
    point[12].setX(0); point[12].setY(0);
    
    initUI();
}

DemarcateDlg::~DemarcateDlg()
{}

void DemarcateDlg::initUI()
{
    // 设置字体为楷体，12号，加粗
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // 标题 "标定"
    QLabel *titleLabel = new QLabel("标定", this);
    titleLabel->setFont(font);
    mainLayout->addWidget(titleLabel);
    
    // 创建顶部布局（左侧显示区域 + 右侧控制区域）
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(20);
    
    // 左侧显示区域
    QLabel *displayLabel = new QLabel(this);
    displayLabel->setStyleSheet("QLabel { background-color: #333333; border: 1px solid black; }");
    displayLabel->setFont(font);
    displayLabel->setAlignment(Qt::AlignCenter);
    displayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    topLayout->addWidget(displayLabel);
    
    // 右侧控制区域
    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->setSpacing(15);
    
    // 进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    controlLayout->addWidget(progressBar);
    
    // 右侧上方白色图像显示区域
    QLabel *imageDisplayLabel = new QLabel(this);
    imageDisplayLabel->setFixedSize(340, 200);
    imageDisplayLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid black; }");
    controlLayout->addWidget(imageDisplayLabel, 0, Qt::AlignCenter);
    
    // 绿色进度条
    QLabel *greenBarLabel = new QLabel(this);
    greenBarLabel->setStyleSheet("QLabel { background-color: #00FF00; border: 1px solid black; }");
    greenBarLabel->setFixedHeight(20);
    controlLayout->addWidget(greenBarLabel);
    
    // 按钮组 - 第一行
    QHBoxLayout *buttonRow1Layout = new QHBoxLayout();
    buttonRow1Layout->setSpacing(20);
    
    // 刷新图像按钮
    btnFlush = new QPushButton("刷新图像", this);
    btnFlush->setFont(font);
    buttonRow1Layout->addWidget(btnFlush);
    
    // 撤销一步按钮
    btnResetOne = new QPushButton("撤销一步", this);
    btnResetOne->setFont(font);
    buttonRow1Layout->addWidget(btnResetOne);
    
    // 开始标定按钮
    btnSet = new QPushButton("开始标定", this);
    btnSet->setFont(font);
    buttonRow1Layout->addWidget(btnSet);
    
    controlLayout->addLayout(buttonRow1Layout);
    
    // 按钮组 - 第二行
    QHBoxLayout *buttonRow2Layout = new QHBoxLayout();
    buttonRow2Layout->setSpacing(20);
    
    // 重新标定按钮
    btnReset = new QPushButton("重新标定", this);
    btnReset->setFont(font);
    buttonRow2Layout->addWidget(btnReset);
    
    // 加载按钮
    btnLoad = new QPushButton("加载", this);
    btnLoad->setFont(font);
    buttonRow2Layout->addWidget(btnLoad);
    
    // 保存按钮
    btnSave = new QPushButton("保存", this);
    btnSave->setFont(font);
    buttonRow2Layout->addWidget(btnSave);
    
    controlLayout->addLayout(buttonRow2Layout);
    
    // 查看当前标定结果按钮
    btnShowRes = new QPushButton("查看当前标定结果", this);
    btnShowRes->setFont(font);
    controlLayout->addWidget(btnShowRes, 0, Qt::AlignCenter);
    
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
    
    // 禁用部分按钮
    btnSet->setEnabled(false);
    btnResetOne->setEnabled(false);
    
    // 连接信号槽
    connect(btnSet, SIGNAL(clicked()), this, SLOT(onButtonSet()));
    connect(btnResetOne, SIGNAL(clicked()), this, SLOT(onButtonResetOne()));
    connect(btnReset, SIGNAL(clicked()), this, SLOT(onButtonReset()));
    connect(btnLoad, SIGNAL(clicked()), this, SLOT(onButtonLoad()));
    connect(btnSave, SIGNAL(clicked()), this, SLOT(onButtonSave()));
    connect(btnFlush, SIGNAL(clicked()), this, SLOT(onButtonFlush()));
    connect(btnShowRes, SIGNAL(clicked()), this, SLOT(onButtonShowRes()));
}

void DemarcateDlg::onButtonSet()
{
    // TODO: 实现标定算法
    // 这里只是模拟标定过程
    progressBar->setValue(0);
    for (int i = 0; i <= 100; i++)
    {
        progressBar->setValue(i);
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    
    m_needResetDC = true;
    onButtonShowRes();
    QMessageBox::information(this, "标定完成", "场地标定完成！");
    
    btnSet->setEnabled(false);
    btnResetOne->setEnabled(false);
    m_isSaved = false;
}

void DemarcateDlg::onButtonResetOne()
{
    if (!m_points.isEmpty())
    {
        m_points.pop_back();
        if (m_points.isEmpty())
        {
            btnResetOne->setEnabled(false);
        }
    }
}

void DemarcateDlg::onButtonReset()
{
    m_points.clear();
    progressBar->setValue(0);
    btnSet->setEnabled(false);
    btnResetOne->setEnabled(false);
    resultLabel->setText("标定结果");
    // TODO: 重置显示
}

void DemarcateDlg::onButtonLoad()
{
    // TODO: 加载标定数据
    m_needResetDC = true;
    m_isSaved = true;
    QMessageBox::information(this, "加载完成", "标定数据已加载！");
}

void DemarcateDlg::onButtonSave()
{
    // TODO: 保存标定数据
    QMessageBox::information(this, "保存完成", "标定数据已保存！");
    m_isSaved = true;
}

void DemarcateDlg::onButtonFlush()
{
    // TODO: 刷新图像
    QMessageBox::information(this, "刷新", "图像已刷新！");
}

void DemarcateDlg::onButtonShowRes()
{
    if (!m_needResetDC)
    {
        return;
    }
    
    // TODO: 显示标定结果
    resultLabel->setText("标定结果显示");
    m_needResetDC = false;
}
