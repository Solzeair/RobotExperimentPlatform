#include "ColorDlg.h"
#include "Debug.h"
#include "DisplayDlg.h"   // Needed for ShowColorTest / ShowRunTest / ShowSingle
#include <QDialog>
#include <QMessageBox>
#include <QDebug>
#include <QStyle>
#include <QMouseEvent>
#include <cstring>

// HLUT 全局查找表（定义在 DisplayDlg.cpp），与 FindPixel 共用同一份数据
extern int HLUT[256][256][256];

// HSI 饱和度计算用的三分量最小值，实现须与 DisplayDlg 完全一致
static inline int Min3(int a, int b, int c) {
    int t = a < b ? a : b;
    return t < c ? t : c;
}

// m_lastFrame 缓存最近一帧完整原图，放大/采样统一基于它，
// 避免直接读取 QLabel::pixmap() 因缩放产生差异


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
    , brightnessGraphLabel(nullptr)
{
    // 顶层窗口可随父布局伸缩
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initUI();
    // 色环已在 initUI 内创建，无需再次绘制
}

ColorDlg::~ColorDlg()
{
}

ColorDlg* ColorDlg::getInstance()
{
    // Meyers 单例，懒加载且线程安全
    static ColorDlg instance;
    return &instance;
}

void ColorDlg::initUI()
{
    // 楷体加粗，与界面其他控件风格统一
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 0, 20, 20);
    mainLayout->setSpacing(10);

    QVBoxLayout* controlLayout = new QVBoxLayout();
    controlLayout->setSpacing(4);

    // 界面标题
    QLabel* titleLabel = new QLabel("采色", this);
    titleLabel->setFont(font);
    controlLayout->addWidget(titleLabel);
    controlLayout->setAlignment(titleLabel, Qt::AlignTop);

    // ── 第一行：预览框 + 操作按钮 ─────────────────────────────
    QHBoxLayout* topRowLayout = new QHBoxLayout();
    topRowLayout->setSpacing(10);

    // 预览区，承载放大图与框选/采样操作（成员变量）
    m_pDisplayLabel = new QLabel(this);
    m_pDisplayLabel->setFixedSize(220, 180);
    m_pDisplayLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid black; }");
    m_pDisplayLabel->setAlignment(Qt::AlignCenter);
    topRowLayout->addWidget(m_pDisplayLabel);
    m_pDisplayLabel->setMouseTracking(true);
    m_pDisplayLabel->installEventFilter(this);

    // 操作按钮组（框选/放大/采样/清空）
    QVBoxLayout* btnLayout = new QVBoxLayout();
    btnLayout->setSpacing(8);

    // 勾选后切换为矩形框选模式，否则为单点采样模式
    QCheckBox* rectCheckBox = new QCheckBox("Rect", this);
    rectCheckBox->setFont(font);
    rectCheckBox->setChecked(true);
    btnLayout->addWidget(rectCheckBox);
    connect(rectCheckBox, &QCheckBox::stateChanged, this, [this](int s){ m_SelectRect = (s == Qt::Checked); });
    m_SelectRect = rectCheckBox->isChecked();

    QPushButton* zoomButton = new QPushButton("放大", this);
    zoomButton->setFont(font);
    btnLayout->addWidget(zoomButton);

    QPushButton* sampleButton = new QPushButton("采样", this);
    sampleButton->setFont(font);
    btnLayout->addWidget(sampleButton);

    QPushButton* clearButton = new QPushButton("清空", this);
    clearButton->setFont(font);
    btnLayout->addWidget(clearButton);

    topRowLayout->addLayout(btnLayout);
    controlLayout->addLayout(topRowLayout);

    // ── 第二行：对象选择 + 色环 + 测试按钮 ──────────────────
    QHBoxLayout* midLayout = new QHBoxLayout();
    midLayout->setSpacing(12);

    // --- 对象选择（radio按钮） ---
    QVBoxLayout* radioLayout = new QVBoxLayout();
    radioLayout->setSpacing(4);

    m_objectGroup = new QButtonGroup(this);

    // 主颜色单选按钮，object 编号 0~4
    struct { const char* label; int object; } radioDefs[] = {
        {"我方队色", 0},
        {"紫色", 1},
        {"绿色", 2},
        {"球色", 3},
        {"敌方队色", 4},
    };
    for (auto& def : radioDefs) {
        QRadioButton* rb = new QRadioButton(def.label, this);
        rb->setFont(font);
        m_objectGroup->addButton(rb, def.object);
        radioLayout->addWidget(rb);
    }

    // 扩展对象，object 编号 5~7
    struct { const char* label; int object; } subDefs[] = {
        {"MEM3", 5},
        {"MEM4", 6},
        {"MEM5", 7},
    };
    for (auto& def : subDefs) {
        QRadioButton* rb = new QRadioButton(def.label, this);
        rb->setFont(font);
        m_objectGroup->addButton(rb, def.object);
        radioLayout->addWidget(rb);
    }
    // 默认选中"我方队色"
    m_objectGroup->button(0)->setChecked(true);
    m_object = 0;

    // 切换对象时加载对应 HSI 阈值，并按当前测试模式联动刷新
    connect(m_objectGroup, &QButtonGroup::buttonClicked,
            this, [this](QAbstractButton* btn) {
        int id = m_objectGroup->id(btn);
        qDebug() << "[RadioButton] clicked, id=" << id << "m_isRunTesting=" << m_isRunTesting;
        if (id >= 0) {
            loadThresholdForObject(id);
            if (m_isRunTesting) {
                // 动态测试中切换对象不打断测试，下帧自动采用新对象
            } else if (m_isColorTesting) {
                // 单帧测试中切换对象则自动重新执行一次测试
                onButtonColorTest();
            }
        }
    });

    midLayout->addLayout(radioLayout);

    // --- 色环 ---
    HSIdisplayLabel = new QLabel(this);
    HSIdisplayLabel->setFixedSize(220, 180);
    HSIdisplayLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid black; }");
    midLayout->addWidget(HSIdisplayLabel);

    // --- 测试/存取按钮组 ---
    QVBoxLayout* testBtnLayout = new QVBoxLayout();
    testBtnLayout->setSpacing(8);

    stopButton = new QPushButton("单帧图像", this);
    stopButton->setFont(font);
    testBtnLayout->addWidget(stopButton);

    colorTestButton = new QPushButton("测试", this);
    colorTestButton->setFont(font);
    testBtnLayout->addWidget(colorTestButton);

    segCheckBox = new QCheckBox("图像分割", this);
    segCheckBox->setFont(font);
    testBtnLayout->addWidget(segCheckBox);

    runTestButton = new QPushButton("动态测试", this);
    runTestButton->setFont(font);
    testBtnLayout->addWidget(runTestButton);

    colorLoadButton = new QPushButton("加载", this);
    colorLoadButton->setFont(font);
    testBtnLayout->addWidget(colorLoadButton);

    colorSaveButton = new QPushButton("保存", this);
    colorSaveButton->setFont(font);
    testBtnLayout->addWidget(colorSaveButton);

    midLayout->addLayout(testBtnLayout);
    controlLayout->addLayout(midLayout);

    // ── 第三行：HSI 六个滑块 ─────────────────────────────────
    QVBoxLayout* scrollBarLayout = new QVBoxLayout();
    scrollBarLayout->setSpacing(6);

    // 色调下限
    QHBoxLayout* hueLowLayout = new QHBoxLayout();
    QLabel* hueLowLabel = new QLabel("色调-低", this);
    hueLowLabel->setFont(font);
    scrollBarHMin = new QScrollBar(Qt::Horizontal, this);
    scrollBarHMin->setRange(0, H_RANGE_MAX);
    scrollBarHMin->setValue(0);
    scrollBarHMin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hueLowLayout->addWidget(hueLowLabel);
    hueLowLayout->addWidget(scrollBarHMin);
    scrollBarLayout->addLayout(hueLowLayout);

    // 色调上限
    QHBoxLayout* hueHighLayout = new QHBoxLayout();
    QLabel* hueHighLabel = new QLabel("色调-高", this);
    hueHighLabel->setFont(font);
    scrollBarHMax = new QScrollBar(Qt::Horizontal, this);
    scrollBarHMax->setRange(0, H_RANGE_MAX);
    scrollBarHMax->setValue(H_RANGE_MAX);
    scrollBarHMax->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hueHighLayout->addWidget(hueHighLabel);
    hueHighLayout->addWidget(scrollBarHMax);
    scrollBarLayout->addLayout(hueHighLayout);

    // 饱和度下限
    QHBoxLayout* satLowLayout = new QHBoxLayout();
    QLabel* satLowLabel = new QLabel("饱和度-低", this);
    satLowLabel->setFont(font);
    scrollBarSMin = new QScrollBar(Qt::Horizontal, this);
    scrollBarSMin->setRange(0, 100);
    scrollBarSMin->setValue(0);
    scrollBarSMin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    satLowLayout->addWidget(satLowLabel);
    satLowLayout->addWidget(scrollBarSMin);
    scrollBarLayout->addLayout(satLowLayout);

    // 饱和度上限
    QHBoxLayout* satHighLayout = new QHBoxLayout();
    QLabel* satHighLabel = new QLabel("饱和度-高", this);
    satHighLabel->setFont(font);
    scrollBarSMax = new QScrollBar(Qt::Horizontal, this);
    scrollBarSMax->setRange(0, 100);
    scrollBarSMax->setValue(100);
    scrollBarSMax->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    satHighLayout->addWidget(satHighLabel);
    satHighLayout->addWidget(scrollBarSMax);
    scrollBarLayout->addLayout(satHighLayout);

    // 亮度分布直方图
    QHBoxLayout* brightnessLayout = new QHBoxLayout();
    QLabel* brightnessLabel = new QLabel("亮度", this);
    brightnessLabel->setFont(font);
    brightnessGraphLabel = new QLabel(this);
    brightnessGraphLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid black; }");
    brightnessGraphLabel->setFixedHeight(60);
    brightnessGraphLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    brightnessLayout->addWidget(brightnessLabel);
    brightnessLayout->addWidget(brightnessGraphLabel);
    scrollBarLayout->addLayout(brightnessLayout);

    // 亮度下限
    QHBoxLayout* brightnessLowLayout = new QHBoxLayout();
    QLabel* brightnessLowLabel = new QLabel("亮度-低", this);
    brightnessLowLabel->setFont(font);
    scrollBarIMin = new QScrollBar(Qt::Horizontal, this);
    scrollBarIMin->setRange(0, I_RANGE_MAX);
    scrollBarIMin->setValue(0);
    scrollBarIMin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    brightnessLowLayout->addWidget(brightnessLowLabel);
    brightnessLowLayout->addWidget(scrollBarIMin);
    scrollBarLayout->addLayout(brightnessLowLayout);

    // 亮度上限
    QHBoxLayout* brightnessHighLayout = new QHBoxLayout();
    QLabel* brightnessHighLabel = new QLabel("亮度-高", this);
    brightnessHighLabel->setFont(font);
    scrollBarIMax = new QScrollBar(Qt::Horizontal, this);
    scrollBarIMax->setRange(0, I_RANGE_MAX);
    scrollBarIMax->setValue(I_RANGE_MAX);
    scrollBarIMax->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    brightnessHighLayout->addWidget(brightnessHighLabel);
    brightnessHighLayout->addWidget(scrollBarIMax);
    scrollBarLayout->addLayout(brightnessHighLayout);

    controlLayout->addLayout(scrollBarLayout);

    mainLayout->addLayout(controlLayout);

    // 添加弹簧，使控件群整体居顶
    mainLayout->addStretch();

    // 按钮→槽
    connect(colorTestButton, SIGNAL(clicked()), this, SLOT(onButtonColorTest()));
    connect(runTestButton, SIGNAL(clicked()), this, SLOT(onButtonRunTest()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(onButtonStopTest()));
    connect(colorSaveButton, SIGNAL(clicked()), this, SLOT(onButtonSave()));
    connect(colorLoadButton, SIGNAL(clicked()), this, SLOT(onButtonLoad()));
    connect(segCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onSegCheckBoxStateChanged(int)));

    // 六个滑块的值变更响应：更新阈值、重绘色环并标记未保存
    // H 色相：允许 Low>High 以覆盖跨越 0° 的环形区间
    connect(scrollBarHMin, &QScrollBar::valueChanged, this, [this](int) {
        m_H_Low = scrollBarHMin->value();
        UpdateHSIThreshold();
        drawHSIRing();
        m_isSaved = false;
    });
    connect(scrollBarHMax, &QScrollBar::valueChanged, this, [this](int) {
        m_H_High = scrollBarHMax->value();
        UpdateHSIThreshold();
        drawHSIRing();
        m_isSaved = false;
    });

    // S 饱和度：Low>High 不合法，强制回退旧值（沿用 MFC 行为）
    connect(scrollBarSMin, &QScrollBar::valueChanged, this, [this](int val) {
        if (val > scrollBarSMax->value()) {
            scrollBarSMin->blockSignals(true);
            scrollBarSMin->setValue(m_S_Low);
            scrollBarSMin->blockSignals(false);
            return;
        }
        m_S_Low = val;
        UpdateHSIThreshold();
        drawHSIRing();
        drawBrightnessHistogram();
        m_isSaved = false;
    });
    connect(scrollBarSMax, &QScrollBar::valueChanged, this, [this](int val) {
        if (val < scrollBarSMin->value()) {
            scrollBarSMax->blockSignals(true);
            scrollBarSMax->setValue(m_S_High);
            scrollBarSMax->blockSignals(false);
            return;
        }
        m_S_High = val;
        UpdateHSIThreshold();
        drawHSIRing();
        drawBrightnessHistogram();
        m_isSaved = false;
    });

    // I 亮度：Low>High 不合法，强制回退旧值（沿用 MFC 行为）
    connect(scrollBarIMin, &QScrollBar::valueChanged, this, [this](int val) {
        if (val > scrollBarIMax->value()) {
            scrollBarIMin->blockSignals(true);
            scrollBarIMin->setValue(m_I_Low);
            scrollBarIMin->blockSignals(false);
            return;
        }
        m_I_Low = val;
        UpdateHSIThreshold();
        drawHSIRing();
        drawBrightnessHistogram();
        m_isSaved = false;
    });
    connect(scrollBarIMax, &QScrollBar::valueChanged, this, [this](int val) {
        if (val < scrollBarIMin->value()) {
            scrollBarIMax->blockSignals(true);
            scrollBarIMax->setValue(m_I_High);
            scrollBarIMax->blockSignals(false);
            return;
        }
        m_I_High = val;
        UpdateHSIThreshold();
        drawHSIRing();
        drawBrightnessHistogram();
        m_isSaved = false;
    });

    // 放大/采样/清空按钮→槽
    connect(zoomButton, &QPushButton::clicked, this, &ColorDlg::onZoom);
    connect(sampleButton, &QPushButton::clicked, this, &ColorDlg::onSample);
    connect(clearButton, &QPushButton::clicked, this, &ColorDlg::onClearSamples);

    // 不自动连接帧到达信号：预览框仅在用户点击"放大"后填充框选内容，
    // 避免连续刷新与手动框选操作冲突。

    // 亮度直方图计数清零
    memset(yi, 0, sizeof(yi));

    // 预设 5 个主要对象的默认 HSI 阈值
    // {H_low, H_high, S_low, S_high, I_low, I_high}  单位: H×10, S%, I(0~255)
    // 设计要点：各对象 H 区间对应色环不同色相区，但弧线跨度统一(≈80°)，
    // S 统一排除灰白(30~100)、I 统一取中段(85~170)，保证弧线形状一致仅位置不同
    int defaults[5][6] = {
        {2000, 2800, 30, 100, 85, 170},   // 0 我方队色（蓝色，H≈200°-280°）
        {2700, 3300, 30, 100, 85, 170},   // 1 紫色（H≈270°-330°）
        { 800, 1600, 30, 100, 85, 170},   // 2 绿色（H≈80°-160°）
        { 100,  500, 30, 100, 85, 170},   // 3 球色（橙色，H≈10°-50°）
        { 400,  800, 30, 100, 85, 170},   // 4 敌方队色（黄色，H≈40°-80°）
    };
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 6; ++j)
            HSIThreshold[i][j] = defaults[i][j];

    // 启动时使用内置默认阈值，不自动读 color.dat；
    // 用户点击"加载"后才以文件内容覆盖
    loadThresholdForObject(0);
    m_isSaved = true;

}


void ColorDlg::onButtonColorTest()
{
    // 单帧测试：清除左侧红框，保留右侧预览便于二次框选
    m_isColorTesting = true;
    m_isRunTesting = false;  // 停止动态测试，进入单帧模式
    DisplayDlg* dispDlg = this->window()->findChild<DisplayDlg*>();
    if (dispDlg) {
        dispDlg->clearOverlaySelection();
    }
    UpdateHSIThreshold();
    if (dispDlg) {
        dispDlg->ShowColorTest(HSIThreshold, m_object);
    }
}

void ColorDlg::onButtonRunTest()
{
    // 动态测试：清除左侧红框，保留右侧预览便于二次框选
    m_isRunTesting = true;
    DisplayDlg* dispDlg = this->window()->findChild<DisplayDlg*>();
    if (dispDlg) {
        dispDlg->clearOverlaySelection();
    }

    UpdateHSIThreshold();
    if (dispDlg) {
        dispDlg->ShowRunTest(m_ImageSeg);
    }
}

void ColorDlg::onButtonStopTest()
{
    m_isColorTesting = false;
    m_isRunTesting = false;
    // 先停定时器再抓单帧，避免抓到测试中间态（对应 MFC ShowColorTest 的 Stop()）
    DisplayDlg* dispDlg = this->window()->findChild<DisplayDlg*>();
    if (dispDlg) {
        dispDlg->Stop();
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

void ColorDlg::saveData()
{
    onButtonSave();
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

            // ── 向后兼容：旧格式 H 值 0–360 → 新格式 0–3600 ──
            {
                bool isOld = true;
                for (int i = 0; i < 8 && isOld; ++i)
                    if (HSIThreshold[i][0] > 360 || HSIThreshold[i][1] > 360)
                        isOld = false;
                if (isOld) {
                    for (int i = 0; i < 8; ++i) {
                        HSIThreshold[i][0] *= 10;
                        HSIThreshold[i][1] *= 10;
                    }
                }
            }

            // 同步当前对象的全部 6 个滑块并重绘色环，
            // 对应 MFC OnBnClickedColorLoad 的 UpdateHSIThreshold() + OnPaint()
            loadThresholdForObject(m_object);
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
    // 批量回读滑块值，用于信号阻塞期间的统一同步
    m_H_Low  = scrollBarHMin->value();
    m_H_High = scrollBarHMax->value();
    m_S_Low  = scrollBarSMin->value();
    m_S_High = scrollBarSMax->value();
    m_I_Low  = scrollBarIMin->value();
    m_I_High = scrollBarIMax->value();

    UpdateHSIThreshold();
    drawHSIRing();
    drawBrightnessHistogram();
}

// 将当前滑块值写入 HSIThreshold 数组（滑块→数组）
void ColorDlg::UpdateHSIThreshold()
{
    if (m_object < 0 || m_object >= 8) return;
    HSIThreshold[m_object][0] = m_H_Low;
    HSIThreshold[m_object][1] = m_H_High;
    HSIThreshold[m_object][2] = m_S_Low;
    HSIThreshold[m_object][3] = m_S_High;
    HSIThreshold[m_object][4] = m_I_Low;
    HSIThreshold[m_object][5] = m_I_High;
}

// 切换对象时调用：从数组读取阈值并更新滑块（数组→滑块）
void ColorDlg::loadThresholdForObject(int obj)
{
    if (obj < 0 || obj >= 8) return;
    m_object = obj;
    m_H_Low  = HSIThreshold[obj][0];
    m_H_High = HSIThreshold[obj][1];
    m_S_Low  = HSIThreshold[obj][2];
    m_S_High = HSIThreshold[obj][3];
    m_I_Low  = HSIThreshold[obj][4];
    m_I_High = HSIThreshold[obj][5];

    // 阻塞信号后批量设值，避免逐个 valueChanged 触发连锁重绘
    scrollBarHMin->blockSignals(true);
    scrollBarHMax->blockSignals(true);
    scrollBarSMin->blockSignals(true);
    scrollBarSMax->blockSignals(true);
    scrollBarIMin->blockSignals(true);
    scrollBarIMax->blockSignals(true);

    scrollBarHMin->setValue(m_H_Low);
    scrollBarHMax->setValue(m_H_High);
    scrollBarSMin->setValue(m_S_Low);
    scrollBarSMax->setValue(m_S_High);
    scrollBarIMin->setValue(m_I_Low);
    scrollBarIMax->setValue(m_I_High);

    scrollBarHMin->blockSignals(false);
    scrollBarHMax->blockSignals(false);
    scrollBarSMin->blockSignals(false);
    scrollBarSMax->blockSignals(false);
    scrollBarIMin->blockSignals(false);
    scrollBarIMax->blockSignals(false);

    UpdateHSIThreshold();
    drawHSIRing();
    drawBrightnessHistogram();
}

void ColorDlg::updateDisplayImage(const QPixmap& pixmap)
{
    if (!m_pDisplayLabel) return;

    // 保留原始帧，供采样时按原图坐标精确取色
    m_lastFrame = pixmap;

    // 保持宽高比缩放以适应显示区
    QPixmap scaled = pixmap.scaled(m_pDisplayLabel->size(), Qt::KeepAspectRatio, Qt::FastTransformation);

    // 叠加已有采样点标记
    if (!m_vecColorSet.isEmpty()) {
        QPixmap overlay(scaled);
        QPainter painter(&overlay);
        painter.setPen(QPen(Qt::red, 2));
        painter.setBrush(QBrush(Qt::red));

        // 由原图坐标换算到缩放后显示坐标（含居中偏移）
        double scaleX = double(scaled.width()) / double(pixmap.width());
        double scaleY = double(scaled.height()) / double(pixmap.height());
        int offsetX = (m_pDisplayLabel->width() - scaled.width()) / 2;
        int offsetY = (m_pDisplayLabel->height() - scaled.height()) / 2;

        for (const QPoint& p : m_vecColorSet) {
            int x = offsetX + int(p.x() * scaleX);
            int y = offsetY + int(p.y() * scaleY);
            painter.drawEllipse(QPoint(x, y), 4, 4);
        }
        painter.end();
        m_pDisplayLabel->setPixmap(overlay);
    }
    else {
        m_pDisplayLabel->setPixmap(scaled);
    }
}

// 重绘预览：叠加点采样标记与当前框选矩形
void ColorDlg::redrawPreview()
{
    if (!m_pDisplayLabel) return;

    QPixmap baseImage;
    QRect   baseSrcRect;   // baseImage 对应的原始图像区域

    if (!m_zoomSourceRect.isEmpty()) {
        // 放大模式：从原始帧截取放大区域作为底图
        if (m_lastFrame.isNull()) return;
        QImage fullImg = m_lastFrame.toImage().convertToFormat(QImage::Format_RGB888);
        QImage crop = fullImg.copy(m_zoomSourceRect);
        baseImage = QPixmap::fromImage(crop);
        baseSrcRect = m_zoomSourceRect;
    }
    else if (!m_lastFrame.isNull()) {
        baseImage = m_lastFrame;
        baseSrcRect = QRect(0, 0, m_lastFrame.width(), m_lastFrame.height());
    }
    else {
        return;
    }

    // 拉伸填满预览框，与 onZoom 同用 IgnoreAspectRatio 以免图像位移
    QSize avail = m_pDisplayLabel->size();
    QPixmap pix = baseImage.scaled(avail, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);

    double pixW = double(pix.width());
    double pixH = double(pix.height());

    // m_vecColorSet 仅用于色环标注，此处不绘制；
    // 点模式收集的坐标以蓝色十字标记在预览图上
    painter.setPen(QPen(Qt::blue, 2));
    for (const QPoint& p : m_points) {
        double rx = double(p.x() - baseSrcRect.left()) / baseSrcRect.width();
        double ry = double(p.y() - baseSrcRect.top())  / baseSrcRect.height();
        int px = int(rx * pixW);
        int py = int(ry * pixH);
        painter.drawLine(px - 4, py, px + 4, py);
        painter.drawLine(px, py - 4, px, py + 4);
    }

    // 绘制框选矩形：鼠标释放后仍保留（对齐 MFC GDI XOR 行为）
    if (!m_currentRect.isNull()) {
        painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
        painter.setBrush(Qt::NoBrush);
        QRect r = m_currentRect.normalized();
        // 裁剪到画布范围
        r = r.intersected(QRect(0, 0, pix.width() - 1, pix.height() - 1));
        if (!r.isEmpty()) {
            painter.drawRect(r);
        }
    }

    painter.end();
    m_pDisplayLabel->setPixmap(pix);
}

#if 0 // [已废弃] 旧版平均取色 — 已被 onSample() 的阈值筛选替代
// 在原始图像坐标的矩形上采样平均颜色并记录点（中心）
void ColorDlg::sampleAtImageRect(const QRect& imgRect)
{
    if (m_lastFrame.isNull()) return;
    QImage img = m_lastFrame.toImage().convertToFormat(QImage::Format_RGB888);
    QRect r = imgRect.intersected(img.rect());
    if (r.isEmpty()) return;
    long sumR = 0, sumG = 0, sumB = 0;
    int count = 0;
    for (int y = r.top(); y <= r.bottom(); ++y) {
        for (int x = r.left(); x <= r.right(); ++x) {
            QRgb rgb = img.pixel(x, y);
            sumR += qRed(rgb);
            sumG += qGreen(rgb);
            sumB += qBlue(rgb);
            ++count;
        }
    }
    if (count == 0) return;
    int ravg = int(sumR / count);
    int gavg = int(sumG / count);
    int bavg = int(sumB / count);

    QPoint center = QPoint((r.left() + r.right())/2, (r.top() + r.bottom())/2);
    m_vecColorSet.append(center);

    // 填亮度直方图
    for (int y = r.top(); y <= r.bottom(); ++y)
        for (int x = r.left(); x <= r.right(); ++x) {
            QRgb p = img.pixel(x, y);
            int I = (qRed(p) + qGreen(p) + qBlue(p)) / 3;
            if (I >= 0 && I < 255) yi[I]++;
        }

    // 用 HLUT 公式计算采样色块的平均 HSI（与 FindPixel 一致）
    int h = int(10.0 * HLUT[ravg][gavg][bavg]);
    int s = int(100.0 * (1.0 - 3.0 * Min3(ravg, gavg, bavg) / (ravg + gavg + bavg)));
    Debug::get()->print(QString("[Color] Rect sampled H=%1 S=%2 I=%3").arg(h).arg(s).arg((ravg+gavg+bavg)/3).toStdWString().c_str());
    redrawPreview();
}
#endif

bool ColorDlg::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_pDisplayLabel) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                QPoint pos = me->pos();
                if (m_SelectRect) {
                    m_selecting = true;
                    m_selectStart = pos;
                    m_currentRect = QRect(pos, pos);
                    redrawPreview();
                }
                else {
                    // 点模式：仅记录点击坐标，采样交由「采样」按钮处理。
                    // 坐标按预览图 IgnoreAspectRatio 拉伸比例逆映射回原图坐标
                    if (m_lastFrame.isNull()) return true;
                    int lw = m_pDisplayLabel->width();
                    int lh = m_pDisplayLabel->height();
                    if (lw <= 0 || lh <= 0) return true;
                    int ix = qBound(0, int(pos.x() * m_lastFrame.width()  / double(lw)), m_lastFrame.width()  - 1);
                    int iy = qBound(0, int(pos.y() * m_lastFrame.height() / double(lh)), m_lastFrame.height() - 1);
                    m_points.push_back(QPoint(ix, iy));
                    redrawPreview();
                }
            }
            return true;
        }
        else if (event->type() == QEvent::MouseMove) {
            if (m_selecting && m_SelectRect) {
                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                QPoint pos = me->pos();
                m_currentRect = QRect(m_selectStart, pos).normalized();
                redrawPreview();
                return true;
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton && m_selecting && m_SelectRect) {
                QPoint pos = me->pos();
                m_currentRect = QRect(m_selectStart, pos).normalized();
                // 框选结果留给「采样」按钮读取，此处不清空
                m_selecting = false;
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

#if 0 // [已废弃] 旧版单点平均取色 — 已被 onSample() 的阈值筛选替代
// 在原始图像坐标点附近（5x5）采样平均颜色并记录点
void ColorDlg::sampleAtImagePoint(const QPoint& imgPt)
{
    if (m_lastFrame.isNull()) return;
    QImage img = m_lastFrame.toImage().convertToFormat(QImage::Format_RGB888);
    int w = img.width(), h = img.height();
    int half = 2;
    int cx = imgPt.x();
    int cy = imgPt.y();
    long sumR = 0, sumG = 0, sumB = 0;
    int count = 0;
    for (int y = std::max(0, cy - half); y <= std::min(h-1, cy + half); ++y) {
        for (int x = std::max(0, cx - half); x <= std::min(w-1, cx + half); ++x) {
            QRgb rgb = img.pixel(x, y);
            sumR += qRed(rgb);
            sumG += qGreen(rgb);
            sumB += qBlue(rgb);
            ++count;
        }
    }
    if (count == 0) return;
    int ravg = int(sumR / count);
    int gavg = int(sumG / count);
    int bavg = int(sumB / count);
    m_vecColorSet.append(imgPt);

    // 填亮度直方图
    for (int y = std::max(0, cy - half); y <= std::min(h-1, cy + half); ++y)
        for (int x = std::max(0, cx - half); x <= std::min(w-1, cx + half); ++x) {
            QRgb p = img.pixel(x, y);
            int I = (qRed(p) + qGreen(p) + qBlue(p)) / 3;
            if (I >= 0 && I < 255) yi[I]++;
        }

    int hue = int(10.0 * HLUT[ravg][gavg][bavg]);
    int sat = int(100.0 * (1.0 - 3.0 * Min3(ravg, gavg, bavg) / (ravg + gavg + bavg)));
    Debug::get()->print(QString("[Color] Point sampled H=%1 S=%2 I=%3").arg(hue).arg(sat).arg((ravg+gavg+bavg)/3).toStdWString().c_str());
    redrawPreview();
}
#endif

void ColorDlg::onZoom()
{
    // 取左侧显示区的框选矩形并截取当前帧，拉伸填充右侧预览框
    DisplayDlg* dispDlg = this->window()->findChild<DisplayDlg*>();
    if (!dispDlg) return;

    QRect selRect = dispDlg->GetRect().normalized();

    // 未框选时提示用户先操作
    if (selRect.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先在显示区框选图像");
        return;
    }

    unsigned char* pSrc = dispDlg->getDispSingle();
    if (!pSrc || !m_pDisplayLabel) return;

    // 框选矩形裁剪到图像边界内
    QRect imgRect(0, 0, DISPLAY_W, DISPLAY_H);
    selRect = selRect.intersected(imgRect);

    if (selRect.isEmpty() || selRect.width() < 2 || selRect.height() < 2) return;

    // 逐行拷贝原始帧的框选区域到独立 QImage
    int sw = selRect.width();
    int sh = selRect.height();
    QImage cropped(sw, sh, QImage::Format_RGB888);
    for (int y = 0; y < sh; ++y) {
        unsigned char* pSrcRow = pSrc + ((selRect.top() + y) * DISPLAY_W + selRect.left()) * 3;
        unsigned char* pDstRow = cropped.scanLine(y);
        std::memcpy(pDstRow, pSrcRow, sw * 3);
    }

    // 拉伸填充右侧预览框
    QPixmap result = QPixmap::fromImage(cropped).scaled(
        m_pDisplayLabel->size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    m_pDisplayLabel->setPixmap(result);

    // 记录放大源矩形，供右侧框选逆映射回原图坐标
    m_zoomSourceRect = selRect;

    // 每次放大都重建全帧副本，因为底层帧可能已被 ShowSingle 更新
    QImage fullImg(pSrc, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
    m_lastFrame = QPixmap::fromImage(fullImg);
}

// 判断像素是否命中当前对象的 HSI 阈值，计算方式与 DisplayDlg::FindPixel 完全一致
bool ColorDlg::isPixelMatchingThreshold(int R, int G, int B)
{
    if (R + G + B == 0) return false;

    int H = 10 * HLUT[R][G][B];                              // H×10, 范围 0~3599
    int S = int(100 * (1 - 3.0 * Min3(R, G, B) / (R + G + B))); // 0~100
    int I = (R + G + B) / 3;                                   // 0~255

    int hLow  = HSIThreshold[m_object][0];
    int hHigh = HSIThreshold[m_object][1];
    int sLow  = HSIThreshold[m_object][2];
    int sHigh = HSIThreshold[m_object][3];
    int iLow  = HSIThreshold[m_object][4];
    int iHigh = HSIThreshold[m_object][5];

    // H 阈值支持跨 0°：当 hLow>hHigh 时取两段并集（OR）
    bool hOK;
    if (hHigh >= hLow)
        hOK = (H >= hLow && H <= hHigh);
    else
        hOK = (H >= hLow || H <= hHigh);

    return hOK && (S >= sLow && S <= sHigh) && (I >= iLow && I <= iHigh);
}

void ColorDlg::onSample()
{
    // 未框选时提示用户先操作
    if (m_currentRect.isNull() || m_currentRect.width() <= 1 || m_currentRect.height() <= 1) {
        QMessageBox::warning(this, "提示", "请先在预览图像上框选采样区域");
        return;
    }

    QImage img;
    QRect  sampleRect;

    if (!m_zoomSourceRect.isEmpty() && !m_lastFrame.isNull()) {
        // 放大模式：从全帧截取放大区域再采样
        img = m_lastFrame.toImage().convertToFormat(QImage::Format_RGB888);
        if (!m_currentRect.isNull() && m_currentRect.width() > 1 && m_currentRect.height() > 1) {
            // 将预览框选区逆映射回原图放大区域
            int lw = m_pDisplayLabel ? m_pDisplayLabel->width() : 1;
            int lh = m_pDisplayLabel ? m_pDisplayLabel->height() : 1;
            QRect cr = m_currentRect.normalized();
            double rx1 = double(cr.left())   / double(lw);
            double ry1 = double(cr.top())    / double(lh);
            double rx2 = double(cr.right())  / double(lw);
            double ry2 = double(cr.bottom()) / double(lh);
            int ix1 = m_zoomSourceRect.left() + int(rx1 * m_zoomSourceRect.width());
            int iy1 = m_zoomSourceRect.top()  + int(ry1 * m_zoomSourceRect.height());
            int ix2 = m_zoomSourceRect.left() + int(rx2 * m_zoomSourceRect.width());
            int iy2 = m_zoomSourceRect.top()  + int(ry2 * m_zoomSourceRect.height());
            sampleRect = QRect(QPoint(ix1, iy1), QPoint(ix2, iy2)).normalized();
        }
        else {
            // 无框选时默认取放大区域中心 5×5
            sampleRect = QRect(m_zoomSourceRect.center().x() - 2,
                               m_zoomSourceRect.center().y() - 2, 5, 5);
        }
    }
    else if (!m_lastFrame.isNull()) {
        // 非放大模式：直接在全帧上采样
        img = m_lastFrame.toImage().convertToFormat(QImage::Format_RGB888);
        if (!m_currentRect.isNull() && m_currentRect.width() > 1 && m_currentRect.height() > 1) {
            // 预览框选逆映射回全帧坐标（IgnoreAspectRatio，与 onZoom 一致）
            int lw = m_pDisplayLabel->width();
            int lh = m_pDisplayLabel->height();
            QRect cr = m_currentRect.normalized();
            int ix1 = qBound(0, int(cr.left()   * m_lastFrame.width()  / double(lw)), img.width()  - 1);
            int iy1 = qBound(0, int(cr.top()    * m_lastFrame.height() / double(lh)), img.height() - 1);
            int ix2 = qBound(0, int(cr.right()  * m_lastFrame.width()  / double(lw)), img.width()  - 1);
            int iy2 = qBound(0, int(cr.bottom() * m_lastFrame.height() / double(lh)), img.height() - 1);
            sampleRect = QRect(ix1, iy1, ix2 - ix1 + 1, iy2 - iy1 + 1);
        }
        else {
            sampleRect = QRect(img.width() / 2 - 2, img.height() / 2 - 2, 5, 5);
        }
    }
    else {
        return;
    }

    // ── 逐像素采样：区域内全部记录，不做阈值过滤（与 MFC ColorAnalyse 一致）──
    // 目的是让用户在色环上看到颜色分布后手动调滑块
    QRect r = sampleRect.intersected(img.rect());
    if (r.isEmpty()) return;

    // 清空旧采样数据，准备新一轮统计
    m_vecColorSet.clear();
    memset(yi, 0, sizeof(yi));

    int count = 0;

    for (int y = r.top(); y <= r.bottom(); ++y) {
        for (int x = r.left(); x <= r.right(); ++x) {
            QRgb rgb = img.pixel(x, y);
            int R = qRed(rgb), G = qGreen(rgb), B = qBlue(rgb);
            if (R + G + B == 0) continue;  // 跳过纯黑像素（无色相意义）

            int Ival = (R + G + B) / 3;

            // 无条件记录像素坐标，供色环标注
            m_vecColorSet.append(QPoint(x, y));
            if (Ival >= 0 && Ival < 255) yi[Ival]++;
            count++;
        }
    }

    if (count == 0) return;

    Debug::get()->print(QString("[Color] Sampled %1 pixels").arg(count).toStdWString().c_str());

    // 仅记录采样结果，不自动改滑块（与 MFC OnNewsample 一致），
    // 由用户依据色环分布手动调整阈值

    // 将当前滑块值写回数组（只写数组不改滑块），再刷新各视图
    UpdateHSIThreshold();

    drawHSIRing();
    drawBrightnessHistogram();
    redrawPreview();
}

void ColorDlg::onClearSamples()
{
    // 清空采样数据与点记录
    m_vecColorSet.clear();
    m_points.clear();
    memset(yi, 0, sizeof(yi));

    // 仅清框选矩形，保留放大图
    m_currentRect = QRect();
    redrawPreview();

    // 采样数据已空，重绘后色环黑点与直方图蓝柱随之消失
    drawHSIRing();

    drawBrightnessHistogram();
}

// ── 色环绘制（位图 + 叠加层）──────────────────────────────
// 加载 MFC 中的 HSICir.bmp 色环位图作为基底，
// 在上面叠加：阈值扇形、采样点标注、高亮当前点
void ColorDlg::drawHSIRing()
{
    if (!HSIdisplayLabel) return;

    // 加载 MFC 同款色环位图作为基底
    QPixmap ringBase(":/QtWidgetDesign/resources/HSICir.bmp");
    if (ringBase.isNull()) {
        drawHSIRingFallback();
        return;
    }

    QSize labelSize = HSIdisplayLabel->size();
    if (labelSize.isEmpty()) return;

    // 位图按比例缩放后居中绘制到画布
    QPixmap pix = ringBase.scaled(labelSize, Qt::KeepAspectRatio, Qt::FastTransformation);
    QPixmap canvas(labelSize);
    canvas.fill(Qt::white);

    int ox = (labelSize.width()  - pix.width())  / 2;
    int oy = (labelSize.height() - pix.height()) / 2;

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPixmap(ox, oy, pix);

    // 色环中心与半径映射 — MFC 中 HSICir.bmp 尺寸 203×202，
    // 色环中心约在 (100, 100)，S=0→0, S=100→100px
    double scaleX = double(pix.width())  / 203.0;
    double scaleY = double(pix.height()) / 202.0;
    double scale  = qMin(scaleX, scaleY);
    QPointF center(ox + 100.0 * scaleX, oy + 100.0 * scaleY);
    double outerR = 100.0 * scale;   // S=100 对应半径
    double innerR =  30.0 * scale;   // 白心内圆半径

    // ── 1. 阈值扇形框：所有对象灰色弧 + 当前对象黑色弧 ──
    //    完全复刻 MFC UpdateSelect + UpdateSelect1 的绘制方式
    //
    //    HSICir.bmp 色环布局（CW，y-down 顺时针）：
    //      H=  0°(红) → 3 点钟方向（右）
    //      H= 60°(黄) → 4-5 点钟方向（右下）
    //      H=120°(绿) → 7-8 点钟方向（左下）
    //      H=180°(青) → 9 点钟方向（左）
    //      H=240°(蓝) → 10-11 点钟方向（左上）
    //      H=300°(品) → 1-2 点钟方向（右上）
    //
    //    使用手动计算弧线路径（polyline 逼近），保证
    //    弧线端点与径向线段精确对齐，避免 Qt drawArc
    //    角度约定不一致导致的错位。
    auto drawArcLines = [&](int obj, const QColor& penColor) {
        int sLow  = HSIThreshold[obj][2];
        int sHigh = HSIThreshold[obj][3];
        int hLow  = HSIThreshold[obj][0];
        int hHigh = HSIThreshold[obj][1];
        if (sLow <= 0 && sHigh <= 0) return;

        double rIn  = sLow  * scale;
        double rOut = sHigh * scale;

        double hLowDeg  = hLow  / 10.0;
        double hHighDeg = hHigh / 10.0;

        double span = hHighDeg - hLowDeg;
        if (span < 0) span += 360.0;
        if (span < 1.0) return;

        // 手动生成弧线点集：+sin 对应色环顺时针布局
        const int N = qMax(int(span / 2.0), 8);  // 每 2° 一段，最少 8 段
        QPolygonF innerPts, outerPts;
        for (int i = 0; i <= N; ++i) {
            double deg = hLowDeg + span * i / N;
            double rad = deg * M_PI / 180.0;
            double c = cos(rad), s = sin(rad);
            innerPts << QPointF(center.x() + rIn * c,  center.y() + rIn * s);
            outerPts << QPointF(center.x() + rOut * c, center.y() + rOut * s);
        }

        painter.setPen(QPen(penColor, 1));
        painter.setBrush(Qt::NoBrush);

        // 用 polyline 逼近内外弧
        painter.drawPolyline(innerPts);
        painter.drawPolyline(outerPts);

        // 径向线段闭合扇形，与弧线端点对齐
        painter.drawLine(innerPts.first(), outerPts.first());
        painter.drawLine(innerPts.last(),  outerPts.last());
    };

    // 先画所有对象灰色弧线，跳过 S 全为零的未使用对象
    for (int i = 0; i < 8; ++i) {
        if (HSIThreshold[i][2] == 0 && HSIThreshold[i][3] == 0) continue;
        drawArcLines(i, QColor(180, 175, 170));  // 灰色
    }
    // 再用黑色叠加当前对象弧线（对应 MFC UpdateSelect1）
    if (m_object >= 0 && m_object < 8)
        drawArcLines(m_object, Qt::black);

    // ── 2. 标注所有采样点（对应 MFC DrawColorPoint）─────
    //    H,S 用 HLUT+Min3 公式，+sin 匹配色环 CW 布局
    //    关闭抗锯齿，避免黑点边缘与色环背景混合导致深浅不一
    painter.setRenderHint(QPainter::Antialiasing, false);
    if (!m_lastFrame.isNull() && !m_vecColorSet.isEmpty()) {
        QImage fullImg = m_lastFrame.toImage()
                             .convertToFormat(QImage::Format_RGB888);
        for (const QPoint& pt : m_vecColorSet) {
            if (pt.x() < 0 || pt.y() < 0 ||
                pt.x() >= fullImg.width() || pt.y() >= fullImg.height())
                continue;
            QRgb rgb = fullImg.pixel(pt.x(), pt.y());
            int R = qRed(rgb), G = qGreen(rgb), B = qBlue(rgb);
            if (R + G + B == 0) continue;
            double hDeg = HLUT[R][G][B];
            double sNorm = 1.0 - 3.0 * Min3(R, G, B) / (R + G + B);
            double aRad = hDeg * M_PI / 180.0;
            double d = innerR + sNorm * (outerR - innerR);
            double mx = center.x() + d * cos(aRad);
            double my = center.y() + d * sin(aRad);  // +sin 匹配色环顺时针布局
            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::black);
            painter.drawEllipse(QPointF(mx, my), 2.5, 2.5);
        }
    }

    painter.end();
    HSIdisplayLabel->setPixmap(canvas);
}

// ── 程序绘制色环（位图加载失败时的回退）──────────────
void ColorDlg::drawHSIRingFallback()
{
    if (!HSIdisplayLabel) return;
    int w = HSIdisplayLabel->width();
    int h = HSIdisplayLabel->height();
    if (w <= 0 || h <= 0) return;

    QPixmap pixmap(w, h);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    int cx = w / 2, cy = h / 2;
    int outerR = qMin(w, h) * 3 / 8;
    int innerR = qMin(w, h) / 8;

    for (int angle = 0; angle < 360; angle += 2) {
        double rad = angle * M_PI / 180.0;
        int x1 = cx + int(outerR * cos(rad));
        int y1 = cy - int(outerR * sin(rad));  // -sin 使 y 轴向上，构成标准色环
        int x2 = cx + int(innerR * cos(rad));
        int y2 = cy - int(innerR * sin(rad));
        QColor c; c.setHsv(angle, 255, 255);
        painter.setPen(QPen(c, 2));
        painter.drawLine(x1, y1, x2, y2);
    }
    painter.end();
    HSIdisplayLabel->setPixmap(pixmap);

    // 回退路径同样标注采样点，用 HLUT 公式且 -sin 以匹配标准色环布局
    if (!m_vecColorSet.isEmpty() && !m_lastFrame.isNull()) {
        QPixmap curPix = HSIdisplayLabel->pixmap();
        QPixmap overlay = curPix.isNull()
                              ? QPixmap(HSIdisplayLabel->size())
                              : curPix.copy();
        if (!overlay.isNull()) {
            QPainter p2(&overlay);
            p2.setRenderHint(QPainter::Antialiasing);
            QImage fullImg = m_lastFrame.toImage()
                                 .convertToFormat(QImage::Format_RGB888);
            for (const QPoint& pt : m_vecColorSet) {
                if (pt.x() < 0 || pt.y() < 0 ||
                    pt.x() >= fullImg.width() || pt.y() >= fullImg.height())
                    continue;
                QRgb rgb = fullImg.pixel(pt.x(), pt.y());
                int R = qRed(rgb), G = qGreen(rgb), B = qBlue(rgb);
                if (R + G + B == 0) continue;
                double hDeg = HLUT[R][G][B];
                double sNorm = 1.0 - 3.0 * Min3(R, G, B) / (R + G + B);
                double aRad2 = hDeg * M_PI / 180.0;
                double d2 = innerR + sNorm * (outerR - innerR);
                double mx2 = cx + d2 * cos(aRad2);
                double my2 = cy - d2 * sin(aRad2);  // -sin 使 y 轴向上
                p2.setPen(Qt::NoPen);
                p2.setBrush(Qt::black);
                p2.drawEllipse(QPointF(mx2, my2), 2.5, 2.5);
            }
            p2.end();
            HSIdisplayLabel->setPixmap(overlay);
        }
    }
}

// ── 亮度直方图绘制 ───────────────────────────────────
void ColorDlg::drawBrightnessHistogram()
{
    if (!brightnessGraphLabel) return;
    int w = brightnessGraphLabel->width();
    int h = brightnessGraphLabel->height();
    if (w <= 4 || h <= 4) return;

    QPixmap pix(w, h);
    pix.fill(QColor(240, 240, 240));
    QPainter painter(&pix);

    // 找 yi[] 峰值用于柱高归一化
    int maxVal = 1;
    for (int i = 0; i < 255; i++)
        if (yi[i] > maxVal) maxVal = yi[i];

    double barW = (double)(w - 4) / 255.0;       // 左右各留 2px 边距
    int plotH   = h - 6;                          // 上下留边

    for (int i = 0; i < 255; i++) {
        int barH = int((double)yi[i] / maxVal * plotH);
        if (barH <= 0) continue;
        QColor c(0, 0, 255);  // 蓝色柱，与 MFC DrawPoint 的 RGB(0,0,255) 一致
        painter.fillRect(QRectF(2 + i * barW, h - 3 - barH,
                                 qMax(1.0, barW), (double)barH), c);
    }

    // 标注 I 低/高阈值红线
    if (w > 4) {
        double lowX = 2 + (m_I_Low / 255.0) * (w - 4);
        double highX= 2 + (m_I_High/ 255.0) * (w - 4);
        painter.setPen(QPen(Qt::red, 2));
        painter.drawLine(QPointF(lowX, 0), QPointF(lowX, h - 1));
        painter.drawLine(QPointF(highX, 0), QPointF(highX, h - 1));
    }

    painter.end();
    brightnessGraphLabel->setPixmap(pix);
}