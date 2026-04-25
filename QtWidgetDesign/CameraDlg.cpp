/*
* 摄像头调整对话框文件
* 写作人 李青
* 功能 摄像头调整界面设计，包含亮度、增益、对比度、快门、红色、绿色、蓝色等参数的滑块和输入框，以及保存按钮。
* 对比度、红、绿、蓝 只是保存了值， 不会实时生效，其余可以实时应用 
*/

#include "CameraDlg.h"
#include "Camera.h"
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QKeyEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <QSpacerItem>

// 使用项目中已有的Camera类

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
    
    // 初始化相机实例
    _pCamera = nullptr;
    
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
    mainLayout->setContentsMargins(20, 0, 20, 20);
    mainLayout->setSpacing(15);
    
    // 标题 "摄像头调整"
    QLabel *titleLabel = new QLabel("摄像头调整", this);
    titleLabel->setFont(font);
    mainLayout->addWidget(titleLabel);
    
    // 获取相机实例并读取初始参数
    _pCamera = Camera::GetInstance();
    if (_pCamera) {
        // 从相机获取初始参数值
        slideBlackLevel = _pCamera->GetBlackLevel();
        slideGain = _pCamera->GetGain();
        slideGamma = _pCamera->GetGamma();
        slideShutter = _pCamera->GetShutter();
        slideRed = _pCamera->GetRed();
        slideGreen = _pCamera->GetGreen();
        slideBlue = _pCamera->GetBlue();
        
        // 转换为显示值
        blackLevel = slideBlackLevel / 1000.0;
        gain = slideGain / 1000.0;
        gamma = slideGamma / 1000.0;
        shutter = slideShutter;
        red = slideRed / 1000.0;
        green = slideGreen / 1000.0;
        blue = slideBlue / 1000.0;
    }
    
    // 亮度参数组
    // 最小值和最大值标签在滚轴上方，中间放置标题
    QHBoxLayout *blackLevelRangeLayout = new QHBoxLayout();
    QLabel *lblMinBlackLevel = new QLabel("0.0", this);
    lblMinBlackLevel->setFont(font);
    QLabel *lblBlackLevelTitle = new QLabel("亮度", this);
    lblBlackLevelTitle->setFont(font);
    QLabel *lblMaxBlackLevel = new QLabel("15.984", this);
    lblMaxBlackLevel->setFont(font);
    blackLevelRangeLayout->addWidget(lblMinBlackLevel);
    blackLevelRangeLayout->addStretch();
    blackLevelRangeLayout->addWidget(lblBlackLevelTitle);
    blackLevelRangeLayout->addStretch();
    blackLevelRangeLayout->addWidget(lblMaxBlackLevel);
    blackLevelRangeLayout->addSpacing(114); // 添加114像素的空间，将最大值标签向左移动约3厘米
    mainLayout->addLayout(blackLevelRangeLayout);
    
    // 滚轴和标签
    QHBoxLayout *blackLevelSliderLayout = new QHBoxLayout();
    sliderBlackLevel = new QSlider(Qt::Horizontal, this);
    sliderBlackLevel->setRange(0, 15984);
    sliderBlackLevel->setValue(slideBlackLevel);
    sliderBlackLevel->setMinimumWidth(200); // 设置最小宽度
    editBlackLevel = new QLineEdit(this);
    editBlackLevel->setText(QString::number(blackLevel, 'f', 3));
    editBlackLevel->setFont(font);
    editBlackLevel->setFixedWidth(60);
    
    blackLevelSliderLayout->addWidget(sliderBlackLevel);
    blackLevelSliderLayout->addWidget(editBlackLevel);
    mainLayout->addLayout(blackLevelSliderLayout);
    
    // 添加间距
    mainLayout->addSpacing(30);
    
    // 增益参数组
    // 最小值和最大值标签在滚轴上方，中间放置标题
    QHBoxLayout *gainRangeLayout = new QHBoxLayout();
    QLabel *lblMinGain = new QLabel("0.0", this);
    lblMinGain->setFont(font);
    QLabel *lblGainTitle = new QLabel("增益", this);
    lblGainTitle->setFont(font);
    QLabel *lblMaxGain = new QLabel("29.9", this);
    lblMaxGain->setFont(font);
    gainRangeLayout->addWidget(lblMinGain);
    gainRangeLayout->addStretch();
    gainRangeLayout->addWidget(lblGainTitle);
    gainRangeLayout->addStretch();
    gainRangeLayout->addWidget(lblMaxGain);
    gainRangeLayout->addSpacing(114); // 添加114像素的空间，将最大值标签向左移动约3厘米
    mainLayout->addLayout(gainRangeLayout);
    
    // 滚轴和标签
    QHBoxLayout *gainSliderLayout = new QHBoxLayout();
    sliderGain = new QSlider(Qt::Horizontal, this);
    sliderGain->setRange(0, 29900);
    sliderGain->setValue(slideGain);
    sliderGain->setMinimumWidth(200); // 设置最小宽度
    editGain = new QLineEdit(this);
    editGain->setText(QString::number(gain, 'f', 3));
    editGain->setFont(font);
    editGain->setFixedWidth(60);
    
    gainSliderLayout->addWidget(sliderGain);
    gainSliderLayout->addWidget(editGain);
    mainLayout->addLayout(gainSliderLayout);
    
    // 添加间距
    mainLayout->addSpacing(30);
    
    // 对比度参数组
    // 最小值和最大值标签在滚轴上方，中间放置标题
    QHBoxLayout *gammaRangeLayout = new QHBoxLayout();
    QLabel *lblMinGamma = new QLabel("0.0", this);
    lblMinGamma->setFont(font);
    QLabel *lblGammaTitle = new QLabel("对比度", this);
    lblGammaTitle->setFont(font);
    QLabel *lblMaxGamma = new QLabel("3.999", this);
    lblMaxGamma->setFont(font);
    gammaRangeLayout->addWidget(lblMinGamma);
    gammaRangeLayout->addStretch();
    gammaRangeLayout->addWidget(lblGammaTitle);
    gammaRangeLayout->addStretch();
    gammaRangeLayout->addWidget(lblMaxGamma);
    gammaRangeLayout->addSpacing(114); // 添加114像素的空间，将最大值标签向左移动约3厘米
    mainLayout->addLayout(gammaRangeLayout);
    
    // 滚轴和标签
    QHBoxLayout *gammaSliderLayout = new QHBoxLayout();
    sliderGamma = new QSlider(Qt::Horizontal, this);
    sliderGamma->setRange(0, 3999);
    sliderGamma->setValue(slideGamma);
    sliderGamma->setMinimumWidth(200); // 设置最小宽度
    editGamma = new QLineEdit(this);
    editGamma->setText(QString::number(gamma, 'f', 3));
    editGamma->setFont(font);
    editGamma->setFixedWidth(60);
    
    gammaSliderLayout->addWidget(sliderGamma);
    gammaSliderLayout->addWidget(editGamma);
    mainLayout->addLayout(gammaSliderLayout);
    
    // 添加间距
    mainLayout->addSpacing(30);
    
    // 快门参数组
    // 最小值和最大值标签在滚轴上方，中间放置标题
    QHBoxLayout *shutterRangeLayout = new QHBoxLayout();
    QLabel *lblMinShutter = new QLabel("0.0", this);
    lblMinShutter->setFont(font);
    QLabel *lblShutterTitle = new QLabel("快门", this);
    lblShutterTitle->setFont(font);
    QLabel *lblMaxShutter = new QLabel("100000", this);
    lblMaxShutter->setFont(font);
    shutterRangeLayout->addWidget(lblMinShutter);
    shutterRangeLayout->addStretch();
    shutterRangeLayout->addWidget(lblShutterTitle);
    shutterRangeLayout->addStretch();
    shutterRangeLayout->addWidget(lblMaxShutter);
    shutterRangeLayout->addSpacing(114); // 添加114像素的空间，将最大值标签向左移动约3厘米
    mainLayout->addLayout(shutterRangeLayout);
    
    // 滚轴和标签
    QHBoxLayout *shutterSliderLayout = new QHBoxLayout();
    sliderShutter = new QSlider(Qt::Horizontal, this);
    sliderShutter->setRange(17, 100000);
    sliderShutter->setValue(slideShutter);
    sliderShutter->setMinimumWidth(200); // 设置最小宽度
    editShutter = new QLineEdit(this);
    editShutter->setText(QString::number(shutter));
    editShutter->setFont(font);
    editShutter->setFixedWidth(60);
    
    shutterSliderLayout->addWidget(sliderShutter);
    shutterSliderLayout->addWidget(editShutter);
    mainLayout->addLayout(shutterSliderLayout);
    
    // 添加间距
    mainLayout->addSpacing(30);
    
    // 红色参数组
    // 最小值和最大值标签在滚轴上方，中间放置标题
    QHBoxLayout *redRangeLayout = new QHBoxLayout();
    QLabel *lblMinRed = new QLabel("0.0", this);
    lblMinRed->setFont(font);
    QLabel *lblRedTitle = new QLabel("红色", this);
    lblRedTitle->setFont(font);
    QLabel *lblMaxRed = new QLabel("15.999", this);
    lblMaxRed->setFont(font);
    redRangeLayout->addWidget(lblMinRed);
    redRangeLayout->addStretch();
    redRangeLayout->addWidget(lblRedTitle);
    redRangeLayout->addStretch();
    redRangeLayout->addWidget(lblMaxRed);
    redRangeLayout->addSpacing(114); // 添加114像素的空间，将最大值标签向左移动约3厘米
    mainLayout->addLayout(redRangeLayout);
    
    // 滚轴和标签
    QHBoxLayout *redSliderLayout = new QHBoxLayout();
    sliderRed = new QSlider(Qt::Horizontal, this);
    sliderRed->setRange(0, 15999);
    sliderRed->setValue(slideRed);
    sliderRed->setMinimumWidth(200); // 设置最小宽度
    editRed = new QLineEdit(this);
    editRed->setText(QString::number(red, 'f', 3));
    editRed->setFont(font);
    editRed->setFixedWidth(60);
    
    redSliderLayout->addWidget(sliderRed);
    redSliderLayout->addWidget(editRed);
    mainLayout->addLayout(redSliderLayout);
    
    // 添加间距
    mainLayout->addSpacing(30);
    
    // 绿色参数组
    // 最小值和最大值标签在滚轴上方，中间放置标题
    QHBoxLayout *greenRangeLayout = new QHBoxLayout();
    QLabel *lblMinGreen = new QLabel("0.0", this);
    lblMinGreen->setFont(font);
    QLabel *lblGreenTitle = new QLabel("绿色", this);
    lblGreenTitle->setFont(font);
    QLabel *lblMaxGreen = new QLabel("15.999", this);
    lblMaxGreen->setFont(font);
    greenRangeLayout->addWidget(lblMinGreen);
    greenRangeLayout->addStretch();
    greenRangeLayout->addWidget(lblGreenTitle);
    greenRangeLayout->addStretch();
    greenRangeLayout->addWidget(lblMaxGreen);
    greenRangeLayout->addSpacing(114); // 添加114像素的空间，将最大值标签向左移动约3厘米
    mainLayout->addLayout(greenRangeLayout);
    
    // 滚轴和标签
    QHBoxLayout *greenSliderLayout = new QHBoxLayout();
    sliderGreen = new QSlider(Qt::Horizontal, this);
    sliderGreen->setRange(0, 15999);
    sliderGreen->setValue(slideGreen);
    sliderGreen->setMinimumWidth(200); // 设置最小宽度
    editGreen = new QLineEdit(this);
    editGreen->setText(QString::number(green, 'f', 3));
    editGreen->setFont(font);
    editGreen->setFixedWidth(60);
    
    greenSliderLayout->addWidget(sliderGreen);
    greenSliderLayout->addWidget(editGreen);
    mainLayout->addLayout(greenSliderLayout);
    
    // 添加间距
    mainLayout->addSpacing(30);
    
    // 蓝色参数组
    // 最小值和最大值标签在滚轴上方，中间放置标题
    QHBoxLayout *blueRangeLayout = new QHBoxLayout();
    QLabel *lblMinBlue = new QLabel("0.0", this);
    lblMinBlue->setFont(font);
    QLabel *lblBlueTitle = new QLabel("蓝色", this);
    lblBlueTitle->setFont(font);
    QLabel *lblMaxBlue = new QLabel("15.999", this);
    lblMaxBlue->setFont(font);
    blueRangeLayout->addWidget(lblMinBlue);
    blueRangeLayout->addStretch();
    blueRangeLayout->addWidget(lblBlueTitle);
    blueRangeLayout->addStretch();
    blueRangeLayout->addWidget(lblMaxBlue);
    blueRangeLayout->addSpacing(114); // 添加114像素的空间，将最大值标签向左移动约3厘米
    mainLayout->addLayout(blueRangeLayout);
    
    // 滚轴和标签
    QHBoxLayout *blueSliderLayout = new QHBoxLayout();
    sliderBlue = new QSlider(Qt::Horizontal, this);
    sliderBlue->setRange(0, 15999);
    sliderBlue->setValue(slideBlue);
    sliderBlue->setMinimumWidth(200); // 设置最小宽度
    editBlue = new QLineEdit(this);
    editBlue->setText(QString::number(blue, 'f', 3));
    editBlue->setFont(font);
    editBlue->setFixedWidth(60);
    
    blueSliderLayout->addWidget(sliderBlue);
    blueSliderLayout->addWidget(editBlue);
    mainLayout->addLayout(blueSliderLayout);
    
    // 添加间距
    mainLayout->addSpacing(30);
    
    // 保存按钮
    btnSaveCamera = new QPushButton("保存设置", this);
    btnSaveCamera->setFont(font);
    mainLayout->addWidget(btnSaveCamera, 0, Qt::AlignCenter);
    
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
    
    // 安装事件过滤器，用于处理ESC键事件
    installEventFilter(this);
}

void CameraDlg::onSliderBlackLevelChanged(int value)
{
    slideBlackLevel = value;
    blackLevel = value / 1000.0;
    editBlackLevel->setText(QString::number(blackLevel, 'f', 3));
    
    // 更新相机参数
    if (_pCamera) {
        _pCamera->SetBlackLevel(value);
    }
}

void CameraDlg::onSliderGainChanged(int value)
{
    slideGain = value;
    gain = value / 1000.0;
    editGain->setText(QString::number(gain, 'f', 3));
    
    // 更新相机参数
    if (_pCamera) {
        _pCamera->SetGain(value);
    }
}

void CameraDlg::onSliderGammaChanged(int value)
{
    slideGamma = value;
    gamma = value / 1000.0;
    editGamma->setText(QString::number(gamma, 'f', 3));
    
    // 更新相机参数
    if (_pCamera) {
        _pCamera->SetGamma(value);
    }
}

void CameraDlg::onSliderShutterChanged(int value)
{
    slideShutter = value;
    shutter = value;
    editShutter->setText(QString::number(shutter));
    
    // 更新相机参数
    if (_pCamera) {
        _pCamera->SetShutter(value);
    }
}

void CameraDlg::onSliderRedChanged(int value)
{
    slideRed = value;
    red = value / 1000.0;
    editRed->setText(QString::number(red, 'f', 3));
    
    // 更新相机参数
    if (_pCamera) {
        _pCamera->SetRed(value);
    }
}

void CameraDlg::onSliderGreenChanged(int value)
{
    slideGreen = value;
    green = value / 1000.0;
    editGreen->setText(QString::number(green, 'f', 3));
    
    // 更新相机参数
    if (_pCamera) {
        _pCamera->SetGreen(value);
    }
}

void CameraDlg::onSliderBlueChanged(int value)
{
    slideBlue = value;
    blue = value / 1000.0;
    editBlue->setText(QString::number(blue, 'f', 3));
    
    // 更新相机参数
    if (_pCamera) {
        _pCamera->SetBlue(value);
    }
}

void CameraDlg::onSaveCamera()
{
    // 保存摄像头参数到文件
    if (_pCamera) {
        _pCamera->WriteConfig();
        QMessageBox::information(this, "保存成功", "摄像头参数已保存！");
    } else {
        QMessageBox::warning(this, "保存失败", "无法连接到相机！");
    }
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
    
    // 更新相机参数
    if (_pCamera) {
        _pCamera->SetBlackLevel(slideBlackLevel);
        _pCamera->SetGain(slideGain);
        _pCamera->SetGamma(slideGamma);
        _pCamera->SetShutter(slideShutter);
        _pCamera->SetRed(slideRed);
        _pCamera->SetGreen(slideGreen);
        _pCamera->SetBlue(slideBlue);
    }
}

bool CameraDlg::eventFilter(QObject *obj, QEvent *event)
{
    // 处理ESC键事件，禁止通过ESC键关闭对话框
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            return true; // 拦截ESC键事件
        }
    }
    return QWidget::eventFilter(obj, event);
}