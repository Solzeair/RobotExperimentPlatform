// DisplayDlg.cpp - Image display area implementation
// Handles camera capture, target recognition, colour analysis,
// and football robot match visualisation.
#include "DisplayDlg.h"
#include "Camera.h"
#include "Debug.h"
#include "ColorDlg.h"
#include "DemarcateDlg.h"   // Needed to forward BORDER_SET clicks
#include <cmath>
#include <QDebug>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════
// ColorDlg 对象编号到内部语义的映射：UI 文字保持不变，仅用于识别逻辑区分颜色对象
// ═══════════════════════════════════════════════════════════════
static const int QT_TEAM  = 0;  // 我方队色
static const int QT_MEMB1 = 1;  // 紫色
static const int QT_MEMB2 = 2;  // 绿色
static const int QT_BALL  = 3;  // 球色
static const int QT_OPP   = 4;  // 敌方队色

// RGB 三通道最小值，用于 HSI 饱和度计算
inline int MIN(int a, int b, int c, int n)
{
    int min_val = a;
    if (b < min_val) min_val = b;
    if (c < min_val) min_val = c;
    return min_val;
}

// RGB->H 查找表：H 通过查表加速，S/I 用公式实时计算
int HLUT[256][256][256];    //RGB-H 转换表，S,I值分别用公式计算

// 帧抓取预分配缓冲区，避免每帧 malloc/free 抖动
static unsigned char* pBuffer = nullptr;

// 机器人外形 12 个关键点相对中心的偏移坐标，按朝向 0~360 度索引
int robot_xy[361][12][2];             //机器人方向图像关键点坐标

// ========================================================================
// OverlayWidget - 透明覆盖层控件
// 叠在 displayLabel 上方，用于采色模式下绘制鼠标框选矩形。
// 用透明背景实现"只在框选时可见"的效果，对应 MFC 的 CDC 绘制层。
// ========================================================================

// 构造函数：透明背景 + 接收鼠标事件，使覆盖层可框选但不遮挡底层画面
OverlayWidget::OverlayWidget(QWidget* parent)
    : QWidget(parent)
    , m_hasSelection(false)
{
    setStyleSheet("background: transparent;");
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

// 设置框选矩形并请求重绘（覆盖层坐标系与图像坐标对齐）
void OverlayWidget::setSelectionRect(const QRect& r)
{
    m_selectionRect = r;
    m_hasSelection = true;
    update();  // 触发 paintEvent 重绘
}

// 清除框选矩形并请求重绘（切换标签页或重新框选时调用）
void OverlayWidget::clearSelectionRect()
{
    m_hasSelection = false;
    m_selectionRect = QRect();
    update();  // 触发 paintEvent 重绘（paintEvent 中 hasSelection=false 会跳过绘制）
}

// 绘制框选矩形：深红线框，颜色固定不随背景变化，保证任意背景下都清晰可见
void OverlayWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if (!m_hasSelection)
        return;

    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setPen(QPen(QColor(200, 0, 0), 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_selectionRect);
}

// 鼠标按下：记录框选起点，初始化 1x1 矩形
void OverlayWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_selectionRect = QRect(event->pos(), QSize(1, 1));
        m_hasSelection = true;
        update();  // 触发 paintEvent，画出初始的 1×1 矩形（实际不可见）
    }
}

// 鼠标拖动：更新矩形终点并重绘。
// 与 MFC 需 XOR 擦旧矩形不同，Qt 每帧从干净状态重画，无需手动擦除。
void OverlayWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        // normalized 处理反向拖拽，保证矩形坐标始终 left<=right, top<=bottom
        m_selectionRect.setBottomRight(event->pos());
        m_selectionRect = m_selectionRect.normalized();
        update();  // 触发 paintEvent 重绘
    }
}

// 初始化显示区域：预建图像缓冲、机器人外形坐标表和 RGB-H 查找表

DisplayDlg::DisplayDlg(QWidget* parent)
    : QWidget(parent)
    , m_ImageSize(DISPLAY_W, DISPLAY_H)
    , m_bErrorSign(false)
    , m_status(STATUS::Stop)
    , m_setStatus(SET_STATUS::NONE)
    , m_IdenOp(true)
    , stackPointer(0)
    , m_xLeft(0)
    , m_xRight(DISPLAY_W)
    , m_yTop(0)
    , m_yBottom(DISPLAY_H)
    , patchConnect(false)
    , m_BalLo(false)
    , m_fps(0.0)
    , m_Length(7.5)
    , m_overlayWidget(nullptr)
{
    // 双缓冲：DispBitmap 用于动态显示，DispSingle 用于单帧抓取；Identify 默认指向动态帧
    m_pDispBitmap = new unsigned char[m_ImageSize.width() * m_ImageSize.height() * 3]();
    m_pDispSingle = new unsigned char[m_ImageSize.width() * m_ImageSize.height() * 3]();
    m_pIdentify = m_pDispBitmap;

    // 预计算机器人外形 12 关键点：4 角顶点 + 中点 + 朝向指示点，按 0~360 度索引
    double ttheta, side = 7.5 * 1.25;
    for (int i = 0; i <= 360; i++)
    {
        ttheta = 3.1415926 * (2 - (double)i / 180);
        robot_xy[i][0][0] = (int)(side * cos(ttheta) - side * sin(ttheta));
        robot_xy[i][0][1] = (int)(side * sin(ttheta) + side * cos(ttheta));
        robot_xy[i][1][0] = (int)(side * cos(ttheta) + side * sin(ttheta));
        robot_xy[i][1][1] = (int)(side * sin(ttheta) - side * cos(ttheta));
        robot_xy[i][2][0] = (int)(-side * cos(ttheta) - side * sin(ttheta));
        robot_xy[i][2][1] = (int)(-side * sin(ttheta) + side * cos(ttheta));
        robot_xy[i][3][0] = (int)(-side * cos(ttheta) + side * sin(ttheta));
        robot_xy[i][3][1] = (int)(-side * sin(ttheta) - side * cos(ttheta));

        robot_xy[i][4][0] = (robot_xy[i][0][0] + robot_xy[i][1][0]) / 2;
        robot_xy[i][4][1] = (robot_xy[i][0][1] + robot_xy[i][1][1]) / 2;
        robot_xy[i][5][0] = (robot_xy[i][2][0] + robot_xy[i][3][0]) / 2;
        robot_xy[i][5][1] = (robot_xy[i][2][1] + robot_xy[i][3][1]) / 2;
        robot_xy[i][6][0] = (robot_xy[i][0][0] + robot_xy[i][2][0]) / 2;
        robot_xy[i][6][1] = (robot_xy[i][0][1] + robot_xy[i][2][1]) / 2;
        robot_xy[i][7][0] = (robot_xy[i][1][0] + robot_xy[i][3][0]) / 2;
        robot_xy[i][7][1] = (robot_xy[i][1][1] + robot_xy[i][3][1]) / 2;
        robot_xy[i][8][0] = robot_xy[i][6][0] / 2;
        robot_xy[i][8][1] = robot_xy[i][6][1] / 2;
        robot_xy[i][9][0] = robot_xy[i][7][0] / 2;
        robot_xy[i][9][1] = robot_xy[i][7][1] / 2;

        robot_xy[i][10][0] = (robot_xy[i][4][0] + robot_xy[i][6][0]) / 2;
        robot_xy[i][10][1] = (robot_xy[i][4][1] + robot_xy[i][6][1]) / 2;
        robot_xy[i][11][0] = (robot_xy[i][4][0] + robot_xy[i][7][0]) / 2;
        robot_xy[i][11][1] = (robot_xy[i][4][1] + robot_xy[i][7][1]) / 2;
    }

    // 加载预生成的 RGB-H 查找表，文件缺失时回退为全 0（H 恒为 0）
    QFile file("resources/HLUT.dat");
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        if (data.size() >= 256 * 256 * 256 * sizeof(int))
        {
            memcpy(HLUT, data.data(), 256 * 256 * 256 * sizeof(int));
        }
        file.close();
    }
    else
    {
        for (int r = 0; r < 256; r++)
        {
            for (int g = 0; g < 256; g++)
            {
                for (int b = 0; b < 256; b++)
                {
                    HLUT[r][g][b] = 0;
                }
            }
        }
    }

    // 机器人/球信息归零，并备份副本用于滤波防抖
    for (int i = 0; i < MAX_ROBOT_NUM; i++)
    {
        robotInfor[i].x = 0.0;
        robotInfor[i].y = 0.0;
        robotInfor[i].theta = 0.0;
        robotInfor[i].num = i;
        robotInfor[i].found = false;
        robotBk[i] = robotInfor[i];
        OpprobotInfor[i] = robotInfor[i];
        OpprobotBk[i] = robotInfor[i];
    }
    ballInfor.x = 0.0;
    ballInfor.y = 0.0;
    ballInfor.theta = 0.0;
    ballInfor.found = false;
    ballBk = ballInfor;

    // 启动时预加载车号图，避免标签页切换时磁盘 IO 卡顿
    m_carNumPixmap = QPixmap("resources/carnum.bmp");

    initUI();
}

// 释放图像缓冲、定时器和共享抓取缓冲区

DisplayDlg::~DisplayDlg()
{
    if (m_pDispBitmap)
        delete[] m_pDispBitmap;
    if (m_pDispSingle)
        delete[] m_pDispSingle;
    if (m_grabTimer)
        delete m_grabTimer;
    if (fpsTimer)
        delete fpsTimer;
    // 释放共享抓取缓冲区
    if (pBuffer)
        delete[] pBuffer;
}

// 构建界面：fpsLabel + displayLabel + 采色覆盖层，并启动帧率定时器

void DisplayDlg::initUI()
{
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // fpsLabel 高度与标签栏一致，保证显示区域上下对齐
    fpsLabel = new QLabel(this);
    fpsLabel->setStyleSheet("QLabel { background-color: transparent; color: black; font-size: 12px; padding: 2px; font-weight: bold; }");
    fpsLabel->setText("FPS: 0");
    fpsLabel->setFixedSize(DISPLAY_W, 32); // 与QTabWidget标签栏高度一致
    fpsLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    displayLabel = new QLabel(this);
    displayLabel->setStyleSheet("QLabel { background-color: #333333; border: 1px solid black; color: white; }");
    displayLabel->setFixedSize(DISPLAY_W, DISPLAY_H);
    displayLabel->setAlignment(Qt::AlignCenter);
    displayLabel->setText("显示区域");

    mainLayout->addWidget(fpsLabel);
    mainLayout->addWidget(displayLabel);

    // 采色覆盖层：作为 displayLabel 子控件完全重叠，本地坐标即图像坐标，无需转换
    m_overlayWidget = new OverlayWidget(displayLabel);
    m_overlayWidget->setFixedSize(DISPLAY_W, DISPLAY_H);
    m_overlayWidget->move(0, 0);  // 与 displayLabel 左上角对齐
    m_overlayWidget->hide();       // 初始隐藏，仅在 COLOR_SET 模式下显示

    // 抓帧定时器驱动 onTimer；帧率定时器每 500ms 刷新 FPS 显示
    m_grabTimer = new QTimer(this);
    connect(m_grabTimer, &QTimer::timeout, this, &DisplayDlg::onTimer);

    fpsTimer = new QTimer(this);
    connect(fpsTimer, &QTimer::timeout, this, &DisplayDlg::updateFPS);
    fpsTimer->start(500); // 每500ms更新一次，与MFC版本保持一致

    // 场地背景图，加载失败时回退纯绿色
    m_groundImage.load("resources/ground.bmp");
    if (m_groundImage.isNull()) {
        m_groundImage = QImage(DISPLAY_W, DISPLAY_H, QImage::Format_RGB32);
        m_groundImage.fill(QColor(0, 128, 0));
    }

    // 启动占位图，避免摄像头未开启时显示空白
    QPixmap placeholder(DISPLAY_W, DISPLAY_H);
    placeholder.fill(QColor(80, 80, 80));
    QPainter ph(&placeholder);
    ph.setPen(Qt::white);
    ph.setFont(QFont("Arial", 14));
    ph.drawText(placeholder.rect(), Qt::AlignCenter, "Camera not started");
    displayLabel->setPixmap(placeholder);
}

// 抓取单帧并显示，叠加标定点十字标记
void DisplayDlg::ShowSingle()
{
    Camera* pCamera = Camera::GetInstance();

    if (!pCamera->IsOpen()) {
        pCamera->Open();
    }
    if (!pCamera->IsGrabbing()) {
        pCamera->StartGrabbing();
    }

    if (pCamera->RetrieveResult(m_pDispSingle)) {

        QImage image(m_pDispSingle, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(image);

        // 标定点以红色十字 + 序号渲染，供 BORDER_SET 标定查看
        if (!m_calibPoints.empty()) {
            QPainter p(&pixmap);
            p.setPen(QPen(Qt::red, 2));
            QFont f;
            f.setPointSize(8);
            p.setFont(f);

            for (int i = 0; i < (int)m_calibPoints.size(); ++i) {
                const QPoint& pt = m_calibPoints[i];
                p.drawLine(pt.x() - 6, pt.y(), pt.x() + 6, pt.y());
                p.drawLine(pt.x(), pt.y() - 6, pt.x(), pt.y() + 6);
                p.drawText(pt.x() + 4, pt.y() - 4, QString::number(i + 1));
            }
        }

        displayLabel->setPixmap(pixmap);
    }
}

// 启动定时抓帧并实时显示画面

void DisplayDlg::ShowDynamic()
{
    Stop();

    Camera* pCamera = Camera::GetInstance();
    if (!pCamera->IsOpen()) {
        if (!pCamera->Open()) {
            displayLabel->setText("无法打开摄像头");
            return;
        }
    }

    if (!pCamera->IsGrabbing()) {
        pCamera->StartGrabbing();
    }

    m_status = STATUS::Display;

    m_DisplayWatch.start();

    m_grabTimer->start(50); // 与MFC版本保持一致

    // 立即抓一帧填充显示，避免切换时短暂黑屏
    unsigned char* tempBuffer = new unsigned char[DISPLAY_W * DISPLAY_H * 3];
    if (pCamera->RetrieveResult(tempBuffer)) {
        pCamera->ConvertBitmap(m_pDispBitmap, tempBuffer, DISPLAY_W, DISPLAY_H);
        QImage image(m_pDispBitmap, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(image);
        displayLabel->setPixmap(pixmap);
    }
    delete[] tempBuffer;
}

// 显示车号图，加载失败时回退文字提示
void DisplayDlg::ShowCarNum()
{
    Camera* pCamera = Camera::GetInstance();
    if (pCamera->IsGrabbing()) {
        this->Stop();
    }
    if (!m_carNumPixmap.isNull()) {
        displayLabel->setPixmap(m_carNumPixmap);
    }
    else {
        displayLabel->setText("车号显示");
    }
}

// 颜色阈值预览：保留符合阈值的像素，其余置白，用于采色调试

void DisplayDlg::ShowColorTest(int(*HSI)[6], int object)
{
    if (!(m_status == STATUS::Stop || m_status == STATUS::Prepare))
    {
        this->Stop();
    }
    GrabSingle();

    unsigned char* m_pTestBitmap;
    m_pTestBitmap = new unsigned char[m_ImageSize.width() * m_ImageSize.height() * 3];
    unsigned char* pOrigin = m_pDispSingle;
    unsigned char* pTest = m_pTestBitmap;

    int i, j, R, G, B, H = 0, S, I;
    if (HSI[object][1] > HSI[object][0])
    {
        for (j = 0; j < DISPLAY_H; j++)
            for (i = 0; i < DISPLAY_W; i++) {
                // 像素字节序为 R,G,B（Pylon RGB8packed）；H 域用 &&（非跨 0）
                R = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0);
                G = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1);
                B = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2);
                H = 10 * HLUT[R][G][B];
                S = int(100 * (1 - 3.0 * MIN(R, G, B, 3) / (R + G + B)));
                I = int((R + G + B) / 3);
                if (H >= HSI[object][0] && H <= HSI[object][1] && S >= HSI[object][2] && S <= HSI[object][3] && I >= HSI[object][4] && I <= HSI[object][5])
                {
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0) = *(pOrigin + (i + (DISPLAY_H - j) * DISPLAY_W) * 3 + 0);
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1) = *(pOrigin + (i + (DISPLAY_H - j) * DISPLAY_W) * 3 + 1);
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2) = *(pOrigin + (i + (DISPLAY_H - j) * DISPLAY_W) * 3 + 2);
                }
                else
                {
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0) = 255;
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1) = 255;
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2) = 255;
                }
            }
    }
    else if (HSI[object][1] < HSI[object][0])
    {
        for (j = 0; j < DISPLAY_H; j++)
            for (i = 0; i < DISPLAY_W; i++) {
                // H 跨越 0° 时用 || 判断，S/I 仍用 &&
                R = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0);
                G = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1);
                B = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2);
                H = 10 * HLUT[R][G][B];
                S = int(100 * (1 - 3.0 * MIN(R, G, B, 3) / (R + G + B)));
                I = int((R + G + B) / 3);
                if (H >= HSI[object][0] || H <= HSI[object][1] && S >= HSI[object][2] && S <= HSI[object][3] && I >= HSI[object][4] && I <= HSI[object][5])
                {
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0) = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0);
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1) = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1);
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2) = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2);
                }
                else
                {
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0) = 255;
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1) = 255;
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2) = 255;
                }
            }
    }

    QImage image(m_pTestBitmap, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image);
    displayLabel->setPixmap(pixmap);
    delete[] m_pTestBitmap;
}

// 启动测试模式：复位识别状态，首帧同步识别并建立滤波基线，随后定时刷新

void DisplayDlg::ShowRunTest(bool ImageSeg)
{
    Stop();

    Camera* pCamera = Camera::GetInstance();
    if (!pCamera->IsOpen()) {
        if (!pCamera->Open()) {
            displayLabel->setText("无法打开摄像头");
            return;
        }
    }
    if (!pCamera->IsGrabbing()) {
        pCamera->StartGrabbing();
    }

    for (int i = 0; i < MAX_ROBOT_NUM; i++) {
        robotInfor[i].x = 0.0;    robotInfor[i].y = 0.0;    robotInfor[i].theta = 0.0;
        robotBk[i]    = robotInfor[i];
        OpprobotInfor[i].x = 0.0; OpprobotInfor[i].y = 0.0; OpprobotInfor[i].theta = 0.0;
        OpprobotBk[i] = OpprobotInfor[i];
        ObjectFound[i] = false;
    }
    ballInfor.x = 0.0;  ballInfor.y = 0.0;  ballInfor.theta = 0.0;
    ballBk = ballInfor;

    if (ImageSeg)
        m_status = STATUS::RunTestSeg;
    else
        m_status = STATUS::RunTest;

    m_DisplayWatch.start();

    // 同步抓首帧并识别，避免 paintEvent 在首帧到达前用 (0,0) 旧数据绘制
    if (!pCamera->IsOpen()) pCamera->Open();
    if (!pCamera->IsGrabbing()) pCamera->StartGrabbing();
    {
        unsigned char* tempBuf = new unsigned char[DISPLAY_W * DISPLAY_H * 3];
        if (pCamera->RetrieveResult(tempBuf)) {
            memcpy(m_pDispBitmap, tempBuf, DISPLAY_W * DISPLAY_H * 3);
            m_pIdentify = m_pDispBitmap;
            IdentifyAll();

            // 首帧识别结果存入备份，作为后续滤波的基线，防止定时器首帧误检冲掉正确位置
            for (int k = 0; k < MAX_ROBOT_NUM; k++) {
                robotBk[k] = robotInfor[k];
                OpprobotBk[k] = OpprobotInfor[k];
            }
            ballBk = ballInfor;
        }
        delete[] tempBuf;
    }

    m_grabTimer->start(33);
}

// 进入预备态：复位目标位置、识别当前帧并启动持续刷新
void DisplayDlg::ShowInitGame()
{
    this->GrabSingle();
    for (int i = 0; i < MAX_ROBOT_NUM; i++) {
        robotInfor[i].x = 0.0;
        robotInfor[i].y = 0.0;
        robotInfor[i].theta = 0.0;
        robotBk[i].x = 0.0;
        robotBk[i].y = 0.0;
        robotBk[i].theta = 0.0;
        OpprobotInfor[i].x = 0.0;
        OpprobotInfor[i].y = 0.0;
        OpprobotBk[i].x = 0.0;
        OpprobotBk[i].y = 0.0;
    }
    ballInfor.x = 0.0;
    ballInfor.y = 0.0;
    ballInfor.theta = 0.0;
    ballBk.x = 0.0;
    ballBk.y = 0.0;
    ballBk.theta = 0.0;

    ClearBallTrail();

    m_pIdentify = m_pDispSingle;
    IdentifyAll();
    m_status = STATUS::Prepare;

    if (!m_grabTimer->isActive()) {
        m_grabTimer->start(33); // 约30fps
    }
    m_DisplayWatch.start();
    this->repaint(); // 使用repaint立即重绘
}

// 进入比赛态

void DisplayDlg::ShowStartGame()
{
    m_status = STATUS::Game;
    StartGame();
}

// 停止抓取并复位状态（错误标志置位时跳过，避免异常状态下误操作）

void DisplayDlg::Stop()
{
    if (!m_bErrorSign) {
        m_grabTimer->stop();

        Camera* pCamera = Camera::GetInstance();
        if (pCamera->IsGrabbing()) {
            pCamera->StopGrabbing();
        }

        m_status = STATUS::Stop;
    }
}

// 切换操作状态并管理采色覆盖层可见性：COLOR_SET 显示覆盖层供框选，其余状态隐藏避免干扰

void DisplayDlg::SelectSetStatus(SET_STATUS s)
{
    m_setStatus = s;

    if (m_overlayWidget) {
        if (s == SET_STATUS::COLOR_SET) {
            m_overlayWidget->clearSelectionRect();
            m_overlayWidget->show();
            m_overlayWidget->raise();
        }
        else {
            m_overlayWidget->hide();
        }
    }
}

// 框选矩形（图像坐标）：覆盖层坐标即图像坐标，无需转换；后备返回 m_Rect 供 BORDER_SET 使用

QRect DisplayDlg::GetRect() const
{
    if (m_overlayWidget && m_overlayWidget->hasSelection()) {
        return m_overlayWidget->getSelectionRect();
    }
    return m_Rect;
}

// 清除采色覆盖层上的框选矩形
void DisplayDlg::clearOverlaySelection()
{
    if (m_overlayWidget) m_overlayWidget->clearSelectionRect();
}

// 设置采色对话框实际实例指针
void DisplayDlg::setColorDlg(ColorDlg* dlg)
{
    m_pColorDlg = dlg;
}

// 绘制场地背景 + 识别结果叠加（比赛/预备/非分割测试态）
// 分割模式 RunTestSeg 由 IdentifyTest() 直接写 displayLabel，不走此处，避免场地背景覆盖分割图

void DisplayDlg::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if (m_status == STATUS::Game || m_status == STATUS::Prepare ||
        m_status == STATUS::RunTest) {
        QPixmap pixmap(DISPLAY_W, DISPLAY_H);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);

        painter.drawImage(0, 0, m_groundImage);

        DrawRobot(&painter);

        DrawOpp(&painter);

        DrawBall(&painter);

        displayLabel->setPixmap(pixmap);
    }
}

// 清除足球轨迹

void DisplayDlg::ClearBallTrail()
{
    m_ballTrail.clear();
}

// 保留空实现：COLOR_SET 框选已交由 OverlayWidget，BORDER_SET 拖拽如需可在此扩展

void DisplayDlg::mouseMoveEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
}

// BORDER_SET 标定：记录标定点并转发给 DemarcateDlg，同时在画面上叠加十字标记

void DisplayDlg::mousePressEvent(QMouseEvent* event)
{
    // DisplayDlg 整体坐标需减去 fpsLabel 高度才得到图像坐标
    QPoint pos = event->pos();

    if (m_setStatus == SET_STATUS::BORDER_SET) {
        QPoint imagePos(pos.x(), pos.y() - fpsLabel->height());

        if (imagePos.x() < 0 || imagePos.x() >= DISPLAY_W ||
            imagePos.y() < 0 || imagePos.y() >= DISPLAY_H)
            return;

        addCalibPoint(imagePos);

        if (m_pDemarcateDlg) {
            m_pDemarcateDlg->PushPoint(imagePos);
        }

        // 用最近一帧重绘，叠加所有已标记的十字
        QImage image(m_pDispSingle, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(image);
        QPainter p(&pixmap);
        p.setPen(QPen(Qt::red, 2));
        QFont f;
        f.setPointSize(8);
        p.setFont(f);
        for (int i = 0; i < (int)m_calibPoints.size(); ++i) {
            const QPoint& pt = m_calibPoints[i];
            p.drawLine(pt.x() - 6, pt.y(), pt.x() + 6, pt.y());
            p.drawLine(pt.x(), pt.y() - 6, pt.x(), pt.y() + 6);
            p.drawText(pt.x() + 4, pt.y() - 4, QString::number(i + 1));
        }
        displayLabel->setPixmap(pixmap);

    }
    // COLOR_SET 框选由 OverlayWidget 直接处理，此处不再介入
}

// 定时抓帧并按当前状态分发处理

void DisplayDlg::onTimer()
{
    Camera* pCamera = Camera::GetInstance();
    if (!pCamera->IsOpen()) {
        if (!pCamera->Open()) {
            return;
        }
    }
    if (!pCamera->IsGrabbing()) {
        pCamera->StartGrabbing();
    }

    // 共享抓取缓冲区按需分配，仅创建一次
    if (!pBuffer) {
        pBuffer = new unsigned char[DISPLAY_W * DISPLAY_H * 3];
    }

    if (pCamera->RetrieveResult(pBuffer)) {
        ProcessImage(pBuffer);
    }

    if (m_status == STATUS::PrepareStop) {
        pCamera->StartGrabbing();
        m_status = STATUS::Stop;
    }
}



// 由平均帧间隔换算实时 FPS 并刷新显示

void DisplayDlg::updateFPS()
{
    double avg = m_DisplayAvg.Avg();
    m_fps = avg == 0 ? 0.0 : 1000.0 / avg;
    m_DisplayAvg.Reset();
    fpsLabel->setText(QString("FPS: %1").arg(m_fps, 0, 'f', 2));
}

// 按状态分发：Display 直显，RunTest 识别+重绘，RunTestSeg 分割直显，Game 识别

void DisplayDlg::ProcessImage(unsigned char* pBmp)
{
    Camera* pCamera = Camera::GetInstance();
    // pCamera->ConvertBitmap(m_pDispBitmap, pBmp, m_ImageSize.width(), m_ImageSize.height());
    memcpy(m_pDispBitmap, pBmp, m_ImageSize.width() * m_ImageSize.height() * 3);

    switch (m_status) {
    case STATUS::Display:
    {
        QImage image(m_pDispBitmap, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(image);
        displayLabel->setPixmap(pixmap);
        m_DisplayAvg.Add(m_DisplayWatch.elapsed());
        m_DisplayWatch.restart();
    }
    break;
    case STATUS::RunTest:
    {
        this->StartTest();
        m_DisplayAvg.Add(m_DisplayWatch.elapsed());
        m_DisplayWatch.restart();
        this->repaint();  // 触发 paintEvent 在场地背景上绘制识别结果
    }
    break;
    case STATUS::RunTestSeg:
    {
        // 分割模式 IdentifyTest 直接写 displayLabel，无需 repaint
        this->IdentifyTest();
        m_DisplayAvg.Add(m_DisplayWatch.elapsed());
        m_DisplayWatch.restart();
    }
    break;
    case STATUS::Game:
    {
        this->StartGame();
        m_DisplayAvg.Add(m_DisplayWatch.elapsed());
        m_DisplayWatch.restart();
    }
    break;
    default:
        break;
    }
}

// 抓单帧到 m_pDispSingle：暂停连续抓取后单帧取图，结束恢复原抓取状态

bool DisplayDlg::GrabSingle()
{
    Camera* pCamera = Camera::GetInstance();

    bool wasGrabbing = pCamera->IsGrabbing();
    if (wasGrabbing) {
        pCamera->StopGrabbing();
    }

    if (!pCamera->IsOpen()) {
        if (!pCamera->Open()) {
            if (wasGrabbing) pCamera->StartGrabbing();
            return false;
        }
    }

    // 统一用 StartGrabbing+RetrieveResult 取图，保证与动态模式格式一致
    bool result = false;
    if (pCamera->StartGrabbing()) {
        result = pCamera->RetrieveResult(m_pDispSingle);
        pCamera->StopGrabbing();
    }

    if (wasGrabbing) {
        pCamera->StartGrabbing();
    }

    return result;
}

// 坐标点栈：泛洪填充用，溢出栈满返回 false

bool DisplayDlg::pop(int& x, int& y)
{
    if (stackPointer > 0)
    {
        stackPointer--;
        x = stackx[stackPointer];
        y = stacky[stackPointer];
        return true;
    }
    else
        return false;
}

bool DisplayDlg::push(int x, int y)
{
    if (stackPointer < StackSize)
    {
        stackx[stackPointer] = x;
        stacky[stackPointer] = y;
        stackPointer++;
        return true;
    }
    else
        return false;
}

void DisplayDlg::emptyStack()
{
    stackPointer = 0;
}

// 单像素 RGB→HSI：H 查表（放大 10 倍便于阈值比较），S/I 公式计算

void DisplayDlg::RGBToHS(int m, int n, unsigned char* P, int& H, int& S, int& I)
{
    int R, G, B;
    int index = (n * m_ImageSize.width() + m) * 3;
    R = *(P + index + 0);
    G = *(P + index + 1);
    B = *(P + index + 2);
    H = 10 * HLUT[R][G][B];
    S = int(100 * (1 - 3.0 * MIN(R, G, B, 3) / (R + G + B)));
    I = (int)(R + G + B) / 3;
}

// 一次性绘制场地背景 + 己方/对方机器人 + 球

void DisplayDlg::DrawAll(QPainter* painter)
{
    painter->drawImage(0, 0, m_groundImage);

    DrawRobot(painter);

    DrawOpp(painter);

    DrawBall(painter);
}

// 绘制对方机器人：品红描边 + 绿色填充，按朝向查 robot_xy 关键点连线

void DisplayDlg::DrawOpp(QPainter* painter)
{
    painter->setPen(QPen(Qt::magenta, 1));
    painter->setBrush(QBrush(Qt::green));

    for (int i = 0; i < MAX_ROBOT_NUM; i++)
    {
        if (OpprobotInfor[i].found)
        {
            // 场地坐标→屏幕坐标的固定缩放偏移
            int x = (int)(OpprobotInfor[i].x * 2.5) + 45;
            int y = (int)(OpprobotInfor[i].y * 2.5) + 15;
            int theta = (int)OpprobotInfor[i].theta;
            if (theta < 0) theta += 360;
            if (theta > 359) continue;

            painter->drawLine(x + robot_xy[theta][0][0], y + robot_xy[theta][0][1],
                x + robot_xy[theta][1][0], y + robot_xy[theta][1][1]);
            painter->drawLine(x + robot_xy[theta][1][0], y + robot_xy[theta][1][1],
                x + robot_xy[theta][3][0], y + robot_xy[theta][3][1]);
            painter->drawLine(x + robot_xy[theta][3][0], y + robot_xy[theta][3][1],
                x + robot_xy[theta][2][0], y + robot_xy[theta][2][1]);
            painter->drawLine(x + robot_xy[theta][2][0], y + robot_xy[theta][2][1],
                x + robot_xy[theta][0][0], y + robot_xy[theta][0][1]);
            painter->drawLine(x + robot_xy[theta][4][0], y + robot_xy[theta][4][1],
                x + robot_xy[theta][5][0], y + robot_xy[theta][5][1]);
            painter->drawLine(x + robot_xy[theta][4][0], y + robot_xy[theta][4][1],
                x + robot_xy[theta][10][0], y + robot_xy[theta][10][1]);
            painter->drawLine(x + robot_xy[theta][4][0], y + robot_xy[theta][4][1],
                x + robot_xy[theta][11][0], y + robot_xy[theta][11][1]);

            if (x - 4 >= 0 && y - 5 >= 0)
            {
                painter->drawText(x - 4, y - 5, 12, 13, Qt::AlignLeft, QString::number(i + 1));
            }
        }
    }
}

// 绘制球：橙色实心圆，仅画当前位置不画轨迹（每帧重建背景自然擦除上一帧）

void DisplayDlg::DrawBall(QPainter* painter)
{
    if (ballInfor.found)
    {
        int x = (int)(ballInfor.x * 2.5) + 45;
        int y = (int)(ballInfor.y * 2.5) + 15;

        painter->setPen(QPen(QColor(255, 128, 0), 1));
        painter->setBrush(QBrush(QColor(255, 128, 0)));
        painter->drawEllipse(x - 4, y - 4, 8, 8);
    }
}

// 绘制己方机器人：黄色描边，逻辑同 DrawOpp

void DisplayDlg::DrawRobot(QPainter* painter)
{
    painter->setPen(QPen(Qt::yellow, 1));

    for (int i = 0; i < MAX_ROBOT_NUM; i++)
    {
        if (robotInfor[i].found)
        {
            int x = (int)(robotInfor[i].x * 2.5) + 45;
            int y = (int)(robotInfor[i].y * 2.5) + 15;
            int theta = (int)robotInfor[i].theta;
            if (theta < 0) theta += 360;
            if (theta > 359) continue;

            painter->drawLine(x + robot_xy[theta][0][0], y + robot_xy[theta][0][1],
                x + robot_xy[theta][1][0], y + robot_xy[theta][1][1]);
            painter->drawLine(x + robot_xy[theta][1][0], y + robot_xy[theta][1][1],
                x + robot_xy[theta][3][0], y + robot_xy[theta][3][1]);
            painter->drawLine(x + robot_xy[theta][3][0], y + robot_xy[theta][3][1],
                x + robot_xy[theta][2][0], y + robot_xy[theta][2][1]);
            painter->drawLine(x + robot_xy[theta][2][0], y + robot_xy[theta][2][1],
                x + robot_xy[theta][0][0], y + robot_xy[theta][0][1]);
            painter->drawLine(x + robot_xy[theta][4][0], y + robot_xy[theta][4][1],
                x + robot_xy[theta][5][0], y + robot_xy[theta][5][1]);
            painter->drawLine(x + robot_xy[theta][4][0], y + robot_xy[theta][4][1],
                x + robot_xy[theta][10][0], y + robot_xy[theta][10][1]);
            painter->drawLine(x + robot_xy[theta][4][0], y + robot_xy[theta][4][1],
                x + robot_xy[theta][11][0], y + robot_xy[theta][11][1]);

            if (x - 4 >= 0 && y - 5 >= 0)
            {
                painter->drawText(x - 4, y - 5, 12, 13, Qt::AlignLeft, QString::number(i + 1));
            }
        }
    }
}

// 四值取最小，供 MINS 调用
int DisplayDlg::GetMinValue(int val1, int val2, int val3, int val4)
{
    int minVal = val1;
    if (val2 < minVal) minVal = val2;
    if (val3 < minVal) minVal = val3;
    if (val4 < minVal) minVal = val4;
    return minVal;
}

int DisplayDlg::MINS(int R, int G, int B, int N)
{
    return GetMinValue(R, G, B, N);
}

// 像素编码为 15-bit 值，用于与已访问标记色 (100,100,100) 及场地背景区分
// 12684 = RGB(96,96,96) 的 15-bit 编码，代表场地背景
int DisplayDlg::screenBuffer(int m, int n, unsigned char* P)
{
    int index = (n * m_ImageSize.width() + m) * 3;
    int r = *(P + index + 0) / 8;  // R
    int g = *(P + index + 1) / 8;  // G
    int b = *(P + index + 2) / 8;  // B
    return ((r & 0x1f) << 10 | (g & 0x1f) << 5 | (b & 0x1f));
}

// 像素 HSI 是否落在对象阈值内：H 跨 0° 用 ||，否则用 &&

bool DisplayDlg::JudgePixel(int object, int H, int S, int I)
{
    ColorDlg* pColorDlg = m_pColorDlg ? m_pColorDlg : ColorDlg::getInstance();
    const int(*HSIThreshold)[6] = pColorDlg->getHSIThreshold();

    if (HSIThreshold[object][1] > HSIThreshold[object][0])
    {
        if (H >= HSIThreshold[object][0] && H <= HSIThreshold[object][1] &&
            S >= HSIThreshold[object][2] && S <= HSIThreshold[object][3] &&
            I >= HSIThreshold[object][4] && I <= HSIThreshold[object][5])
            return true;
        else
            return false;
    }
    else if (HSIThreshold[object][1] < HSIThreshold[object][0])
    {
        if ((H >= HSIThreshold[object][0] || H <= HSIThreshold[object][1]) &&
            S >= HSIThreshold[object][2] && S <= HSIThreshold[object][3] &&
            I >= HSIThreshold[object][4] && I <= HSIThreshold[object][5])
            return true;
        else
            return false;
    }
    return false;
}


// 统计 MEMB1/MEMB2/黑色像素数量判定颜色类型：1=紫 2=绿 0=黑 -1=不确定

int DisplayDlg::JudgeColor(int a, int b, int c)
{
    if (a >= 8 && b >= 8)
        return -1;
    if (a >= 8) return 1;      // MEMB1 为主
    if (b >= 8) return 2;      // MEMB2 为主
    if (c >= 8) return 0;      // 黑色为主
    return -1;                 // 像素数不足，无法判断
}

// 读取单像素 HSI 并判定是否匹配对象颜色

bool DisplayDlg::FindPixel(int object, int m, int n, unsigned char* P)
{
    int H, S, I;
    RGBToHS(m, n, P, H, S, I);
    return JudgePixel(object, H, S, I);
}

// 扫描线泛洪填充：从种子点连通同色像素，统计面积/边界框/质心
// 已访问像素置灰 (100,100,100) 防止重复匹配；球用更严格尺寸约束排除噪声

bool DisplayDlg::IdentifySearchLUT(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char* pStart, bool isBall)
{
    int sum = 0, sumx = 0, sumy = 0;
    int x, y, y1;
    bool spanLeft, spanRight;
    int m = m_ImageSize.width();
    int n = m_ImageSize.height();

    emptyStack();
    x = Startx;
    y = Starty;
    m_xLeft = m_xRight = x;
    m_yTop = m_yBottom = y;

    if (!push(x, y)) return false;

    while (pop(x, y))
    {
        // 向上扫描到色块顶部
        y1 = y;
        while (y1 >= 0) {
            if (!FindPixel(tab, x, y1, pStart))
                break;
            y1--;
        }
        y1++;  // 回退到第一个匹配像素

        spanLeft = false;
        spanRight = false;

        // 从顶部向下扫描整列，标记已访问并统计，左右邻居入栈扩展
        while (y1 < n)
        {
            if (!FindPixel(tab, x, y1, pStart))
                break;

            int idx = (y1 * m + x) * 3;
            pStart[idx]     = 100;  // R
            pStart[idx + 1] = 100;  // G
            pStart[idx + 2] = 100;  // B

            sum++;
            sumx += x;
            sumy += y1;

            if (x <= m_xLeft) m_xLeft = x;
            else if (x >= m_xRight) m_xRight = x;
            if (y1 <= m_yTop) m_yTop = y1;
            else if (y1 >= m_yBottom) m_yBottom = y1;

            if (!spanLeft && x > 0 && FindPixel(tab, x - 1, y1, pStart))
            {
                if (!push(x - 1, y1)) return false;
                spanLeft = true;
            }
            else if (spanLeft && x > 0 && !FindPixel(tab, x - 1, y1, pStart))
            {
                spanLeft = false;
            }

            if (!spanRight && x < m - 1 && FindPixel(tab, x + 1, y1, pStart))
            {
                if (!push(x + 1, y1)) return false;
                spanRight = true;
            }
            else if (spanRight && x < m - 1 && !FindPixel(tab, x + 1, y1, pStart))
            {
                spanRight = false;
            }

            y1++;
        }
    }

    m_lastBlobCount = sum;

    // 尺寸与边界框形状约束：球宽高 2~15，非球宽高 2~25
    if (sum >= SizeMin && sum <= SizeMax)
    {
        int bw = m_xRight - m_xLeft;
        int bh = m_yBottom - m_yTop;
        if (isBall) {
            if (bw < 2 || bw > 15 || bh < 2 || bh > 15)
                return false;
        } else {
            if (bw < 2 || bw > 25 || bh < 3 || bh > 25)
                return false;
        }

        m_Target[tab].setX(sumx / sum);
        m_Target[tab].setY(sumy / sum);
        return true;
    }
    return false;
}

// 单帧目标识别：复位结果后调用 IdentifyAll，再对球做位置滤波

void DisplayDlg::StartTest()
{
    for (int i = 0; i < MAX_ROBOT_NUM; i++) {
        robotInfor[i].found = false;
        OpprobotInfor[i].found = false;
        ObjectFound[i] = false;
    }
    ballInfor.found = false;
    ObjectFound[10] = false;
    ObjectFound[11] = false;

    m_pIdentify = m_pDispBitmap;

    IdentifyAll();
    BallPosFilter();      // 球位置滤波防抖（MFC 在 IdentifyAll 后调用）
}

// ═══════════════════════════════════════════════════════════════
// 一体化目标识别：步长 4 扫描，else if 链 OPP→TEAM→BALL
// 循环后统一转坐标 + 编号识别 + 球选最大候选
// ═══════════════════════════════════════════════════════════════
void DisplayDlg::IdentifyAll()
{
    int i, j;
    int xLeftTem, xRightTem, yTopTem, yBottomTem;
    int m = m_ImageSize.width();
    int n = m_ImageSize.height();
    int H, S, I;

    for (i = 0; i < MAX_ROBOT_NUM; i++)
        ObjectFound[i] = false;
    ObjectFound[10] = false;
    ObjectFound[11] = false;

    // 候选结果临时存储
    struct Candidate { int x, y, num; };
    Candidate TemBall[10];
    QPoint TeamTarget[20], OppTarget[20];
    double NormalTheta[20], OppNormalTheta[20];
    int NumOpp = 0, NumBall = 0, NumTeam = 0;

    // 复制帧缓冲：泛洪填充会改写像素，必须拷贝避免污染原图
    unsigned char* m_pTestBitmap = new unsigned char[m * n * 3];
    memcpy(m_pTestBitmap, m_pDispBitmap, m * n * 3);
    unsigned char* pTest = m_pTestBitmap;

    ColorDlg* pColorDlg = m_pColorDlg ? m_pColorDlg : ColorDlg::getInstance();
    const int(*HSIThreshold)[6] = pColorDlg->getHSIThreshold();

    // HSI 阈值匹配：H 跨 0° 用 ||，否则 &&；S/I 恒用 &&
    auto hsiMatch = [&](int idx) -> bool {
        int hLow = HSIThreshold[idx][0], hHigh = HSIThreshold[idx][1];
        int sLow = HSIThreshold[idx][2], sHigh = HSIThreshold[idx][3];
        int iLow = HSIThreshold[idx][4], iHigh = HSIThreshold[idx][5];
        bool hOK = (hLow <= hHigh) ? (H >= hLow && H <= hHigh)
                                   : (H >= hLow || H <= hHigh);
        return hOK && (S >= sLow && S <= sHigh) && (I >= iLow && I <= iHigh);
    };

    // 主扫描循环：OPP 优先于 TEAM 优先于 BALL
    for (j = 0; j < n; j += 4)
    {
        for (i = 0; i < m; i += 4)
        {
            RGBToHS(i, j, pTest, H, S, I);

            if (hsiMatch(QT_OPP))
            {
                xLeftTem = m_xLeft; xRightTem = m_xRight;
                yTopTem = m_yTop;   yBottomTem = m_yBottom;
                m_xLeft = m; m_xRight = 0; m_yTop = n; m_yBottom = 0;

                if (NumOpp < 20 && IdentifySearchLUT(QT_OPP, i, j, 30, 300, pTest))
                {
                    int cx = (m_xLeft + m_xRight) / 2;
                    int cy = (m_yTop + m_yBottom) / 2;
                    if (cx >= 0 && cx < DISPLAY_W && cy >= 0 && cy < DISPLAY_H) {
                        OppTarget[NumOpp] = QPoint(cx, cy);
                        int w = m_xRight - m_xLeft;
                        int h = m_yBottom - m_yTop;
                        OppNormalTheta[NumOpp] = (w >= h) ? 0.0 : M_PI / 2;
                        NumOpp++;
                    }
                }
                else { m_xLeft = xLeftTem; m_xRight = xRightTem; m_yTop = yTopTem; m_yBottom = yBottomTem; }
            }
            else if (hsiMatch(QT_TEAM))
            {
                xLeftTem = m_xLeft; xRightTem = m_xRight;
                yTopTem = m_yTop;   yBottomTem = m_yBottom;
                m_xLeft = m; m_xRight = 0; m_yTop = n; m_yBottom = 0;

                if (NumTeam < 20 && IdentifySearchLUT(QT_TEAM, i, j, 50, 300, pTest))
                {
                    int cx = (m_xLeft + m_xRight) / 2;
                    int cy = (m_yTop + m_yBottom) / 2;
                    TeamTarget[NumTeam] = QPoint(cx, cy);

                    // 边界框宽高比估算朝向：宽>高为水平，否则垂直
                    int w = m_xRight - m_xLeft;
                    int h = m_yBottom - m_yTop;
                    NormalTheta[NumTeam] = (w >= h) ? 0.0 : M_PI / 2;
                    NumTeam++;
                }
                else { m_xLeft = xLeftTem; m_xRight = xRightTem; m_yTop = yTopTem; m_yBottom = yBottomTem; }
            }
            else if (NumBall < 5 && hsiMatch(QT_BALL))
            {
                xLeftTem = m_xLeft; xRightTem = m_xRight;
                yTopTem = m_yTop;   yBottomTem = m_yBottom;
                m_xLeft = m; m_xRight = 0; m_yTop = n; m_yBottom = 0;

                if (IdentifySearchLUT(QT_BALL, i, j, 30, 300, pTest, true))
                {
                    int cx = (m_xLeft + m_xRight) / 2;
                    int cy = (m_yTop + m_yBottom) / 2;
                    qDebug() << "[Ball] candidate at (" << cx << "," << cy << ") pixels=" << m_lastBlobCount;
                    TemBall[NumBall].x = cx;
                    TemBall[NumBall].y = cy;
                    TemBall[NumBall].num = m_lastBlobCount;  // 用像素计数，对应 MFC m_TargetN.num = sum
                    NumBall++;
                }
                else { m_xLeft = xLeftTem; m_xRight = xRightTem; m_yTop = yTopTem; m_yBottom = yBottomTem; }
            }
        }
    }

    // 统一处理：己方/对方编号识别，球选面积最大的候选并转场地坐标
    IdentiRoboFromTargets(TeamTarget, NormalTheta, NumTeam, false);

    // 对方侧边标记与己方相同，复用同一编号逻辑
    IdentiRoboFromTargets(OppTarget, OppNormalTheta, NumOpp, true);

    qDebug() << "[Ball] NumBall=" << NumBall
             << "Threshold=[" << HSIThreshold[QT_BALL][0] << HSIThreshold[QT_BALL][1]
             << HSIThreshold[QT_BALL][2] << HSIThreshold[QT_BALL][3]
             << HSIThreshold[QT_BALL][4] << HSIThreshold[QT_BALL][5] << "]";
    if (NumBall >= 1) {
        int bestIdx = 0;
        for (int k = 1; k < NumBall; k++)
            if (TemBall[k].num > TemBall[bestIdx].num) bestIdx = k;
        int bx = TemBall[bestIdx].x, by = TemBall[bestIdx].y;
        qDebug() << "[Ball] best candidate: pixel=(" << bx << "," << by << ") area=" << TemBall[bestIdx].num;
        if (bx >= 0 && bx < DISPLAY_W && by >= 0 && by < DISPLAY_H) {
            ballInfor.x = ground.groundInfo[bx][by].x;
            ballInfor.y = ground.groundInfo[bx][by].y;
            qDebug() << "[Ball] field=(" << ballInfor.x << "," << ballInfor.y << ")";
        }
        ballInfor.found = true;
        ballInfor.theta = 0;
        ObjectFound[10] = true;
    }

    delete[] m_pTestBitmap;
}

// 球位置防抖：位移超过阈值时视为误检，回退到上一帧位置

void DisplayDlg::BallPosFilter()
{
    if (!ballInfor.found)
        return;

    if (ballBk.found)
    {
        double deltx = ballInfor.x - ballBk.x;
        double delty = ballInfor.y - ballBk.y;
        double delt = sqrt(deltx * deltx + delty * delty);
        if (delt > 15)
        {
            ballInfor.x = ballBk.x;
            ballInfor.y = ballBk.y;
        }
    }
    ballBk = ballInfor;
}

// 己方/对方机器人识别（独立扫描版本）
// 己方流程：泛洪找色块→边界框估角度→算4参考点→FindBlackID定朝向→FindRobotID定编号
// 对方只取位置不做编号

void DisplayDlg::IdentiRobo(int ObjectCount)
{
    int i, j;
    int m = m_ImageSize.width();
    int n = m_ImageSize.height();
    int H, S, I;
    int xLeftTem, xRightTem, yTopTem, yBottomTem;
    int robotNum = 0;

    QPoint TeamTarget[20];
    double NormalTheta[20];
    int NumTeam = 0;

    unsigned char* m_pTestBitmap = new unsigned char[m * n * 3];
    memcpy(m_pTestBitmap, m_pDispBitmap, m * n * 3);
    unsigned char* pTest = m_pTestBitmap;

    ColorDlg* pColorDlg = m_pColorDlg ? m_pColorDlg : ColorDlg::getInstance();
    const int(*HSIThreshold)[6] = pColorDlg->getHSIThreshold();

    // 第一遍：扫描全部像素找色块
    for (j = 0; j < n; j++)
    {
        for (i = 0; i < m; i++)
        {
            RGBToHS(i, j, pTest, H, S, I);

            if (H >= HSIThreshold[ObjectCount][0] && H <= HSIThreshold[ObjectCount][1] &&
                S >= HSIThreshold[ObjectCount][2] && S <= HSIThreshold[ObjectCount][3] &&
                I >= HSIThreshold[ObjectCount][4] && I <= HSIThreshold[ObjectCount][5])
            {
                xLeftTem = m_xLeft;
                xRightTem = m_xRight;
                yTopTem = m_yTop;
                yBottomTem = m_yBottom;
                m_xLeft = m;
                m_xRight = 0;
                m_yTop = n;
                m_yBottom = 0;

                if (IdentifySearchLUT(ObjectCount, i, j, 30, 300, pTest))
                {
                    int x = (m_xLeft + m_xRight) / 2;
                    int y = (m_yTop + m_yBottom) / 2;

                    if (ObjectCount == 4)
                    {
                        // 对手：仅记录位置，Y 翻转适配场地坐标系
                        if (robotNum < MAX_ROBOT_NUM && x >= 0 && x < DISPLAY_W && y >= 0 && y < DISPLAY_H)
                        {
                            int gy = DISPLAY_H - 1 - y;
                            OpprobotInfor[robotNum].x = ground.groundInfo[x][gy].x;
                            OpprobotInfor[robotNum].y = ground.groundInfo[x][gy].y;
                            OpprobotInfor[robotNum].found = true;
                            OpprobotInfor[robotNum].num = robotNum;
                            robotNum++;
                        }
                    }
                    else
                    {
                        // 己方：记录色块中心，用边界框宽高比估算法线角度
                        if (NumTeam < 20)
                        {
                            TeamTarget[NumTeam] = QPoint(x, y);

                            int w = m_xRight - m_xLeft;
                            int h = m_yBottom - m_yTop;
                            if (w > 3 && h > 3)
                            {
                                if (w >= h)
                                    NormalTheta[NumTeam] = 0.0;      // 水平
                                else
                                    NormalTheta[NumTeam] = M_PI / 2; // 垂直
                            }
                            else
                            {
                                NormalTheta[NumTeam] = 0.0;
                            }
                            NumTeam++;
                        }
                    }
                }
                else
                {
                    m_xLeft = xLeftTem;
                    m_xRight = xRightTem;
                    m_yTop = yTopTem;
                    m_yBottom = yBottomTem;
                }
            }
        }
    }

    // 第二遍：己方色块编号识别
    double m_Length = 7.5;
    m_pIdentify = m_pDispBitmap;  // FindBlackID 需要读取原始图像

    for (i = 0; i < NumTeam && robotNum < MAX_ROBOT_NUM; i++)
    {
        int RobotID = -1;
        double OrientAngle = 0;

        // 由法线角度算 4 个参考点位置
        double temptheta = M_PI / 2 - atan(0.75) - NormalTheta[i];
        QPoint ReferPoint[4];
        ReferPoint[0] = QPoint((int)(TeamTarget[i].x() + m_Length * cos(temptheta)),
                                (int)(TeamTarget[i].y() + m_Length * sin(temptheta)));
        ReferPoint[1] = QPoint((int)(TeamTarget[i].x() + m_Length * cos(temptheta + 2 * atan(0.75))),
                                (int)(TeamTarget[i].y() + m_Length * sin(temptheta + 2 * atan(0.75))));
        ReferPoint[2] = QPoint((int)(TeamTarget[i].x() + m_Length * cos(temptheta + M_PI)),
                                (int)(TeamTarget[i].y() + m_Length * sin(temptheta + M_PI)));
        ReferPoint[3] = QPoint((int)(TeamTarget[i].x() + m_Length * cos(temptheta + 2 * atan(0.75) + M_PI)),
                                (int)(TeamTarget[i].y() + m_Length * sin(temptheta + 2 * atan(0.75) + M_PI)));

        // 检测 4 个参考点是否为黑色
        bool blackID[4] = {false, false, false, false};
        for (j = 0; j < 4; j++)
            blackID[j] = FindBlackID(ReferPoint[j].x(), ReferPoint[j].y(), j);

        // 黑色参考点组合决定朝向和待识别的编号参考点
        if (blackID[0] && blackID[1])
        {
            OrientAngle = NormalTheta[i] + M_PI;
            RobotID = FindRobotID(ReferPoint[2], ReferPoint[3]);
        }
        else if (blackID[2] && blackID[3])
        {
            OrientAngle = NormalTheta[i];
            RobotID = FindRobotID(ReferPoint[0], ReferPoint[1]);
        }
        else if (!blackID[0] && !blackID[1])
        {
            OrientAngle = NormalTheta[i];
            RobotID = FindRobotIDD(ReferPoint[2], ReferPoint[3]);
        }
        else if (!blackID[2] && !blackID[3])
        {
            OrientAngle = NormalTheta[i] + M_PI;
            RobotID = FindRobotIDD(ReferPoint[0], ReferPoint[1]);
        }

        if (RobotID >= 0 && RobotID < MAX_ROBOT_NUM)
        {
            int px = TeamTarget[i].x();
            int py = TeamTarget[i].y();
            if (px >= 0 && px < DISPLAY_W && py >= 0 && py < DISPLAY_H)
            {
                // Y 翻转：像素 top-down → 场地 bottom-up
                int gpy = DISPLAY_H - 1 - py;
                robotInfor[RobotID].theta = OrientAngle * 180.0 / M_PI;
                robotInfor[RobotID].x = ground.groundInfo[px][gpy].x;
                robotInfor[RobotID].y = ground.groundInfo[px][gpy].y;
                robotInfor[RobotID].found = true;
                robotInfor[RobotID].num = RobotID;
                ObjectFound[RobotID] = true;
            }
        }
    }

    delete[] m_pTestBitmap;
}

// ═══════════════════════════════════════════════════════════════
// 编号识别：对已聚类的色块用参考点黑色/颜色组合定朝向与编号
// 己方与对方共用同一编号逻辑（侧边标记一致）
// ═══════════════════════════════════════════════════════════════
void DisplayDlg::IdentiRoboFromTargets(QPoint targets[], double normalTheta[], int count, bool isOpponent)
{
    double m_Length = 7.5;
    m_pIdentify = m_pDispBitmap;  // FindBlackID 需要读取原始图像

    // isOpponent 决定结果写入己方还是对方数组
    RobotInford* resultArr = isOpponent ? OpprobotInfor : robotInfor;

    for (int i = 0; i < count; i++)
    {
        int RobotID = -1;
        double OrientAngle = 0;

        double temptheta = M_PI / 2 - atan(0.75) - normalTheta[i];
        QPoint ReferPoint[4];
        ReferPoint[0] = QPoint((int)(targets[i].x() + m_Length * cos(temptheta)),
                                (int)(targets[i].y() + m_Length * sin(temptheta)));
        ReferPoint[1] = QPoint((int)(targets[i].x() + m_Length * cos(temptheta + 2 * atan(0.75))),
                                (int)(targets[i].y() + m_Length * sin(temptheta + 2 * atan(0.75))));
        ReferPoint[2] = QPoint((int)(targets[i].x() + m_Length * cos(temptheta + M_PI)),
                                (int)(targets[i].y() + m_Length * sin(temptheta + M_PI)));
        ReferPoint[3] = QPoint((int)(targets[i].x() + m_Length * cos(temptheta + 2 * atan(0.75) + M_PI)),
                                (int)(targets[i].y() + m_Length * sin(temptheta + 2 * atan(0.75) + M_PI)));

        bool blackID[4] = {false, false, false, false};
        for (int j = 0; j < 4; j++)
            blackID[j] = FindBlackID(ReferPoint[j].x(), ReferPoint[j].y(), j);

        if (blackID[0] && blackID[1])
        {
            OrientAngle = normalTheta[i] + M_PI;
            RobotID = FindRobotID(ReferPoint[2], ReferPoint[3]);
        }
        else if (blackID[2] && blackID[3])
        {
            OrientAngle = normalTheta[i];
            RobotID = FindRobotID(ReferPoint[0], ReferPoint[1]);
        }
        else if (!blackID[0] && !blackID[1])
        {
            OrientAngle = normalTheta[i];
            RobotID = FindRobotIDD(ReferPoint[2], ReferPoint[3]);
        }
        else if (!blackID[2] && !blackID[3])
        {
            OrientAngle = normalTheta[i] + M_PI;
            RobotID = FindRobotIDD(ReferPoint[0], ReferPoint[1]);
        }

        if (RobotID >= 0 && RobotID < MAX_ROBOT_NUM)
        {
            int px = targets[i].x();
            int py = targets[i].y();
            if (px >= 0 && px < DISPLAY_W && py >= 0 && py < DISPLAY_H)
            {
                resultArr[RobotID].theta = OrientAngle * 180.0 / M_PI;
                resultArr[RobotID].x = ground.groundInfo[px][py].x;
                resultArr[RobotID].y = ground.groundInfo[px][py].y;
                resultArr[RobotID].found = true;
                resultArr[RobotID].num = RobotID;
                if (!isOpponent) ObjectFound[RobotID] = true;
            }
        }
    }
}

// 在两个参考点附近各取 5x5 区域统计 MEMB1/MEMB2/黑色像素，按颜色组合定编号
// 编号对照: (RPID1,RPID2) 0/1→0号, 1/0→1号, 1/1→2号, 0/2→3号, 2/0→4号,
//           2/2→5号, 2/1→6号, 1/2→7号；RPID: 0=黑 1=紫 2=绿
int DisplayDlg::FindRobotID(QPoint RP1, QPoint RP2)
{
    int roboID, RPID1, RPID2;
    int ii, jj;
    int sum1, sum2, sum0;
    int H = 0, S = 0, I = 0;

    sum1 = sum2 = sum0 = 0;
    for (jj = RP1.y() - 2; jj <= RP1.y() + 2; jj++)
        for (ii = RP1.x() - 2; ii <= RP1.x() + 2; ii++)
        {
            if (jj < 0 || jj >= DISPLAY_H || ii < 0 || ii >= DISPLAY_W) continue;
            RGBToHS(ii, jj, m_pIdentify, H, S, I);
            if (JudgePixel(1, H, S, I))        // MEMB1
                sum1++;
            else if (JudgePixel(2, H, S, I))   // MEMB2
                sum2++;
            else if (screenBuffer(ii, jj, m_pIdentify) != 12684)  // 非场地背景
                sum0++;
        }
    RPID1 = JudgeColor(sum1, sum2, sum0);

    sum1 = sum2 = sum0 = 0;
    for (jj = RP2.y() - 2; jj <= RP2.y() + 2; jj++)
        for (ii = RP2.x() - 2; ii <= RP2.x() + 2; ii++)
        {
            if (jj < 0 || jj >= DISPLAY_H || ii < 0 || ii >= DISPLAY_W) continue;
            RGBToHS(ii, jj, m_pIdentify, H, S, I);
            if (JudgePixel(1, H, S, I))
                sum1++;
            else if (JudgePixel(2, H, S, I))
                sum2++;
            else if (screenBuffer(ii, jj, m_pIdentify) != 12684)
                sum0++;
        }
    RPID2 = JudgeColor(sum1, sum2, sum0);

    if (RPID1 == 0 && RPID2 == 1) roboID = 0;
    else if (RPID1 == 1 && RPID2 == 0) roboID = 1;
    else if (RPID1 == 1 && RPID2 == 1) roboID = 2;
    else if (RPID1 == 0 && RPID2 == 2) roboID = 3;
    else if (RPID1 == 2 && RPID2 == 0) roboID = 4;
    else if (RPID1 == 2 && RPID2 == 2) roboID = 5;
    else if (RPID1 == 2 && RPID2 == 1) roboID = 6;
    else if (RPID1 == 1 && RPID2 == 2) roboID = 7;
    else roboID = -1;

    return roboID;
}

// FindRobotID 的别名：黑色参考点检测不确定时的后备入口，逻辑相同
int DisplayDlg::FindRobotIDD(QPoint RP1, QPoint RP2)
{
    return FindRobotID(RP1, RP2);
}

// 检测参考点 3x3 区域黑色像素数：>=5 判黑，否则记录计数供后续判断

bool DisplayDlg::FindBlackID(int m, int n, int Num)
{
    int ii, jj;
    int H = 0, S = 0, I = 0;
    int sumblack = 0;

    for (jj = n - 1; jj <= n + 1; jj++)
        for (ii = m - 1; ii <= m + 1; ii++)
        {
            if (jj < 0 || jj >= DISPLAY_H || ii < 0 || ii >= DISPLAY_W) continue;
            RGBToHS(ii, jj, m_pIdentify, H, S, I);
            // 非 MEMB1、非 MEMB2、非场地背景即视为黑色
            if (!JudgePixel(1, H, S, I)
                && !JudgePixel(2, H, S, I)
                && (screenBuffer(ii, jj, m_pIdentify) != 12684))
            {
                sumblack++;
                if (sumblack >= 5) break;
            }
        }
    if (sumblack >= 5) {
        return true;
    } else {
        BlackSum[Num] = sumblack;
        return false;
    }
}

// 四邻接泛洪填充搜索色块（旧版接口，按面积区间判定有效性）

bool DisplayDlg::SeachOppAndBall(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char* pStart)
{
    int x, y;
    int startindex;
    int count = 0;
    int m = m_ImageSize.width();
    int n = m_ImageSize.height();
    emptyStack();
    startindex = Starty * m + Startx;
    if (pStart[startindex] != 0)
        return false;
    push(Startx, Starty);
    while (pop(x, y))
    {
        startindex = y * m + x;
        pStart[startindex] = 255;
        count++;

        if (x > 0 && pStart[startindex - 1] == 0 && FindPixel(tab, x - 1, y, pStart))
        {
            push(x - 1, y);
            pStart[startindex - 1] = 254;
        }
        if (x < m - 1 && pStart[startindex + 1] == 0 && FindPixel(tab, x + 1, y, pStart))
        {
            push(x + 1, y);
            pStart[startindex + 1] = 254;
        }
        if (y > 0 && pStart[startindex - m] == 0 && FindPixel(tab, x, y - 1, pStart))
        {
            push(x, y - 1);
            pStart[startindex - m] = 254;
        }
        if (y < n - 1 && pStart[startindex + m] == 0 && FindPixel(tab, x, y + 1, pStart))
        {
            push(x, y + 1);
            pStart[startindex + m] = 254;
        }
    }
    if (count < SizeMin || count > SizeMax)
    {
        return false;
    }
    else
    {
        return true;
    }
}

// 四邻接泛洪填充搜索色块（旧版接口，按面积区间判定有效性）

bool DisplayDlg::SearchTeam(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char* pStart)
{
    int x, y;
    int startindex;
    int count = 0;
    int m = m_ImageSize.width();
    int n = m_ImageSize.height();
    emptyStack();
    startindex = Starty * m + Startx;
    if (pStart[startindex] != 0)
        return false;
    push(Startx, Starty);
    while (pop(x, y))
    {
        startindex = y * m + x;
        pStart[startindex] = 255;
        count++;

        if (x > 0 && pStart[startindex - 1] == 0 && FindPixel(tab, x - 1, y, pStart))
        {
            push(x - 1, y);
            pStart[startindex - 1] = 254;
        }
        if (x < m - 1 && pStart[startindex + 1] == 0 && FindPixel(tab, x + 1, y, pStart))
        {
            push(x + 1, y);
            pStart[startindex + 1] = 254;
        }
        if (y > 0 && pStart[startindex - m] == 0 && FindPixel(tab, x, y - 1, pStart))
        {
            push(x, y - 1);
            pStart[startindex - m] = 254;
        }
        if (y < n - 1 && pStart[startindex + m] == 0 && FindPixel(tab, x, y + 1, pStart))
        {
            push(x, y + 1);
            pStart[startindex + m] = 254;
        }
    }
    if (count < SizeMin || count > SizeMax)
    {
        return false;
    }
    else
    {
        return true;
    }
}

// 图像分割测试：黑底上用绿色边框 + 红色十字标记检测到的色块，直接写 displayLabel

void DisplayDlg::IdentifyTest()
{
    ColorDlg* pColorDlg = m_pColorDlg ? m_pColorDlg : ColorDlg::getInstance();
    int object = pColorDlg->currentObject();
    qDebug() << "[IdentifyTest] object=" << object
             << "H=[" << pColorDlg->getHSIThreshold()[object][0]
             << "," << pColorDlg->getHSIThreshold()[object][1] << "]";
    if (object < 0 || object >= 8) return;

    QImage segImage(DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
    segImage.fill(Qt::black);

    // 拷贝缓冲：泛洪填充会改写像素，不能污染原图
    int bufSize = DISPLAY_W * DISPLAY_H * 3;
    unsigned char* pTestBitmap = new unsigned char[bufSize];
    memcpy(pTestBitmap, m_pDispBitmap, bufSize);

    QPainter painter(&segImage);
    painter.setRenderHint(QPainter::Antialiasing, false);

    QPen greenPen(QColor(0, 255, 0), 2);
    QPen redPen(QColor(255, 0, 0), 1);

    int Num = 0;

    // 步长 4 扫描，命中颜色即泛洪找色块（面积 30~300，最多 20 个）
    for (int i = 0; i < DISPLAY_W; i += 4) {
        for (int j = 0; j < DISPLAY_H; j += 4) {
            if (FindPixel(object, i, j, pTestBitmap)) {
                if (Num < 20 && IdentifySearchLUT(object, i, j, 30, 300, pTestBitmap)) {
                    if (m_Target[object].x() < 0 || m_Target[object].x() >= DISPLAY_W ||
                        m_Target[object].y() < 0 || m_Target[object].y() >= DISPLAY_H)
                        continue;

                    painter.setPen(greenPen);
                    painter.setBrush(Qt::NoBrush);
                    painter.drawRect(m_xLeft, m_yTop,
                                    m_xRight - m_xLeft, m_yBottom - m_yTop);

                    painter.setPen(redPen);
                    painter.drawLine(m_xLeft, m_Target[object].y(),
                                    m_xRight, m_Target[object].y());
                    painter.drawLine(m_Target[object].x(), m_yTop,
                                    m_Target[object].x(), m_yBottom);
                    Num++;
                }
            }
        }
    }

    painter.end();
    delete[] pTestBitmap;

    displayLabel->setPixmap(QPixmap::fromImage(segImage));
}

// 比赛态入口：实际识别与绘制由 onTimer→ProcessImage→paintEvent 驱动

void DisplayDlg::StartGame()
{
}

// 统计矩形区域内符合 HSI 阈值的像素点集

void DisplayDlg::ColorAnalyse(const QRect& rect, int yi[], std::vector<QPoint>& vecColorSet)
{
    vecColorSet.clear();
    int x1 = rect.left();
    int y1 = rect.top();
    int x2 = rect.right();
    int y2 = rect.bottom();
    int i, j;
    int H, S, I;
    int count = 0;
    for (j = y1; j <= y2; j++)
    {
        for (i = x1; i <= x2; i++)
        {
            RGBToHS(i, j, m_pDispSingle, H, S, I);
            if (H >= yi[0] && H <= yi[1] && S >= yi[2] && S <= yi[3] && I >= yi[4] && I <= yi[5])
            {
                vecColorSet.push_back(QPoint(i, j));
                count++;
            }
        }
    }
}

// 统计点集合中符合 HSI 阈值的像素

void DisplayDlg::ColorAnalyse(const std::vector<QPoint>& pts, int yi[], std::vector<QPoint>& vecColorSet)
{
    vecColorSet.clear();
    int H, S, I;
    for (int i = 0; i < pts.size(); i++)
    {
        int x = pts[i].x();
        int y = pts[i].y();
        RGBToHS(x, y, m_pDispSingle, H, S, I);
        if (H >= yi[0] && H <= yi[1] && S >= yi[2] && S <= yi[3] && I >= yi[4] && I <= yi[5])
        {
            vecColorSet.push_back(QPoint(x, y));
        }
    }
}