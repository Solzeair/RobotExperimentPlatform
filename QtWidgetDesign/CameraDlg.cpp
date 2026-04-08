#include "CameraDlg.h"
#include <QMessageBox>

CameraDlg::CameraDlg(QWidget *parent)
    : QWidget(parent)
{
    // 初始化数据
    slideBlackLevel = 0;
    slideGain = 0;
    slideGamma = 0;
    slideShutter = 0;
    slideRed = 0;
    slideGreen = 0;
    slideBlue = 0;
    
    blackLevel = 0.0;
    gain = 0.0;
    gamma = 0.0;
    shutter = 0;
    red = 0.0;
    green = 0.0;
    blue = 0.0;
    
    // 设置大小策略为可伸缩
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    initUI();
}

CameraDlg::~CameraDlg()
{}

void CameraDlg::initUI()
{
    // 设置字体为楷体，12pt，加粗
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // 标题 "摄像头调整"
    QLabel *titleLabel = new QLabel("摄像头调整", this);
    titleLabel->setFont(font);
    mainLayout->addWidget(titleLabel);
    
    // 创建顶部布局（摄像头显示 + 右侧控制）
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(20);
    
    // 摄像头显示区域
    cameraViewLabel = new QLabel(this);
    cameraViewLabel->setStyleSheet("QLabel { background-color: #333333; border: 1px solid black; }");
    cameraViewLabel->setFont(font);
    cameraViewLabel->setText("摄像头显示区域");
    cameraViewLabel->setAlignment(Qt::AlignCenter);
    cameraViewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    topLayout->addWidget(cameraViewLabel);
    
    // 右侧控制区域
    QVBoxLayout *controlLayout = new QVBoxLayout();
    // 设置滚轴之间间隔，使用合理的值确保所有滚轴都能显示
    controlLayout->setSpacing(30);
    
    // 亮度参数组
    QHBoxLayout *blackLevelLayout = new QHBoxLayout();
    QLabel *lblMinBlackLevel = new QLabel("0.0", this);
    lblMinBlackLevel->setFont(font);
    sliderBlackLevel = new QSlider(Qt::Horizontal, this);
    sliderBlackLevel->setRange(0, 15984);
    sliderBlackLevel->setValue(slideBlackLevel);
    sliderBlackLevel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QLabel *lblBlackLevel = new QLabel("亮度", this);
    lblBlackLevel->setFont(font);
    QLabel *lblMaxBlackLevel = new QLabel("15.984", this);
    lblMaxBlackLevel->setFont(font);
    editBlackLevel = new QLineEdit(this);
    editBlackLevel->setText(QString::number(blackLevel, 'f', 3));
    editBlackLevel->setFont(font);
    editBlackLevel->setFixedWidth(60);
    
    blackLevelLayout->addWidget(lblMinBlackLevel);
    blackLevelLayout->addWidget(sliderBlackLevel);
    blackLevelLayout->addWidget(lblBlackLevel);
    blackLevelLayout->addWidget(lblMaxBlackLevel);
    blackLevelLayout->addWidget(editBlackLevel);
    controlLayout->addLayout(blackLevelLayout);
    
    // 增益参数组
    QHBoxLayout *gainLayout = new QHBoxLayout();
    QLabel *lblMinGain = new QLabel("0.0", this);
    lblMinGain->setFont(font);
    sliderGain = new QSlider(Qt::Horizontal, this);
    sliderGain->setRange(0, 29900);
    sliderGain->setValue(slideGain);
    sliderGain->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QLabel *lblGain = new QLabel("增益", this);
    lblGain->setFont(font);
    QLabel *lblMaxGain = new QLabel("29.9", this);
    lblMaxGain->setFont(font);
    editGain = new QLineEdit(this);
    editGain->setText(QString::number(gain, 'f', 3));
    editGain->setFont(font);
    editGain->setFixedWidth(60);
    
    gainLayout->addWidget(lblMinGain);
    gainLayout->addWidget(sliderGain);
    gainLayout->addWidget(lblGain);
    gainLayout->addWidget(lblMaxGain);
    gainLayout->addWidget(editGain);
    controlLayout->addLayout(gainLayout);
    
    // 对比度参数组
    QHBoxLayout *gammaLayout = new QHBoxLayout();
    QLabel *lblMinGamma = new QLabel("0.0", this);
    lblMinGamma->setFont(font);
    sliderGamma = new QSlider(Qt::Horizontal, this);
    sliderGamma->setRange(0, 3999);
    sliderGamma->setValue(slideGamma);
    sliderGamma->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QLabel *lblGamma = new QLabel("对比度", this);
    lblGamma->setFont(font);
    QLabel *lblMaxGamma = new QLabel("3.999", this);
    lblMaxGamma->setFont(font);
    editGamma = new QLineEdit(this);
    editGamma->setText(QString::number(gamma, 'f', 3));
    editGamma->setFont(font);
    editGamma->setFixedWidth(60);
    
    gammaLayout->addWidget(lblMinGamma);
    gammaLayout->addWidget(sliderGamma);
    gammaLayout->addWidget(lblGamma);
    gammaLayout->addWidget(lblMaxGamma);
    gammaLayout->addWidget(editGamma);
    controlLayout->addLayout(gammaLayout);
    
    // 快门参数组
    QHBoxLayout *shutterLayout = new QHBoxLayout();
    QLabel *lblMinShutter = new QLabel("0.0", this);
    lblMinShutter->setFont(font);
    sliderShutter = new QSlider(Qt::Horizontal, this);
    sliderShutter->setRange(17, 100000);
    sliderShutter->setValue(slideShutter);
    sliderShutter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QLabel *lblShutter = new QLabel("快门", this);
    lblShutter->setFont(font);
    QLabel *lblMaxShutter = new QLabel("100000", this);
    lblMaxShutter->setFont(font);
    editShutter = new QLineEdit(this);
    editShutter->setText(QString::number(shutter));
    editShutter->setFont(font);
    editShutter->setFixedWidth(60);
    
    shutterLayout->addWidget(lblMinShutter);
    shutterLayout->addWidget(sliderShutter);
    shutterLayout->addWidget(lblShutter);
    shutterLayout->addWidget(lblMaxShutter);
    shutterLayout->addWidget(editShutter);
    controlLayout->addLayout(shutterLayout);
    
    // 红色参数组
    QHBoxLayout *redLayout = new QHBoxLayout();
    QLabel *lblMinRed = new QLabel("0.0", this);
    lblMinRed->setFont(font);
    sliderRed = new QSlider(Qt::Horizontal, this);
    sliderRed->setRange(0, 15999);
    sliderRed->setValue(slideRed);
    sliderRed->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QLabel *lblRed = new QLabel("红色", this);
    lblRed->setFont(font);
    QLabel *lblMaxRed = new QLabel("15.999", this);
    lblMaxRed->setFont(font);
    editRed = new QLineEdit(this);
    editRed->setText(QString::number(red, 'f', 3));
    editRed->setFont(font);
    editRed->setFixedWidth(60);
    
    redLayout->addWidget(lblMinRed);
    redLayout->addWidget(sliderRed);
    redLayout->addWidget(lblRed);
    redLayout->addWidget(lblMaxRed);
    redLayout->addWidget(editRed);
    controlLayout->addLayout(redLayout);
    
    // 绿色参数组
    QHBoxLayout *greenLayout = new QHBoxLayout();
    QLabel *lblMinGreen = new QLabel("0.0", this);
    lblMinGreen->setFont(font);
    sliderGreen = new QSlider(Qt::Horizontal, this);
    sliderGreen->setRange(0, 15999);
    sliderGreen->setValue(slideGreen);
    sliderGreen->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QLabel *lblGreen = new QLabel("绿色", this);
    lblGreen->setFont(font);
    QLabel *lblMaxGreen = new QLabel("15.999", this);
    lblMaxGreen->setFont(font);
    editGreen = new QLineEdit(this);
    editGreen->setText(QString::number(green, 'f', 3));
    editGreen->setFont(font);
    editGreen->setFixedWidth(60);
    
    greenLayout->addWidget(lblMinGreen);
    greenLayout->addWidget(sliderGreen);
    greenLayout->addWidget(lblGreen);
    greenLayout->addWidget(lblMaxGreen);
    greenLayout->addWidget(editGreen);
    controlLayout->addLayout(greenLayout);
    
    // 蓝色参数组
    QHBoxLayout *blueLayout = new QHBoxLayout();
    QLabel *lblMinBlue = new QLabel("0.0", this);
    lblMinBlue->setFont(font);
    sliderBlue = new QSlider(Qt::Horizontal, this);
    sliderBlue->setRange(0, 15999);
    sliderBlue->setValue(slideBlue);
    sliderBlue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QLabel *lblBlue = new QLabel("蓝色", this);
    lblBlue->setFont(font);
    QLabel *lblMaxBlue = new QLabel("15.999", this);
    lblMaxBlue->setFont(font);
    editBlue = new QLineEdit(this);
    editBlue->setText(QString::number(blue, 'f', 3));
    editBlue->setFont(font);
    editBlue->setFixedWidth(60);
    
    blueLayout->addWidget(lblMinBlue);
    blueLayout->addWidget(sliderBlue);
    blueLayout->addWidget(lblBlue);
    blueLayout->addWidget(lblMaxBlue);
    blueLayout->addWidget(editBlue);
    controlLayout->addLayout(blueLayout);
    
    // 保存按钮
    btnSaveCamera = new QPushButton("保存设置", this);
    btnSaveCamera->setFont(font);
    controlLayout->addWidget(btnSaveCamera, 0, Qt::AlignCenter);
    
    topLayout->addLayout(controlLayout);
    mainLayout->addLayout(topLayout);
    
    // 输出区域
    outputLabel = new QLabel(this);
    outputLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid black; font-family: Consolas; font-size: 10pt; }");
    outputLabel->setFont(font);
    outputLabel->setText("输出区域:");
    outputLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    outputLabel->setFixedHeight(100);
    mainLayout->addWidget(outputLabel);
    
    // 连接信号槽
    connect(sliderBlackLevel, SIGNAL(valueChanged(int)), this, SLOT(onSliderBlackLevelChanged(int)));
    connect(sliderGain, SIGNAL(valueChanged(int)), this, SLOT(onSliderGainChanged(int)));
    connect(sliderGamma, SIGNAL(valueChanged(int)), this, SLOT(onSliderGammaChanged(int)));
    connect(sliderShutter, SIGNAL(valueChanged(int)), this, SLOT(onSliderShutterChanged(int)));
    connect(sliderRed, SIGNAL(valueChanged(int)), this, SLOT(onSliderRedChanged(int)));
    connect(sliderGreen, SIGNAL(valueChanged(int)), this, SLOT(onSliderGreenChanged(int)));
    connect(sliderBlue, SIGNAL(valueChanged(int)), this, SLOT(onSliderBlueChanged(int)));
    connect(btnSaveCamera, SIGNAL(clicked()), this, SLOT(onSaveCamera()));
    
    // 连接编辑框回车事件
    connect(editBlackLevel, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    connect(editGain, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    connect(editGamma, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    connect(editShutter, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    connect(editRed, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    connect(editGreen, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    connect(editBlue, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
}

void CameraDlg::onSliderBlackLevelChanged(int value)
{
    slideBlackLevel = value;
    blackLevel = value / 1000.0;
    editBlackLevel->setText(QString::number(blackLevel, 'f', 3));
}

void CameraDlg::onSliderGainChanged(int value)
{
    slideGain = value;
    gain = value / 1000.0;
    editGain->setText(QString::number(gain, 'f', 3));
}

void CameraDlg::onSliderGammaChanged(int value)
{
    slideGamma = value;
    gamma = value / 1000.0;
    editGamma->setText(QString::number(gamma, 'f', 3));
}

void CameraDlg::onSliderShutterChanged(int value)
{
    slideShutter = value;
    shutter = value;
    editShutter->setText(QString::number(shutter));
}

void CameraDlg::onSliderRedChanged(int value)
{
    slideRed = value;
    red = value / 1000.0;
    editRed->setText(QString::number(red, 'f', 3));
}

void CameraDlg::onSliderGreenChanged(int value)
{
    slideGreen = value;
    green = value / 1000.0;
    editGreen->setText(QString::number(green, 'f', 3));
}

void CameraDlg::onSliderBlueChanged(int value)
{
    slideBlue = value;
    blue = value / 1000.0;
    editBlue->setText(QString::number(blue, 'f', 3));
}

void CameraDlg::onSaveCamera()
{
    // TODO: 保存摄像头参数到文件
    QMessageBox::information(this, "保存成功", "摄像头参数已保存！");
}

void CameraDlg::onEditReturnPressed()
{
    // 从编辑框读取值并更新滑块
    blackLevel = editBlackLevel->text().toDouble();
    slideBlackLevel = (int)(blackLevel * 1000);
    sliderBlackLevel->setValue(slideBlackLevel);
    
    gain = editGain->text().toDouble();
    slideGain = (int)(gain * 1000);
    sliderGain->setValue(slideGain);
    
    gamma = editGamma->text().toDouble();
    slideGamma = (int)(gamma * 1000);
    sliderGamma->setValue(slideGamma);
    
    shutter = editShutter->text().toInt();
    slideShutter = shutter;
    sliderShutter->setValue(slideShutter);
    
    red = editRed->text().toDouble();
    slideRed = (int)(red * 1000);
    sliderRed->setValue(slideRed);
    
    green = editGreen->text().toDouble();
    slideGreen = (int)(green * 1000);
    sliderGreen->setValue(slideGreen);
    
    blue = editBlue->text().toDouble();
    slideBlue = (int)(blue * 1000);
    sliderBlue->setValue(slideBlue);
}
