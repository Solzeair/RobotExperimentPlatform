/*
* 摄像头调整对话框
* 提供亮度、增益、对比度、快门及 RGB 白平衡通道的滑块与输入框调节，
* 调节实时下发相机并支持参数持久化保存。
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

// 复用项目内的单例 Camera 进行相机参数读写

CameraDlg::CameraDlg(QWidget *parent)
    : QWidget(parent)
{
    // 各参数的滑块整型值与显示浮点值，初始置零以防未连接相机时访问野值
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
    
    // 相机句柄延迟到 initUI 中获取，构造期保持空
    _pCamera = nullptr;

    // 对话框随父窗口伸缩，避免固定尺寸在小屏被裁切
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initUI();
}

CameraDlg::~CameraDlg()
{}

void CameraDlg::initUI()
{
    // 统一楷体 12pt 加粗，与平台其他对话框视觉风格一致
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);

    // 自顶向下的单列垂直布局，所有参数行顺序堆叠
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 0, 20, 20);
    mainLayout->setSpacing(15);

    QLabel *titleLabel = new QLabel("摄像头调整", this);
    titleLabel->setFont(font);
    mainLayout->addWidget(titleLabel);

    // 取相机单例并读取当前参数作为滑块初值，保证界面与硬件一致
    _pCamera = Camera::GetInstance();
    if (_pCamera) {
        // 相机底层以放大 1000 倍的整数表示，读取后还原为浮点用于显示
        slideBlackLevel = _pCamera->GetBlackLevel();
        slideGain = _pCamera->GetGain();
        slideGamma = _pCamera->GetGamma();
        slideShutter = _pCamera->GetShutter();
        slideRed = _pCamera->GetRed();
        slideGreen = _pCamera->GetGreen();
        slideBlue = _pCamera->GetBlue();

        // 除快门外其余参数均按 /1000 还原为浮点显示值
        blackLevel = slideBlackLevel / 1000.0;
        gain = slideGain / 1000.0;
        gamma = slideGamma / 1000.0;
        shutter = slideShutter;
        red = slideRed / 1000.0;
        green = slideGreen / 1000.0;
        blue = slideBlue / 1000.0;
    }
    
    // 亮度（BlackLevel）参数组：范围标签 + 滑块 + 数值输入框三段式布局
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
    // 末尾占位与右侧 60px 输入框对齐，使最大值标签落在滑块可视末端
    blackLevelRangeLayout->addSpacing(114);
    mainLayout->addLayout(blackLevelRangeLayout);

    QHBoxLayout *blackLevelSliderLayout = new QHBoxLayout();
    sliderBlackLevel = new QSlider(Qt::Horizontal, this);
    sliderBlackLevel->setRange(0, 15984);
    sliderBlackLevel->setValue(slideBlackLevel);
    sliderBlackLevel->setMinimumWidth(200);
    editBlackLevel = new QLineEdit(this);
    editBlackLevel->setText(QString::number(blackLevel, 'f', 3));
    editBlackLevel->setFont(font);
    editBlackLevel->setFixedWidth(60);

    blackLevelSliderLayout->addWidget(sliderBlackLevel);
    blackLevelSliderLayout->addWidget(editBlackLevel);
    mainLayout->addLayout(blackLevelSliderLayout);

    mainLayout->addSpacing(30);
    
    // 增益（Gain）参数组
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
    // 末尾占位与右侧 60px 输入框对齐，使最大值标签落在滑块可视末端
    gainRangeLayout->addSpacing(114);
    mainLayout->addLayout(gainRangeLayout);

    QHBoxLayout *gainSliderLayout = new QHBoxLayout();
    sliderGain = new QSlider(Qt::Horizontal, this);
    sliderGain->setRange(0, 29900);
    sliderGain->setValue(slideGain);
    sliderGain->setMinimumWidth(200);
    editGain = new QLineEdit(this);
    editGain->setText(QString::number(gain, 'f', 3));
    editGain->setFont(font);
    editGain->setFixedWidth(60);

    gainSliderLayout->addWidget(sliderGain);
    gainSliderLayout->addWidget(editGain);
    mainLayout->addLayout(gainSliderLayout);

    mainLayout->addSpacing(30);
    
    // 对比度（Gamma）参数组
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
    // 末尾占位与右侧 60px 输入框对齐，使最大值标签落在滑块可视末端
    gammaRangeLayout->addSpacing(114);
    mainLayout->addLayout(gammaRangeLayout);

    QHBoxLayout *gammaSliderLayout = new QHBoxLayout();
    sliderGamma = new QSlider(Qt::Horizontal, this);
    sliderGamma->setRange(0, 3999);
    sliderGamma->setValue(slideGamma);
    sliderGamma->setMinimumWidth(200);
    editGamma = new QLineEdit(this);
    editGamma->setText(QString::number(gamma, 'f', 3));
    editGamma->setFont(font);
    editGamma->setFixedWidth(60);

    gammaSliderLayout->addWidget(sliderGamma);
    gammaSliderLayout->addWidget(editGamma);
    mainLayout->addLayout(gammaSliderLayout);

    mainLayout->addSpacing(30);
    
    // 快门（Shutter）参数组，下限 17 为相机硬件最小曝光行数限制
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
    // 末尾占位与右侧 60px 输入框对齐，使最大值标签落在滑块可视末端
    shutterRangeLayout->addSpacing(114);
    mainLayout->addLayout(shutterRangeLayout);

    QHBoxLayout *shutterSliderLayout = new QHBoxLayout();
    sliderShutter = new QSlider(Qt::Horizontal, this);
    sliderShutter->setRange(17, 100000);
    sliderShutter->setValue(slideShutter);
    sliderShutter->setMinimumWidth(200);
    editShutter = new QLineEdit(this);
    editShutter->setText(QString::number(shutter));
    editShutter->setFont(font);
    editShutter->setFixedWidth(60);

    shutterSliderLayout->addWidget(sliderShutter);
    shutterSliderLayout->addWidget(editShutter);
    mainLayout->addLayout(shutterSliderLayout);

    mainLayout->addSpacing(30);
    
    // 红色通道（白平衡）参数组
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
    // 末尾占位与右侧 60px 输入框对齐，使最大值标签落在滑块可视末端
    redRangeLayout->addSpacing(114);
    mainLayout->addLayout(redRangeLayout);

    QHBoxLayout *redSliderLayout = new QHBoxLayout();
    sliderRed = new QSlider(Qt::Horizontal, this);
    sliderRed->setRange(0, 15999);
    sliderRed->setValue(slideRed);
    sliderRed->setMinimumWidth(200);
    editRed = new QLineEdit(this);
    editRed->setText(QString::number(red, 'f', 3));
    editRed->setFont(font);
    editRed->setFixedWidth(60);

    redSliderLayout->addWidget(sliderRed);
    redSliderLayout->addWidget(editRed);
    mainLayout->addLayout(redSliderLayout);

    mainLayout->addSpacing(30);
    
    // 绿色通道（白平衡）参数组
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
    // 末尾占位与右侧 60px 输入框对齐，使最大值标签落在滑块可视末端
    greenRangeLayout->addSpacing(114);
    mainLayout->addLayout(greenRangeLayout);

    QHBoxLayout *greenSliderLayout = new QHBoxLayout();
    sliderGreen = new QSlider(Qt::Horizontal, this);
    sliderGreen->setRange(0, 15999);
    sliderGreen->setValue(slideGreen);
    sliderGreen->setMinimumWidth(200);
    editGreen = new QLineEdit(this);
    editGreen->setText(QString::number(green, 'f', 3));
    editGreen->setFont(font);
    editGreen->setFixedWidth(60);

    greenSliderLayout->addWidget(sliderGreen);
    greenSliderLayout->addWidget(editGreen);
    mainLayout->addLayout(greenSliderLayout);

    mainLayout->addSpacing(30);
    
    // 蓝色通道（白平衡）参数组
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
    // 末尾占位与右侧 60px 输入框对齐，使最大值标签落在滑块可视末端
    blueRangeLayout->addSpacing(114);
    mainLayout->addLayout(blueRangeLayout);

    QHBoxLayout *blueSliderLayout = new QHBoxLayout();
    sliderBlue = new QSlider(Qt::Horizontal, this);
    sliderBlue->setRange(0, 15999);
    sliderBlue->setValue(slideBlue);
    sliderBlue->setMinimumWidth(200);
    editBlue = new QLineEdit(this);
    editBlue->setText(QString::number(blue, 'f', 3));
    editBlue->setFont(font);
    editBlue->setFixedWidth(60);

    blueSliderLayout->addWidget(sliderBlue);
    blueSliderLayout->addWidget(editBlue);
    mainLayout->addLayout(blueSliderLayout);

    mainLayout->addSpacing(30);
    
    btnSaveCamera = new QPushButton("保存设置", this);
    btnSaveCamera->setFont(font);
    mainLayout->addWidget(btnSaveCamera, 0, Qt::AlignCenter);

    // 末尾弹性空间，保证内容顶部对齐、对话框拉伸时按钮不跟随上浮
    mainLayout->addStretch();

    // 滑块拖动实时下发相机，无需手动确认
    connect(sliderBlackLevel, SIGNAL(valueChanged(int)), this, SLOT(onSliderBlackLevelChanged(int)));
    connect(sliderGain, SIGNAL(valueChanged(int)), this, SLOT(onSliderGainChanged(int)));
    connect(sliderGamma, SIGNAL(valueChanged(int)), this, SLOT(onSliderGammaChanged(int)));
    connect(sliderShutter, SIGNAL(valueChanged(int)), this, SLOT(onSliderShutterChanged(int)));
    connect(sliderRed, SIGNAL(valueChanged(int)), this, SLOT(onSliderRedChanged(int)));
    connect(sliderGreen, SIGNAL(valueChanged(int)), this, SLOT(onSliderGreenChanged(int)));
    connect(sliderBlue, SIGNAL(valueChanged(int)), this, SLOT(onSliderBlueChanged(int)));
    connect(btnSaveCamera, SIGNAL(clicked()), this, SLOT(onSaveCamera()));

    // 各输入框共用一个回车槽，提交时统一回写滑块与相机
    connect(editBlackLevel, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    connect(editGain, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    connect(editGamma, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    connect(editShutter, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    connect(editRed, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    connect(editGreen, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));
    connect(editBlue, SIGNAL(returnPressed()), this, SLOT(onEditReturnPressed()));

    // 安装事件过滤器拦截 ESC，防止误关对话框丢失未保存调整
    installEventFilter(this);
}

void CameraDlg::onSliderBlackLevelChanged(int value)
{
    slideBlackLevel = value;
    blackLevel = value / 1000.0;
    editBlackLevel->setText(QString::number(blackLevel, 'f', 3));

    // 滑块整数放大 1000 倍，直接下发相机并同步输入框显示
    if (_pCamera) {
        _pCamera->SetBlackLevel(value);
    }
}

void CameraDlg::onSliderGainChanged(int value)
{
    slideGain = value;
    gain = value / 1000.0;
    editGain->setText(QString::number(gain, 'f', 3));

    if (_pCamera) {
        _pCamera->SetGain(value);
    }
}

void CameraDlg::onSliderGammaChanged(int value)
{
    slideGamma = value;
    gamma = value / 1000.0;
    editGamma->setText(QString::number(gamma, 'f', 3));

    if (_pCamera) {
        _pCamera->SetGamma(value);
    }
}

void CameraDlg::onSliderShutterChanged(int value)
{
    slideShutter = value;
    shutter = value;
    editShutter->setText(QString::number(shutter));

    // 快门为整型曝光时间，无需放大换算
    if (_pCamera) {
        _pCamera->SetShutter(value);
    }
}

void CameraDlg::onSliderRedChanged(int value)
{
    slideRed = value;
    red = value / 1000.0;
    editRed->setText(QString::number(red, 'f', 3));

    if (_pCamera) {
        _pCamera->SetRed(value);
    }
}

void CameraDlg::onSliderGreenChanged(int value)
{
    slideGreen = value;
    green = value / 1000.0;
    editGreen->setText(QString::number(green, 'f', 3));

    if (_pCamera) {
        _pCamera->SetGreen(value);
    }
}

void CameraDlg::onSliderBlueChanged(int value)
{
    slideBlue = value;
    blue = value / 1000.0;
    editBlue->setText(QString::number(blue, 'f', 3));

    if (_pCamera) {
        _pCamera->SetBlue(value);
    }
}

void CameraDlg::onSaveCamera()
{
    // 以输入框为准回读所有参数，浮点值乘 1000 转回滑块整数
    blackLevel = editBlackLevel->text().toDouble();
    slideBlackLevel = (int)(blackLevel * 1000);

    gain = editGain->text().toDouble();
    slideGain = (int)(gain * 1000);

    gamma = editGamma->text().toDouble();
    slideGamma = (int)(gamma * 1000);

    shutter = editShutter->text().toInt();
    slideShutter = shutter;

    red = editRed->text().toDouble();
    slideRed = (int)(red * 1000);

    green = editGreen->text().toDouble();
    slideGreen = (int)(green * 1000);

    blue = editBlue->text().toDouble();
    slideBlue = (int)(blue * 1000);

    if (_pCamera) {
        _pCamera->SetBlackLevel(slideBlackLevel);
        _pCamera->SetGain(slideGain);
        _pCamera->SetGamma(slideGamma);
        _pCamera->SetShutter(slideShutter);
        _pCamera->SetRed(slideRed);
        _pCamera->SetGreen(slideGreen);
        _pCamera->SetBlue(slideBlue);

        // 下发后写入配置文件，保证下次上电沿用本次调整
        _pCamera->WriteConfig();
        QMessageBox::information(this, "保存成功", "摄像头参数已保存！");
    } else {
        QMessageBox::warning(this, "保存失败", "无法连接到相机！");
    }
}

void CameraDlg::onEditReturnPressed()
{
    // 任一输入框回车都会回读全部参数并回写滑块，setValue 会触发对应滑块槽完成相机下发
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
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            // 吞掉 ESC，避免误关对话框导致未保存的调整丢失
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}