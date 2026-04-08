#include "ColorDlg.h"
#include <QPainter>
#include <QColor>
#include <cmath>

ColorDlg::ColorDlg(QWidget *parent)
    : QWidget(parent)
    , m_SelectRect(false)
    , m_H_High(0)
    , m_H_Low(0)
    , m_S_High(0)
    , m_S_Low(0)
    , m_I_High(0)
    , m_I_Low(0)
    , m_object(0)
    , m_ImageSeg(false)
    , m_isSaved(false)
{
    // 设置大小策略为可伸缩
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    initUI();
    // 不再调用drawHSIRing()，因为已经在initUI()中创建了色环
}

ColorDlg::~ColorDlg()
{}

void ColorDlg::initUI()
{
    // 设置字体为楷体，12号，加粗
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // 标题 "采色"
    QLabel *titleLabel = new QLabel("采色", this);
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
    
    // 右侧上方布局
    QHBoxLayout *topRightLayout = new QHBoxLayout();
    topRightLayout->setSpacing(15);
    
    // 右侧上方白色显示区域
    QLabel *rightDisplayLabel = new QLabel(this);
    rightDisplayLabel->setFixedSize(200, 200);
    rightDisplayLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid black; }");
    topRightLayout->addWidget(rightDisplayLabel);
    
    // 右侧按钮组
    QVBoxLayout *buttonGroupLayout = new QVBoxLayout();
    buttonGroupLayout->setSpacing(10);
    
    // 矩形选择框复选框
    QCheckBox *rectCheckBox = new QCheckBox("Rect", this);
    rectCheckBox->setFont(font);
    rectCheckBox->setChecked(true);
    buttonGroupLayout->addWidget(rectCheckBox);
    
    // 放大按钮
    QPushButton *zoomButton = new QPushButton("放大", this);
    zoomButton->setFont(font);
    buttonGroupLayout->addWidget(zoomButton);
    
    // 采样按钮
    QPushButton *sampleButton = new QPushButton("采样", this);
    sampleButton->setFont(font);
    buttonGroupLayout->addWidget(sampleButton);
    
    // 清空按钮
    QPushButton *clearButton = new QPushButton("清空", this);
    clearButton->setFont(font);
    buttonGroupLayout->addWidget(clearButton);
    
    topRightLayout->addLayout(buttonGroupLayout);
    controlLayout->addLayout(topRightLayout);
    
    // 中间布局
    QHBoxLayout *midLayout = new QHBoxLayout();
    midLayout->setSpacing(15);
    
    // 我方队色选项
    QVBoxLayout *teamColorLayout = new QVBoxLayout();
    teamColorLayout->setSpacing(5);
    
    QLabel *teamColorLabel = new QLabel("我方队色", this);
    teamColorLabel->setFont(font);
    teamColorLayout->addWidget(teamColorLabel);
    
    // 颜色选择单选按钮
    QRadioButton *purpleRadio = new QRadioButton("紫色", this);
    purpleRadio->setFont(font);
    teamColorLayout->addWidget(purpleRadio);
    
    QRadioButton *greenRadio = new QRadioButton("绿色", this);
    greenRadio->setFont(font);
    teamColorLayout->addWidget(greenRadio);
    
    QRadioButton *blueRadio = new QRadioButton("蓝色", this);
    blueRadio->setFont(font);
    teamColorLayout->addWidget(blueRadio);
    
    QRadioButton *opponentRadio = new QRadioButton("敌方队色", this);
    opponentRadio->setFont(font);
    teamColorLayout->addWidget(opponentRadio);
    
    QRadioButton *mem3Radio = new QRadioButton("MEM3", this);
    mem3Radio->setFont(font);
    teamColorLayout->addWidget(mem3Radio);
    
    QRadioButton *mem4Radio = new QRadioButton("MEM4", this);
    mem4Radio->setFont(font);
    teamColorLayout->addWidget(mem4Radio);
    
    QRadioButton *mem5Radio = new QRadioButton("MEM5", this);
    mem5Radio->setFont(font);
    teamColorLayout->addWidget(mem5Radio);
    
    midLayout->addLayout(teamColorLayout);
    
    // 色环显示 - 背景白色，只保留彩色色环
    HSIdisplayLabel = new QLabel(this);
    HSIdisplayLabel->setFixedSize(220, 180);
    HSIdisplayLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid black; }");
    HSIdisplayLabel->setFont(font);
    
    // 创建彩色色环
    QLabel *colorRingLabel = new QLabel(HSIdisplayLabel);
    colorRingLabel->setGeometry(30, 10, 160, 160);
    colorRingLabel->setStyleSheet("QLabel { background-color: qconicalgradient(cx:0.5, cy:0.5, angle:0, stop:0 #ff0000, stop:0.16 #ffff00, stop:0.33 #00ff00, stop:0.5 #00ffff, stop:0.66 #0000ff, stop:0.83 #ff00ff, stop:1 #ff0000); border-radius: 80px; border: 1px solid black; }");
    
    midLayout->addWidget(HSIdisplayLabel);
    
    // 右侧测试按钮组
    QVBoxLayout *testButtonLayout = new QVBoxLayout();
    testButtonLayout->setSpacing(10);
    
    // 单帧图像按钮
    stopButton = new QPushButton("单帧图像", this);
    stopButton->setFont(font);
    testButtonLayout->addWidget(stopButton);
    
    // 测试按钮
    colorTestButton = new QPushButton("测试", this);
    colorTestButton->setFont(font);
    testButtonLayout->addWidget(colorTestButton);
    
    // 图像分割复选框
    segCheckBox = new QCheckBox("图像分割", this);
    segCheckBox->setFont(font);
    testButtonLayout->addWidget(segCheckBox);
    
    // 动态测试按钮
    runTestButton = new QPushButton("动态测试", this);
    runTestButton->setFont(font);
    testButtonLayout->addWidget(runTestButton);
    
    // 加载按钮
    colorLoadButton = new QPushButton("加载", this);
    colorLoadButton->setFont(font);
    testButtonLayout->addWidget(colorLoadButton);
    
    // 保存按钮
    colorSaveButton = new QPushButton("保存", this);
    colorSaveButton->setFont(font);
    testButtonLayout->addWidget(colorSaveButton);
    
    midLayout->addLayout(testButtonLayout);
    controlLayout->addLayout(midLayout);
    
    // 底部滚轴控制区域
    QVBoxLayout *scrollBarLayout = new QVBoxLayout();
    scrollBarLayout->setSpacing(10);
    
    // 色调-低 标签和滑块
    QHBoxLayout *hueLowLayout = new QHBoxLayout();
    QLabel *hueLowLabel = new QLabel("色调-低", this);
    hueLowLabel->setFont(font);
    scrollBarHMin = new QScrollBar(Qt::Horizontal, this);
    scrollBarHMin->setRange(0, 360);
    scrollBarHMin->setValue(0);
    scrollBarHMin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hueLowLayout->addWidget(hueLowLabel);
    hueLowLayout->addWidget(scrollBarHMin);
    scrollBarLayout->addLayout(hueLowLayout);
    
    // 色调-高 标签和滑块
    QHBoxLayout *hueHighLayout = new QHBoxLayout();
    QLabel *hueHighLabel = new QLabel("色调-高", this);
    hueHighLabel->setFont(font);
    QScrollBar *scrollBarHMax = new QScrollBar(Qt::Horizontal, this);
    scrollBarHMax->setRange(0, 360);
    scrollBarHMax->setValue(0);
    scrollBarHMax->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hueHighLayout->addWidget(hueHighLabel);
    hueHighLayout->addWidget(scrollBarHMax);
    scrollBarLayout->addLayout(hueHighLayout);
    
    // 饱和度-低 标签和滑块
    QHBoxLayout *saturationLowLayout = new QHBoxLayout();
    QLabel *saturationLowLabel = new QLabel("饱和度-低", this);
    saturationLowLabel->setFont(font);
    scrollBarSMin = new QScrollBar(Qt::Horizontal, this);
    scrollBarSMin->setRange(0, 100);
    scrollBarSMin->setValue(0);
    scrollBarSMin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    saturationLowLayout->addWidget(saturationLowLabel);
    saturationLowLayout->addWidget(scrollBarSMin);
    scrollBarLayout->addLayout(saturationLowLayout);
    
    // 饱和度-高 标签和滑块
    QHBoxLayout *saturationHighLayout = new QHBoxLayout();
    QLabel *saturationHighLabel = new QLabel("饱和度-高", this);
    saturationHighLabel->setFont(font);
    QScrollBar *scrollBarSMax = new QScrollBar(Qt::Horizontal, this);
    scrollBarSMax->setRange(0, 100);
    scrollBarSMax->setValue(0);
    scrollBarSMax->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    saturationHighLayout->addWidget(saturationHighLabel);
    saturationHighLayout->addWidget(scrollBarSMax);
    scrollBarLayout->addLayout(saturationHighLayout);
    
    // 亮度 标签和图表
    QHBoxLayout *brightnessLayout = new QHBoxLayout();
    QLabel *brightnessLabel = new QLabel("亮度", this);
    brightnessLabel->setFont(font);
    QLabel *brightnessGraphLabel = new QLabel(this);
    brightnessGraphLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid black; }");
    brightnessGraphLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    brightnessLayout->addWidget(brightnessLabel);
    brightnessLayout->addWidget(brightnessGraphLabel);
    scrollBarLayout->addLayout(brightnessLayout);
    
    // 亮度-低 标签和滑块
    QHBoxLayout *brightnessLowLayout = new QHBoxLayout();
    QLabel *brightnessLowLabel = new QLabel("亮度-低", this);
    brightnessLowLabel->setFont(font);
    scrollBarIMin = new QScrollBar(Qt::Horizontal, this);
    scrollBarIMin->setRange(0, 100);
    scrollBarIMin->setValue(0);
    scrollBarIMin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    brightnessLowLayout->addWidget(brightnessLowLabel);
    brightnessLowLayout->addWidget(scrollBarIMin);
    scrollBarLayout->addLayout(brightnessLowLayout);
    
    // 亮度-高 标签和滑块
    QHBoxLayout *brightnessHighLayout = new QHBoxLayout();
    QLabel *brightnessHighLabel = new QLabel("亮度-高", this);
    brightnessHighLabel->setFont(font);
    QScrollBar *scrollBarIMax = new QScrollBar(Qt::Horizontal, this);
    scrollBarIMax->setRange(0, 100);
    scrollBarIMax->setValue(0);
    scrollBarIMax->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    brightnessHighLayout->addWidget(brightnessHighLabel);
    brightnessHighLayout->addWidget(scrollBarIMax);
    scrollBarLayout->addLayout(brightnessHighLayout);
    
    controlLayout->addLayout(scrollBarLayout);
    
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
    connect(colorTestButton, SIGNAL(clicked()), this, SLOT(onButtonColorTest()));
    connect(runTestButton, SIGNAL(clicked()), this, SLOT(onButtonRunTest()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(onButtonStopTest()));
    connect(colorSaveButton, SIGNAL(clicked()), this, SLOT(onButtonSave()));
    connect(colorLoadButton, SIGNAL(clicked()), this, SLOT(onButtonLoad()));
    connect(segCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onSegCheckBoxStateChanged(int)));
    
    // 连接滚动条
    connect(scrollBarHMin, SIGNAL(valueChanged(int)), this, SLOT(onScrollBarChanged()));
    connect(scrollBarSMin, SIGNAL(valueChanged(int)), this, SLOT(onScrollBarChanged()));
    connect(scrollBarIMin, SIGNAL(valueChanged(int)), this, SLOT(onScrollBarChanged()));
}

void ColorDlg::onButtonColorTest()
{
    // TODO: 实现颜色测试功能
}

void ColorDlg::onButtonRunTest()
{
    // TODO: 实现动态测试功能
}

void ColorDlg::onButtonStopTest()
{
    // TODO: 实现单帧图像功能
}

void ColorDlg::onButtonSave()
{
    // TODO: 实现保存功能
}

void ColorDlg::onButtonLoad()
{
    // TODO: 实现加载功能
}

void ColorDlg::onSegCheckBoxStateChanged(int state)
{
    m_ImageSeg = (state == Qt::Checked);
    // TODO: 处理图像分割状态变化
}

void ColorDlg::onScrollBarChanged()
{
    // TODO: 处理滚动条变化
    m_H_Low = scrollBarHMin->value();
    m_S_Low = scrollBarSMin->value();
    m_I_Low = scrollBarIMin->value();
}

void ColorDlg::drawHSIRing()
{
    // 创建HSI色环图像
    QPixmap pixmap(150, 150);
    pixmap.fill(Qt::white);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制色环
    int centerX = 75;
    int centerY = 75;
    int outerRadius = 60;
    int innerRadius = 30;
    
    // 绘制完整的彩色圆环
    for (int angle = 0; angle < 360; angle += 1)
    {
        double radians = angle * M_PI / 180.0;
        int x1 = centerX + outerRadius * cos(radians);
        int y1 = centerY + outerRadius * sin(radians);
        int x2 = centerX + innerRadius * cos(radians);
        int y2 = centerY + innerRadius * sin(radians);
        
        // 从角度计算H值 (0-360度映射到0-255)
        int h = angle * 255 / 360;
        QColor color;
        color.setHsv(h, 255, 255);
        
        painter.setPen(QPen(color, 2));
        painter.drawLine(x1, y1, x2, y2);
    }
    
    // 绘制中心点
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(Qt::white);
    painter.drawEllipse(centerX - 5, centerY - 5, 10, 10);
    
    // 显示色环
    HSIdisplayLabel->setPixmap(pixmap);
}