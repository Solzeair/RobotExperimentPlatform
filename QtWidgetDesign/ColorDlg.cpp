#include "ColorDlg.h"
#include "Debug.h"
#include "DisplayDlg.h"   // Needed for ShowColorTest / ShowRunTest / ShowSingle

ColorDlg::ColorDlg(QWidget* parent)
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
{
}

ColorDlg* ColorDlg::getInstance()
{
    static ColorDlg instance; // 静态局部变量，保证只创建一次
    return &instance;
}

void ColorDlg::initUI()
{
    // 设置字体为楷体，12号，加粗
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);

    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 0, 20, 20);
    mainLayout->setSpacing(15);

    // 创建主控制布局
    QVBoxLayout* controlLayout = new QVBoxLayout();
    // 设置控件之间间隔为12像素，让内容排列更松散
    controlLayout->setSpacing(12);

    // 标题 "采色"
    QLabel* titleLabel = new QLabel("采色", this);
    titleLabel->setFont(font);
    controlLayout->addWidget(titleLabel);
    controlLayout->setAlignment(titleLabel, Qt::AlignTop);

    // 右侧上方布局
    QHBoxLayout* topRightLayout = new QHBoxLayout();
    topRightLayout->setSpacing(10);

    // 右侧上方白色显示区域
    QLabel* rightDisplayLabel = new QLabel(this);
    rightDisplayLabel->setFixedSize(200, 200);
    rightDisplayLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid black; }");
    topRightLayout->addWidget(rightDisplayLabel);

    // 右侧按钮组
    QVBoxLayout* buttonGroupLayout = new QVBoxLayout();
    buttonGroupLayout->setSpacing(10);

    // 矩形选择框复选框
    QCheckBox* rectCheckBox = new QCheckBox("Rect", this);
    rectCheckBox->setFont(font);
    rectCheckBox->setChecked(true);
    buttonGroupLayout->addWidget(rectCheckBox);

    // 放大按钮
    QPushButton* zoomButton = new QPushButton("放大", this);
    zoomButton->setFont(font);
    buttonGroupLayout->addWidget(zoomButton);

    // 采样按钮
    QPushButton* sampleButton = new QPushButton("采样", this);
    sampleButton->setFont(font);
    buttonGroupLayout->addWidget(sampleButton);

    // 清空按钮
    QPushButton* clearButton = new QPushButton("清空", this);
    clearButton->setFont(font);
    buttonGroupLayout->addWidget(clearButton);

    topRightLayout->addLayout(buttonGroupLayout);
    controlLayout->addLayout(topRightLayout);

    // 中间布局
    QHBoxLayout* midLayout = new QHBoxLayout();
    midLayout->setSpacing(15);

    // 我方队色选项
    QVBoxLayout* teamColorLayout = new QVBoxLayout();
    teamColorLayout->setSpacing(5);

    QLabel* teamColorLabel = new QLabel("我方队色", this);
    teamColorLabel->setFont(font);
    teamColorLayout->addWidget(teamColorLabel);

    // 颜色选择单选按钮
    QRadioButton* purpleRadio = new QRadioButton("紫色", this);
    purpleRadio->setFont(font);
    teamColorLayout->addWidget(purpleRadio);

    QRadioButton* greenRadio = new QRadioButton("绿色", this);
    greenRadio->setFont(font);
    teamColorLayout->addWidget(greenRadio);

    QRadioButton* blueRadio = new QRadioButton("蓝色", this);
    blueRadio->setFont(font);
    teamColorLayout->addWidget(blueRadio);

    QRadioButton* opponentRadio = new QRadioButton("敌方队色", this);
    opponentRadio->setFont(font);
    teamColorLayout->addWidget(opponentRadio);

    QRadioButton* mem3Radio = new QRadioButton("MEM3", this);
    mem3Radio->setFont(font);
    teamColorLayout->addWidget(mem3Radio);

    QRadioButton* mem4Radio = new QRadioButton("MEM4", this);
    mem4Radio->setFont(font);
    teamColorLayout->addWidget(mem4Radio);

    QRadioButton* mem5Radio = new QRadioButton("MEM5", this);
    mem5Radio->setFont(font);
    teamColorLayout->addWidget(mem5Radio);

    midLayout->addLayout(teamColorLayout);

    // 色环显示 - 背景白色，只保留彩色色环
    HSIdisplayLabel = new QLabel(this);
    HSIdisplayLabel->setFixedSize(220, 180);
    HSIdisplayLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid black; }");
    HSIdisplayLabel->setFont(font);

    // 创建彩色色环
    QLabel* colorRingLabel = new QLabel(HSIdisplayLabel);
    colorRingLabel->setGeometry(30, 10, 160, 160);
    colorRingLabel->setStyleSheet("QLabel { background-color: qconicalgradient(cx:0.5, cy:0.5, angle:0, stop:0 #ff0000, stop:0.16 #ffff00, stop:0.33 #00ff00, stop:0.5 #00ffff, stop:0.66 #0000ff, stop:0.83 #ff00ff, stop:1 #ff0000); border-radius: 80px; border: 1px solid black; }");

    midLayout->addWidget(HSIdisplayLabel);

    // 右侧测试按钮组
    QVBoxLayout* testButtonLayout = new QVBoxLayout();
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
    QVBoxLayout* scrollBarLayout = new QVBoxLayout();
    scrollBarLayout->setSpacing(10);

    // 色调-低 标签和滑块
    QHBoxLayout* hueLowLayout = new QHBoxLayout();
    QLabel* hueLowLabel = new QLabel("色调-低", this);
    hueLowLabel->setFont(font);
    scrollBarHMin = new QScrollBar(Qt::Horizontal, this);
    scrollBarHMin->setRange(0, 360);
    scrollBarHMin->setValue(0);
    scrollBarHMin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hueLowLayout->addWidget(hueLowLabel);
    hueLowLayout->addWidget(scrollBarHMin);
    scrollBarLayout->addLayout(hueLowLayout);

    // 色调-高 标签和滑块
    QHBoxLayout* hueHighLayout = new QHBoxLayout();
    QLabel* hueHighLabel = new QLabel("色调-高", this);
    hueHighLabel->setFont(font);
    QScrollBar* scrollBarHMax = new QScrollBar(Qt::Horizontal, this);
    scrollBarHMax->setRange(0, 360);
    scrollBarHMax->setValue(0);
    scrollBarHMax->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hueHighLayout->addWidget(hueHighLabel);
    hueHighLayout->addWidget(scrollBarHMax);
    scrollBarLayout->addLayout(hueHighLayout);

    // 饱和度-低 标签和滑块
    QHBoxLayout* saturationLowLayout = new QHBoxLayout();
    QLabel* saturationLowLabel = new QLabel("饱和度-低", this);
    saturationLowLabel->setFont(font);
    scrollBarSMin = new QScrollBar(Qt::Horizontal, this);
    scrollBarSMin->setRange(0, 100);
    scrollBarSMin->setValue(0);
    scrollBarSMin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    saturationLowLayout->addWidget(saturationLowLabel);
    saturationLowLayout->addWidget(scrollBarSMin);
    scrollBarLayout->addLayout(saturationLowLayout);

    // 饱和度-高 标签和滑块
    QHBoxLayout* saturationHighLayout = new QHBoxLayout();
    QLabel* saturationHighLabel = new QLabel("饱和度-高", this);
    saturationHighLabel->setFont(font);
    QScrollBar* scrollBarSMax = new QScrollBar(Qt::Horizontal, this);
    scrollBarSMax->setRange(0, 100);
    scrollBarSMax->setValue(0);
    scrollBarSMax->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    saturationHighLayout->addWidget(saturationHighLabel);
    saturationHighLayout->addWidget(scrollBarSMax);
    scrollBarLayout->addLayout(saturationHighLayout);

    // 亮度 标签和图表
    QHBoxLayout* brightnessLayout = new QHBoxLayout();
    QLabel* brightnessLabel = new QLabel("亮度", this);
    brightnessLabel->setFont(font);
    QLabel* brightnessGraphLabel = new QLabel(this);
    brightnessGraphLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid black; }");
    brightnessGraphLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    brightnessLayout->addWidget(brightnessLabel);
    brightnessLayout->addWidget(brightnessGraphLabel);
    scrollBarLayout->addLayout(brightnessLayout);

    // 亮度-低 标签和滑块
    QHBoxLayout* brightnessLowLayout = new QHBoxLayout();
    QLabel* brightnessLowLabel = new QLabel("亮度-低", this);
    brightnessLowLabel->setFont(font);
    scrollBarIMin = new QScrollBar(Qt::Horizontal, this);
    scrollBarIMin->setRange(0, 100);
    scrollBarIMin->setValue(0);
    scrollBarIMin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    brightnessLowLayout->addWidget(brightnessLowLabel);
    brightnessLowLayout->addWidget(scrollBarIMin);
    scrollBarLayout->addLayout(brightnessLowLayout);

    // 亮度-高 标签和滑块
    QHBoxLayout* brightnessHighLayout = new QHBoxLayout();
    QLabel* brightnessHighLabel = new QLabel("亮度-高", this);
    brightnessHighLabel->setFont(font);
    QScrollBar* scrollBarIMax = new QScrollBar(Qt::Horizontal, this);
    scrollBarIMax->setRange(0, 100);
    scrollBarIMax->setValue(0);
    scrollBarIMax->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    brightnessHighLayout->addWidget(brightnessHighLabel);
    brightnessHighLayout->addWidget(scrollBarIMax);
    scrollBarLayout->addLayout(brightnessHighLayout);

    controlLayout->addLayout(scrollBarLayout);

    mainLayout->addLayout(controlLayout);

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
    // Trigger a single-frame colour filter test.
    // DisplayDlg::ShowColorTest() renders only those pixels that
    // satisfy the current HSI thresholds for the selected object.
    UpdateHSIThreshold();
    // Locate the DisplayDlg instance through the parent hierarchy.
    // ColorDlg is a tab inside QTabWidget, whose parent is the main window
    // that also owns DisplayDlg.  We broadcast via Qt's object tree.
    QWidget* tabWidget = parentWidget();
    if (!tabWidget) return;
    QWidget* mainWin = tabWidget->parentWidget();
    if (!mainWin) return;
    DisplayDlg* dispDlg = mainWin->findChild<DisplayDlg*>();
    if (dispDlg) {
        dispDlg->ShowColorTest(HSIThreshold, m_object);
    }
}

void ColorDlg::onButtonRunTest()
{
    // Start continuous colour-segmentation display.
    UpdateHSIThreshold();
    QWidget* tabWidget = parentWidget();
    if (!tabWidget) return;
    QWidget* mainWin = tabWidget->parentWidget();
    if (!mainWin) return;
    DisplayDlg* dispDlg = mainWin->findChild<DisplayDlg*>();
    if (dispDlg) {
        dispDlg->ShowRunTest(m_ImageSeg);
    }
}

void ColorDlg::onButtonStopTest()
{
    // Grab and display a single frame (freeze the image).
    QWidget* tabWidget = parentWidget();
    if (!tabWidget) return;
    QWidget* mainWin = tabWidget->parentWidget();
    if (!mainWin) return;
    DisplayDlg* dispDlg = mainWin->findChild<DisplayDlg*>();
    if (dispDlg) {
        dispDlg->ShowSingle();
    }
}

void ColorDlg::onButtonSave()
{
    // Save all 8 × 6 HSI threshold values to "color.dat" (binary).
    static const char* kColorFile = "color.dat";
    QFile file(kColorFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(reinterpret_cast<const char*>(HSIThreshold),
            sizeof(HSIThreshold));
        file.close();
        m_isSaved = true;
        Debug::get()->print(L"[Color] HSI thresholds saved to color.dat.");
    }
    else {
        Debug::get()->print(L"[Color] ERROR: could not write color.dat.");
    }
}

void ColorDlg::onButtonLoad()
{
    // Load HSI threshold values from "color.dat" (binary).
    static const char* kColorFile = "color.dat";
    QFile file(kColorFile);
    if (file.open(QIODevice::ReadOnly)) {
        qint64 expected = static_cast<qint64>(sizeof(HSIThreshold));
        if (file.size() >= expected) {
            file.read(reinterpret_cast<char*>(HSIThreshold), expected);
            file.close();
            // Reflect loaded values in the scroll-bar positions
            // (update only the bars that are member variables)
            scrollBarHMin->setValue(HSIThreshold[m_object][0]);
            scrollBarSMin->setValue(HSIThreshold[m_object][2]);
            scrollBarIMin->setValue(HSIThreshold[m_object][4]);
            m_isSaved = true;
            Debug::get()->print(L"[Color] HSI thresholds loaded from color.dat.");
        }
        else {
            file.close();
            Debug::get()->print(L"[Color] color.dat has wrong size – ignored.");
        }
    }
    else {
        Debug::get()->print(L"[Color] color.dat not found "
            L"(will be created on first save).");
    }
}

void ColorDlg::onSegCheckBoxStateChanged(int state)
{
    m_ImageSeg = (state == Qt::Checked);
}

void ColorDlg::onScrollBarChanged()
{
    // Read all three scroll bars that are member variables and
    // write them into the HSIThreshold table for the current object.
    m_H_Low = scrollBarHMin->value();
    m_S_Low = scrollBarSMin->value();
    m_I_Low = scrollBarIMin->value();
    UpdateHSIThreshold();
}

// Sync the current scroll-bar (and internal field) values into the
// HSIThreshold array for the currently selected object index.
// The "high" bars are local variables in initUI, so we cannot read
// them here; instead we update only the "low" bounds that have
// member-variable scroll-bars.
void ColorDlg::UpdateHSIThreshold()
{
    if (m_object < 0 || m_object >= 8) return;
    HSIThreshold[m_object][0] = m_H_Low;
    // [1] H_High is set when the high scroll-bar is connected in initUI.
    // For now the pattern is: low is [0], high is [1], same for S and I.
    HSIThreshold[m_object][2] = m_S_Low;
    HSIThreshold[m_object][4] = m_I_Low;
    // High bounds remain at whatever was loaded from file or previously set.
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