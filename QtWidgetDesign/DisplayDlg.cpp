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

// 自定义MIN函数，计算三个值中的最小值
inline int MIN(int a, int b, int c, int n)
{
    int min_val = a;
    if (b < min_val) min_val = b;
    if (c < min_val) min_val = c;
    return min_val;
}

// RGB -> H 转换表
int HLUT[256][256][256];    //RGB-H 转换表，S,I值分别用公式计算

// 预分配缓冲区，避免频繁内存分配
static unsigned char* pBuffer = nullptr;

// 机器人形状坐标
int robot_xy[361][12][2];             //机器人方向图像关键点坐标

// ========================================================================
// OverlayWidget 实现 - 透明覆盖层控件
// ========================================================================
// 设计说明：
//   此控件叠在 displayLabel（摄像头画面）上方，用于采色模式下
//   绘制鼠标框选的红框矩形。采用透明背景 + XOR 组合模式，
//   实现与 MFC 的 CDC::SetROP2(R2_NOTXORPEN) 等价的视觉效果。
//
//   与 MFC 的对应关系：
//     MFC: CDisplayDlg::OnLButtonDown / OnMouseMove 中
//          pDC = m_display.GetDC(); pDC->SetROP2(R2_NOTXORPEN);
//     Qt:  OverlayWidget::mousePressEvent / mouseMoveEvent 中
//          QPainter + RasterOp_SourceXorDestination
// ========================================================================

// 构造函数 - 初始化覆盖层控件
// 参数 parent: 父控件（DisplayDlg），覆盖层将叠在 displayLabel 上方
OverlayWidget::OverlayWidget(QWidget* parent)
    : QWidget(parent)
    , m_hasSelection(false)
{
    // 设置透明背景，使覆盖层不会遮挡底层的摄像头画面
    setStyleSheet("background: transparent;");
    // 确保覆盖层可以接收鼠标事件（默认 true，显式设置以防万一）
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    // 启用鼠标追踪，mouseMoveEvent 在按下状态下才会触发
    // （实际拖拽时由 mousePressEvent 中的按钮状态控制）
}

// 设置框选矩形并触发重绘
// 参数 r: 矩形区域（覆盖层控件坐标系，与 displayLabel 的图像坐标对齐）
// 说明：外部调用此方法可以直接设置矩形，当前由 mousePressEvent/mouseMoveEvent 内部使用
void OverlayWidget::setSelectionRect(const QRect& r)
{
    m_selectionRect = r;
    m_hasSelection = true;
    update();  // 触发 paintEvent 重绘
}

// 清除框选矩形并触发重绘
// 调用时机：切换标签页或重新开始框选时
void OverlayWidget::clearSelectionRect()
{
    m_hasSelection = false;
    m_selectionRect = QRect();
    update();  // 触发 paintEvent 重绘（paintEvent 中 hasSelection=false 会跳过绘制）
}

// 重绘事件 - 用 XOR 模式绘制红色矩形线框
// 触发时机：每次调用 update() 或窗口系统要求重绘时自动调用
// 绘制原理：
//   使用 RasterOp_SourceXorDestination 组合模式，红色线条与背景像素
//   进行异或运算，产生高对比度的可见框线（类似 MFC 的 R2_NOTXORPEN）。
//   由于覆盖层背景是透明的，XOR 操作直接作用在底层摄像头画面的像素上，
//   视觉效果与 MFC 完全一致。
void OverlayWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    // 没有有效框选时不需要绘制
    if (!m_hasSelection)
        return;

    QPainter painter(this);
    // 使用 SourceOver 模式直接绘制深红色线框，颜色不随背景变化
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setPen(QPen(QColor(200, 0, 0), 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_selectionRect);
}

// 鼠标按下事件 - 记录框选起点
// 触发时机：用户在覆盖层上按下鼠标左键
// 对应 MFC：CDisplayDlg::OnLButtonDown 中 COLOR_SET 分支
//   m_Rect.left = pt.x; m_Rect.top = pt.y;
void OverlayWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // 记录框选起点，初始化一个 1×1 的矩形
        // 与 MFC 的 m_Rect.right = pt.x + 1; m_Rect.bottom = pt.y + 1 一致
        m_selectionRect = QRect(event->pos(), QSize(1, 1));
        m_hasSelection = true;
        update();  // 触发 paintEvent，画出初始的 1×1 矩形（实际不可见）
    }
}

// 鼠标拖动事件 - 更新矩形终点并重绘
// 触发时机：用户按住左键拖动鼠标时
// 对应 MFC：CDisplayDlg::OnMouseMove 中 COLOR_SET 分支
//
// MFC 的做法（两步）：
//   1. pDC->SetROP2(R2_NOTXORPEN);
//      画旧矩形 → XOR 擦除（与背景异或还原）
//   2. 更新 m_Rect.right/bottom
//      画新矩形 → XOR 显示
//
// Qt 的做法（一步）：
//   直接更新 m_selectionRect 的终点，然后调用 update()。
//   paintEvent 每次从干净的覆盖层状态重画一个 XOR 矩形，
//   不需要手动"擦旧"，视觉效果等同于 MFC 的两步操作。
//   这是因为 Qt 的 update() 会先清除 widget 内容再调用 paintEvent，
//   而 MFC 的 GDI 绘制是持久的，需要手动 XOR 擦除。
void OverlayWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        // 更新矩形终点（normalized 确保 left<right, top<bottom，
        // 即使用户从右下往左上拖也能正确显示）
        m_selectionRect.setBottomRight(event->pos());
        m_selectionRect = m_selectionRect.normalized();
        update();  // 触发 paintEvent 重绘
    }
}

//功能：初始化显示区域，设置图像数据、机器人形状坐标和颜色转换表

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
    , m_PatchAngle(0.0)
    , m_overlayWidget(nullptr)
{
    // 初始化图像数据
    m_pDispBitmap = new unsigned char[m_ImageSize.width() * m_ImageSize.height() * 3]();
    m_pDispSingle = new unsigned char[m_ImageSize.width() * m_ImageSize.height() * 3]();
    m_pIdentify = m_pDispBitmap;

    // 初始化机器人形状坐标
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

    // 初始化RGB-H转换表，从文件加载
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
        // 如果文件加载失败，初始化为0
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

    // 初始化机器人信息
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

    // 预加载车号图像，避免切换标签页时从磁盘加载延迟
    m_carNumPixmap = QPixmap("resources/carnum.bmp");

    initUI();
}

// 功能：释放图像数据和定时器资源

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
    // 释放预分配的缓冲区
    if (pBuffer)
        delete[] pBuffer;
}

// 功能：创建显示区域、帧率标签和定时器

void DisplayDlg::initUI()
{
    // 设置字体
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);

    // 创建主布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 帧率显示标签 - 在显示区域上方，与右侧标签栏高度一致
    fpsLabel = new QLabel(this);
    fpsLabel->setStyleSheet("QLabel { background-color: transparent; color: black; font-size: 12px; padding: 2px; font-weight: bold; }");
    fpsLabel->setText("FPS: 0");
    fpsLabel->setFixedSize(DISPLAY_W, 32); // 与QTabWidget标签栏高度一致
    fpsLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // 显示区域
    displayLabel = new QLabel(this);
    displayLabel->setStyleSheet("QLabel { background-color: #333333; border: 1px solid black; color: white; }");
    displayLabel->setFixedSize(DISPLAY_W, DISPLAY_H);
    displayLabel->setAlignment(Qt::AlignCenter);
    displayLabel->setText("显示区域");

    mainLayout->addWidget(fpsLabel);
    mainLayout->addWidget(displayLabel);

    // ── 创建采色模式的透明覆盖层 ─────────────────────────────────────────
    // 覆盖层作为 displayLabel 的子控件，与 displayLabel 完全重叠。
    // 坐标系说明：
    //   displayLabel 的坐标范围为 (0,0) ~ (DISPLAY_W-1, DISPLAY_H-1)
    //   由于 overlay 是 displayLabel 的子控件，overlay 的本地坐标
    //   直接对应 displayLabel 中的图像坐标，无需额外的坐标转换。
    //   这与 MFC 中 m_display.ScreenToClient(&pt) 的效果一致。
    //
    // 对应 MFC 代码：
    //   CDisplayDlg::OnLButtonDown / OnMouseMove 中
    //   pDC = m_display.GetDC() → 直接在 m_display 上画红框
    m_overlayWidget = new OverlayWidget(displayLabel);
    m_overlayWidget->setFixedSize(DISPLAY_W, DISPLAY_H);
    m_overlayWidget->move(0, 0);  // 与 displayLabel 左上角对齐
    m_overlayWidget->hide();       // 初始隐藏，仅在 COLOR_SET 模式下显示

    // 初始化定时器
    m_grabTimer = new QTimer(this);
    connect(m_grabTimer, &QTimer::timeout, this, &DisplayDlg::onTimer);

    // 初始化帧率更新定时器
    fpsTimer = new QTimer(this);
    connect(fpsTimer, &QTimer::timeout, this, &DisplayDlg::updateFPS);
    fpsTimer->start(500); // 每500ms更新一次，与MFC版本保持一致

    // Load field image
    m_groundImage.load("resources/ground.bmp");
    if (m_groundImage.isNull()) {
        m_groundImage = QImage(DISPLAY_W, DISPLAY_H, QImage::Format_RGB32);
        m_groundImage.fill(QColor(0, 128, 0));
    }

    // Show a grey placeholder so the display area is never blank on startup.
    // Once the camera opens (ShowDynamic / ShowSingle) this will be replaced.
    QPixmap placeholder(DISPLAY_W, DISPLAY_H);
    placeholder.fill(QColor(80, 80, 80));
    QPainter ph(&placeholder);
    ph.setPen(Qt::white);
    ph.setFont(QFont("Arial", 14));
    ph.drawText(placeholder.rect(), Qt::AlignCenter, "Camera not started");
    displayLabel->setPixmap(placeholder);
}

// Grab one frame, display it, and overlay any calibration point markers.
void DisplayDlg::ShowSingle()
{
    GrabSingle();

    // Build display pixmap from the single-grab buffer
    QImage image(m_pDispSingle, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image);

    // Overlay calibration point markers (red cross, label)
    if (!m_calibPoints.empty()) {
        QPainter p(&pixmap);
        p.setPen(QPen(Qt::red, 2));
        QFont f;
        f.setPointSize(8);
        p.setFont(f);
        for (int i = 0; i < (int)m_calibPoints.size(); ++i) {
            const QPoint& pt = m_calibPoints[i];
            // Cross arms ±6 px
            p.drawLine(pt.x() - 6, pt.y(), pt.x() + 6, pt.y());
            p.drawLine(pt.x(), pt.y() - 6, pt.x(), pt.y() + 6);
            // Index label (1-based)
            p.drawText(pt.x() + 4, pt.y() - 4, QString::number(i + 1));
        }
    }

    displayLabel->setPixmap(pixmap);
}

//功能：启动定时器，持续从摄像头获取图像并显示

void DisplayDlg::ShowDynamic()
{
    // 确保停止之前的状态
    Stop();

    // 确保摄像头已打开
    Camera* pCamera = Camera::GetInstance();
    if (!pCamera->IsOpen()) {
        if (!pCamera->Open()) {
            // 如果摄像头打开失败，显示错误信息
            displayLabel->setText("无法打开摄像头");
            return;
        }
    }

    // 确保摄像头处于抓取状态
    if (!pCamera->IsGrabbing()) {
        pCamera->StartGrabbing();
    }

    // 设置状态为显示模式
    m_status = STATUS::Display;

    // 开始计时
    m_DisplayWatch.start();

    // 启动抓取线程
    m_grabTimer->start(50); // 与MFC版本保持一致

    // 立即获取并显示一帧图像，避免切换时出现黑屏或显示旧图像
    unsigned char* tempBuffer = new unsigned char[DISPLAY_W * DISPLAY_H * 3];
    if (pCamera->RetrieveResult(tempBuffer)) {
        pCamera->ConvertBitmap(m_pDispBitmap, tempBuffer, DISPLAY_W, DISPLAY_H);
        QImage image(m_pDispBitmap, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(image);
        displayLabel->setPixmap(pixmap);
    }
    delete[] tempBuffer;
}

/**
 * @brief 显示车号
 * 功能：加载并显示车号图像
 */
void DisplayDlg::ShowCarNum()
{
    Camera* pCamera = Camera::GetInstance();
    if (pCamera->IsGrabbing()) {
        this->Stop();
    }
    // 使用预加载的车号图像，避免从磁盘加载延迟
    if (!m_carNumPixmap.isNull()) {
        displayLabel->setPixmap(m_carNumPixmap);
    }
    else {
        // 如果图像加载失败，显示默认文本
        displayLabel->setText("车号显示");
    }
}

// 功能：根据颜色阈值显示符合条件的图像区域

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
                // RGB 格式: byte 0=R, 1=G, 2=B（与 Pylon RGB8packed 一致）
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
                // RGB 格式: byte 0=R, 1=G, 2=B（与 Pylon RGB8packed 一致）
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

// 功能：启动测试模式，定时获取并处理图像

void DisplayDlg::ShowRunTest(bool ImageSeg)
{
    // 确保停止之前的状态
    Stop();

    // 确保摄像头已打开并抓取
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

    // 清空历史数据
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
    m_grabTimer->start(33);
}

//功能：设置准备状态，初始化游戏并识别所有目标
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

    // 清除足球轨迹
    ClearBallTrail();

    m_pIdentify = m_pDispSingle;
    IdentifyAll();
    m_status = STATUS::Prepare;

    // 启动定时器，持续更新画面
    if (!m_grabTimer->isActive()) {
        m_grabTimer->start(33); // 约30fps
    }
    m_DisplayWatch.start();
    this->repaint(); // 使用repaint立即重绘
}

// 功能：设置游戏状态，启动游戏逻辑

void DisplayDlg::ShowStartGame()
{
    m_status = STATUS::Game;
    StartGame();
}

//功能：停止定时器，设置停止状态

void DisplayDlg::Stop()
{
    if (!m_bErrorSign) {
        // 停止定时器
        m_grabTimer->stop();

        // 停止摄像头抓取
        Camera* pCamera = Camera::GetInstance();
        if (pCamera->IsGrabbing()) {
            pCamera->StopGrabbing();
        }

        // 设置状态为停止
        m_status = STATUS::Stop;
    }
}

//功能：设置当前操作状态

// 功能：设置当前操作状态，并控制覆盖层的显示/隐藏
// 对应 MFC：CDisplayDlg::SelectSetStatus(SET_STATUS s) { m_setStatus = s; }
//
// MFC 版本只需设置状态标志，因为 GDI 绘制是在同一个 DC 上直接操作。
// Qt 版本需要额外管理 OverlayWidget 的可见性：
//   - COLOR_SET：显示覆盖层，使用户可以在上面框选颜色区域
//   - 其他状态：隐藏覆盖层，避免干扰其他模式的鼠标交互

void DisplayDlg::SelectSetStatus(SET_STATUS s)
{
    m_setStatus = s;

    // 控制覆盖层可见性
    if (m_overlayWidget) {
        if (s == SET_STATUS::COLOR_SET) {
            // 进入采色框选模式：显示覆盖层，清除之前的框选
            m_overlayWidget->clearSelectionRect();
            m_overlayWidget->show();
            // 提升覆盖层到最前面，确保能接收鼠标事件
            m_overlayWidget->raise();
        }
        else {
            // 离开采色框选模式：隐藏覆盖层
            m_overlayWidget->hide();
        }
    }
}

// 功能：获取当前框选矩形（图像坐标）
// 供 ColorDlg::onZoom() 和 ColorDlg::onSample() 调用，读取用户框选的颜色区域。
//
// 坐标系说明：
//   OverlayWidget 是 displayLabel 的子控件，其本地坐标直接对应图像坐标。
//   因此 overlay 的 selectionRect 无需任何坐标转换即可作为图像坐标使用。
//   （对比旧版本：需要手动减去 fpsLabel 的高度 32px 来转换坐标）
//
// 对应 MFC：CDisplayDlg::GetRect() const { return m_Rect; }

QRect DisplayDlg::GetRect() const
{
    if (m_overlayWidget && m_overlayWidget->hasSelection()) {
        // 从覆盖层获取框选矩形（已是图像坐标，无需转换）
        return m_overlayWidget->getSelectionRect();
    }
    // 后备：返回 DisplayDlg 自身的 m_Rect（BORDER_SET 等模式使用）
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

// 功能：绘制足球场背景和机器人

void DisplayDlg::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    // 足球场背景 + 识别叠加层：适用于比赛、预备、非分割动态测试状态
    // 注意：RunTestSeg（分割模式）由 IdentifyTest() 直接管理 displayLabel 显示，
    // 不走 paintEvent，避免场地背景覆盖黑色分割图
    if (m_status == STATUS::Game || m_status == STATUS::Prepare ||
        m_status == STATUS::RunTest) {
        QPixmap pixmap(DISPLAY_W, DISPLAY_H);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);

        // 绘制足球场背景
        painter.drawImage(0, 0, m_groundImage);

        // 绘制机器人
        DrawRobot(&painter);

        // 绘制对手
        DrawOpp(&painter);

        // 绘制球
        DrawBall(&painter);

        // 设置绘制好的pixmap
        displayLabel->setPixmap(pixmap);
    }
    // 在其他状态下（如Display、Stop等），不进行任何绘制操作，避免干扰摄像头图像显示
}

// 功能：清除足球轨迹

void DisplayDlg::ClearBallTrail()
{
    m_ballTrail.clear();
}

// 功能：处理鼠标拖拽操作
// 注意：COLOR_SET 框选逻辑已移至 OverlayWidget::mouseMoveEvent。
// 此处保留空函数，将来如 BORDER_SET 模式需要拖拽交互可在此扩展。

void DisplayDlg::mouseMoveEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    // 目前无 DisplayDlg 级别的拖拽逻辑
}

//功能：处理颜色设置时的鼠标按下操作

void DisplayDlg::mousePressEvent(QMouseEvent* event)
{
    // event->pos() 是相对于 DisplayDlg 整体的坐标。
    // 布局：fpsLabel（高 32px）在上，displayLabel（640×480）在下。
    // 因此图像坐标 = 鼠标坐标 − fpsLabel 高度。
    QPoint pos = event->pos();

    if (m_setStatus == SET_STATUS::BORDER_SET) {
        // ── 将 DisplayDlg 坐标转换为图像坐标 ─────────────────
        QPoint imagePos(pos.x(), pos.y() - fpsLabel->height());

        // 确保点击落在图像范围内，否则忽略
        if (imagePos.x() < 0 || imagePos.x() >= DISPLAY_W ||
            imagePos.y() < 0 || imagePos.y() >= DISPLAY_H)
            return;

        // 记录标记点（以图像坐标存储）
        addCalibPoint(imagePos);

        // 转发给 DemarcateDlg 记录坐标
        if (m_pDemarcateDlg) {
            m_pDemarcateDlg->PushPoint(imagePos);
        }

        // 用最近一帧重绘，叠加所有已标记的十字（位置已是图像坐标）
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
    // 注意：COLOR_SET 分支已移除，框选功能现在由 OverlayWidget 接管。
    // OverlayWidget 作为 displayLabel 的子控件，直接处理鼠标事件，
    // 无需在 DisplayDlg 中进行坐标转换（fpsLabel 偏移等）。
    // 参见 OverlayWidget::mousePressEvent 和 OverlayWidget::mouseMoveEvent。
}

//功能：定时获取并处理图像

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

    // 预分配缓冲区，避免频繁内存分配
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



//功能：计算并显示实时帧率

void DisplayDlg::updateFPS()
{
    double avg = m_DisplayAvg.Avg();
    m_fps = avg == 0 ? 0.0 : 1000.0 / avg;
    m_DisplayAvg.Reset();
    fpsLabel->setText(QString("FPS: %1").arg(m_fps, 0, 'f', 2));
}

//功能：根据当前状态处理图像并显示

void DisplayDlg::ProcessImage(unsigned char* pBmp)
{
    Camera* pCamera = Camera::GetInstance();
    // 转换图像格式
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
        // IdentifyTest() 直接设置 displayLabel 的 pixmap，不需要 repaint
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

//功能：从摄像头获取一帧图像，使用与RetrieveResult一致的Pylon格式转换器

bool DisplayDlg::GrabSingle()
{
    Camera* pCamera = Camera::GetInstance();

    // If continuous grabbing is active, pause it briefly to use GrabOne.
    bool wasGrabbing = pCamera->IsGrabbing();
    if (wasGrabbing) {
        pCamera->StopGrabbing();
    }

    if (!pCamera->IsOpen()) {
        if (!pCamera->Open()) {
            // Camera unavailable – leave m_pDispSingle as-is (grey placeholder)
            if (wasGrabbing) pCamera->StartGrabbing();
            return false;
        }
    }

    // GrabOne uses ConvertBitmap internally; to stay consistent with
    // RetrieveResult (which uses Pylon CImageFormatConverter → RGB8),
    // we use StartGrabbing + RetrieveResult + StopGrabbing here too.
    bool result = false;
    if (pCamera->StartGrabbing()) {
        result = pCamera->RetrieveResult(m_pDispSingle);
        pCamera->StopGrabbing();
    }

    // Restore continuous grabbing if it was running before
    if (wasGrabbing) {
        pCamera->StartGrabbing();
    }

    return result;
}

//功能：从栈中弹出一个坐标点

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

// 功能：向栈中压入一个坐标点
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

// 功能：清空坐标点栈
void DisplayDlg::emptyStack()
{
    stackPointer = 0;
}

//功能：将RGB颜色转换为HSI颜色空间

void DisplayDlg::RGBToHS(int m, int n, unsigned char* P, int& H, int& S, int& I)
{
    int R, G, B;
    int index = (n * m_ImageSize.width() + m) * 3;
    // Pylon RGB8packed / QImage::Format_RGB888: byte order = R, G, B
    R = *(P + index + 0);
    G = *(P + index + 1);
    B = *(P + index + 2);
    H = 10 * HLUT[R][G][B];
    S = int(100 * (1 - 3.0 * MIN(R, G, B, 3) / (R + G + B)));
    I = (int)(R + G + B) / 3;
}

// 功能：绘制足球场背景、机器人和足球

void DisplayDlg::DrawAll(QPainter* painter)
{
    // 绘制足球场背景
    painter->drawImage(0, 0, m_groundImage);

    // 绘制机器人
    DrawRobot(painter);

    // 绘制对手
    DrawOpp(painter);

    // 绘制球
    DrawBall(painter);
}

//功能：绘制对方机器人

void DisplayDlg::DrawOpp(QPainter* painter)
{
    painter->setPen(QPen(Qt::magenta, 1));
    painter->setBrush(QBrush(Qt::green));

    for (int i = 0; i < MAX_ROBOT_NUM; i++)
    {
        if (OpprobotInfor[i].found)
        {
            int x = (int)(OpprobotInfor[i].x * 2.5) + 45;
            int y = (int)(OpprobotInfor[i].y * 2.5) + 15;
            int theta = (int)OpprobotInfor[i].theta;
            if (theta < 0) theta += 360;
            if (theta > 359) continue;

            // 绘制机器人形状
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

            // 绘制编号
            if (x - 4 >= 0 && y - 5 >= 0)
            {
                painter->drawText(x - 4, y - 5, 12, 13, Qt::AlignLeft, QString::number(i + 1));
            }
        }
    }
}

// 功能：绘制足球位置

void DisplayDlg::DrawBall(QPainter* painter)
{
    // 与 MFC DrawBall 一致：只画当前球位置的红色圆点，不画轨迹
    // MFC 通过 StretchBlt 从 groundDC 擦除上一帧 → 每帧重建场地背景自然实现
    if (ballInfor.found)
    {
        // 场地坐标 → 屏幕坐标（与 MFC 一致：x*2.5+45, y*2.5+15）
        int x = (int)(ballInfor.x * 2.5) + 45;
        int y = (int)(ballInfor.y * 2.5) + 15;

        // 绘制红色实心圆（半径 4px，与 MFC pDC->Ellipse(CRect(x-4,y-4,x+4,y+4)) 一致）
        painter->setPen(QPen(QColor(255, 128, 0), 1));
        painter->setBrush(QBrush(QColor(255, 128, 0)));
        painter->drawEllipse(x - 4, y - 4, 8, 8);
    }
}

//功能：绘制己方机器人

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

            // 绘制机器人形状
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

            // 绘制编号
            if (x - 4 >= 0 && y - 5 >= 0)
            {
                painter->drawText(x - 4, y - 5, 12, 13, Qt::AlignLeft, QString::number(i + 1));
            }
        }
    }
}

//功能：计算三个或四个值中的最小值
int DisplayDlg::GetMinValue(int val1, int val2, int val3, int val4)
{
    // 清晰的最小值逻辑，易维护
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

//功能：计算图像点的灰度值
int DisplayDlg::screenBuffer(int m, int n, unsigned char* P)
{
    int R, G, B;
    int index = (n * m_ImageSize.width() + m) * 3;
    R = *(P + index + 2);
    G = *(P + index + 1);
    B = *(P + index + 0);
    return (R * 30 + G * 60 + B * 10) / 100;
}

//功能：判断像素是否符合指定对象的颜色阈值

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


// 功能：判断像素颜色所属的对象类别

// 功能：根据参考点附近的 MEMB1/MEMB2 像素数量判断颜色类型
// 对应 MFC JudgeColor（E:\bot\RobotFootball\DisplayDlg.cpp:1740）
// a = MEMB1 像素数, b = MEMB2 像素数, c = 黑色像素数
// 返回: 1=MEMB1(紫), 2=MEMB2(绿), 0=黑色, -1=无法判断
int DisplayDlg::JudgeColor(int a, int b, int c)
{
    // MEMB1 和 MEMB2 同时大量出现 → 无法判断
    if (a >= 8 && b >= 8)
        return -1;
    if (a >= 8) return 1;      // MEMB1 为主
    if (b >= 8) return 2;      // MEMB2 为主
    if (c >= 8) return 0;      // 黑色为主
    return -1;                 // 像素数不足，无法判断
}

// 功能：查找符合目标对象颜色的像素

bool DisplayDlg::FindPixel(int object, int m, int n, unsigned char* P)
{
    int H, S, I;
    RGBToHS(m, n, P, H, S, I);
    return JudgePixel(object, H, S, I);
}

// 功能：扫描线泛洪填充搜索并识别目标对象
// 严格参考 MFC IdentifySearchLUT（E:\bot\RobotFootball\DisplayDlg.cpp:856）
// 改动：适配 Qt 的 top-down 缓冲区布局（3 字节/像素 unsigned char*）

bool DisplayDlg::IdentifySearchLUT(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char* pStart)
{
    int sum = 0, sumx = 0, sumy = 0;
    double sumxx = 0, sumyy = 0, sumxy = 0;
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

    // 扫描线泛洪填充（与 MFC 完全一致的算法）
    while (pop(x, y))
    {
        // 从当前点向上扫描找色块顶部
        y1 = y;
        while (y1 >= 0) {
            if (!FindPixel(tab, x, y1, pStart))
                break;
            y1--;
        }
        y1++;  // 回退到第一个匹配像素

        spanLeft = false;
        spanRight = false;

        // 从顶部向下扫描整列
        while (y1 < n)
        {
            if (!FindPixel(tab, x, y1, pStart))
                break;

            // 标记已访问（3 字节全设为灰色，防止 FindPixel 再次匹配）
            int idx = (y1 * m + x) * 3;
            pStart[idx]     = 100;  // R
            pStart[idx + 1] = 100;  // G
            pStart[idx + 2] = 100;  // B

            sum++;
            sumx += x;
            sumy += y1;
            sumxx += (double)x * x;
            sumyy += (double)y1 * y1;
            sumxy += (double)x * y1;

            // 更新边界框
            if (x <= m_xLeft) m_xLeft = x;
            else if (x >= m_xRight) m_xRight = x;
            if (y1 <= m_yTop) m_yTop = y1;
            else if (y1 >= m_yBottom) m_yBottom = y1;

            // 左侧邻居扩展
            if (!spanLeft && x > 0 && FindPixel(tab, x - 1, y1, pStart))
            {
                if (!push(x - 1, y1)) return false;
                spanLeft = true;
            }
            else if (spanLeft && x > 0 && !FindPixel(tab, x - 1, y1, pStart))
            {
                spanLeft = false;
            }

            // 右侧邻居扩展
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

    // 尺寸和形状检查（与 MFC 一致）
    if (sum >= SizeMin && sum <= SizeMax)
    {
        // 形状检查：宽高差不能超过 20（排除长条形误检）
        if (abs((m_xRight - m_xLeft) - (m_yBottom - m_yTop)) > 20)
            return false;

        // 计算色块质心（与 MFC m_Target 一致）
        m_Target[tab].setX(sumx / sum);
        m_Target[tab].setY(sumy / sum);

        // PCA 求色块主方向角，替代旧版边界框宽高比估算（仅 0°/90°）
        {
            double cx = (double)sumx / sum;
            double cy = (double)sumy / sum;
            double mu20 = sumxx / sum - cx * cx;
            double mu02 = sumyy / sum - cy * cy;
            double mu11 = sumxy / sum - cx * cy;
            double delta = mu20 - mu02;
            // 退化保护：圆形色块无主方向，回退到边界框判断
            if (fabs(delta) < 1e-6 && fabs(mu11) < 1e-6)
            {
                int w = m_xRight - m_xLeft;
                int h = m_yBottom - m_yTop;
                m_PatchAngle = (w >= h) ? 0.0 : M_PI / 2;
            }
            else
            {
                m_PatchAngle = 0.5 * atan2(2.0 * mu11, delta);
            }
        }
        return true;
    }
    return false;
}

//功能：启动目标识别（球 + 己方机器人 + 对手）

void DisplayDlg::StartTest()
{
    // 清空上一帧的识别结果
    for (int i = 0; i < MAX_ROBOT_NUM; i++) {
        robotInfor[i].found = false;
        OpprobotInfor[i].found = false;
        ObjectFound[i] = false;
    }
    ballInfor.found = false;
    ObjectFound[10] = false;
    ObjectFound[11] = false;

    // 设置识别图像指针（FindBlackID/FindRobotID 需要读取原始图像）
    // 对应 MFC: m_pIdentify = (RGBTRIPLE*)m_pDispBitmap;
    m_pIdentify = m_pDispBitmap;

    IdentifyAll();        // 识别球（阈值 0 = 我方队色）
    IdentiRobo(1);        // 识别己方机器人 MEMB1（阈值 1）
    IdentiRobo(2);        // 识别己方机器人 MEMB2 / 对手（阈值 2）
    IdentiRobo(4);        // 识别对手（阈值 4 = 敌方队色）
    BallPosFilter();      // 球位置滤波防抖
}

//功能：识别足球和机器人
void DisplayDlg::IdentifyAll()
{
    int i, j;
    int xLeftTem, xRightTem, yTopTem, yBottomTem;
    int m = m_ImageSize.width();
    int n = m_ImageSize.height();
    int index;
    int H, S, I;

    for (i = 0; i < MAX_ROBOT_NUM; i++)
    {
        ObjectFound[i] = false;
    }
    ObjectFound[10] = false;
    ObjectFound[11] = false;

    m_xLeft = m;
    m_xRight = 0;
    m_yTop = n;
    m_yBottom = 0;

    unsigned char* m_pTestBitmap = new unsigned char[m * n * 3];
    memcpy(m_pTestBitmap, m_pDispBitmap, m * n * 3);
    unsigned char* pTest = m_pTestBitmap;

    ColorDlg* pColorDlg = m_pColorDlg ? m_pColorDlg : ColorDlg::getInstance();
    const int(*HSIThreshold)[6] = pColorDlg->getHSIThreshold();

    // ── 球识别（对应 MFC IdentifyAll 中的球检测部分，E:\bot\RobotFootball\DisplayDlg.cpp:934） ──
    // MFC 做法：步长 4 扫描，BALL=6 阈值，面积 30~300，选最大候选
    // Qt 对应：步长 4，我方队色阈值（object=0），面积 30~300，选最大候选
    struct BallCandidate { int x, y, num; };
    BallCandidate ballCandidates[5];
    int NumBall = 0;

    for (j = 0; j < n; j += 4)
    {
        for (i = 0; i < m; i += 4)
        {
            RGBToHS(i, j, pTest, H, S, I);

            // 使用我方队色阈值（object=0）
            if (H >= HSIThreshold[0][0] && H <= HSIThreshold[0][1] &&
                S >= HSIThreshold[0][2] && S <= HSIThreshold[0][3] &&
                I >= HSIThreshold[0][4] && I <= HSIThreshold[0][5])
            {
                xLeftTem = m_xLeft;
                xRightTem = m_xRight;
                yTopTem = m_yTop;
                yBottomTem = m_yBottom;
                m_xLeft = m;
                m_xRight = 0;
                m_yTop = n;
                m_yBottom = 0;

                if (NumBall < 5 && IdentifySearchLUT(0, i, j, 30, 300, pTest))
                {
                    int x = (m_xLeft + m_xRight) / 2;
                    int y = (m_yTop + m_yBottom) / 2;
                    ballCandidates[NumBall].x = x;
                    ballCandidates[NumBall].y = y;
                    ballCandidates[NumBall].num = (m_xRight - m_xLeft) * (m_yBottom - m_yTop);
                    NumBall++;
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

    // 选最大候选（对应 MFC: if (TemBall[0].num < TemBall[i].num) TemBall[0] = TemBall[i]）
    if (NumBall >= 1)
    {
        int bestIdx = 0;
        for (int k = 1; k < NumBall; k++)
        {
            if (ballCandidates[k].num > ballCandidates[bestIdx].num)
                bestIdx = k;
        }
        int bx = ballCandidates[bestIdx].x;
        int by = ballCandidates[bestIdx].y;
        if (bx >= 0 && bx < DISPLAY_W && by >= 0 && by < DISPLAY_H) {
            // Y 翻转：Qt top-down 像标 → MFC bottom-up 场地标
            int gby = DISPLAY_H - 1 - by;
            ballInfor.x = ground.groundInfo[bx][gby].x;
            ballInfor.y = ground.groundInfo[bx][gby].y;
        }
        ballInfor.found = true;
        ballInfor.theta = 0;
        ObjectFound[10] = true;
    }

    delete[] m_pTestBitmap;
}

// 功能：过滤足球位置，防止抖动

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

// 功能：识别己方和对方机器人

// 功能：识别己方和对方机器人
// 对应 MFC IdentiRobo（E:\bot\RobotFootball\DisplayDlg.cpp:1189）
//
// 己方机器人识别流程：
//   1. 泛洪填充找到队色色块，得到中心坐标 (TeamTarget)
//   2. 通过色块形状计算法线角度 (NormalTheta)
//   3. 根据角度计算 4 个参考点位置（距离中心 m_Length 处）
//   4. FindBlackID 检测参考点黑色/非黑色 → 确定机器人朝向
//   5. FindRobotID 检测参考点的 MEMB1/MEMB2 颜色组合 → 确定编号
//   6. ground.groundInfo[][] 将像素坐标转为场地坐标
//
// 对手机器人：只检测色块位置，不做编号识别
void DisplayDlg::IdentiRobo(int ObjectCount)
{
    int i, j;
    int m = m_ImageSize.width();
    int n = m_ImageSize.height();
    int H, S, I;
    int xLeftTem, xRightTem, yTopTem, yBottomTem;
    int robotNum = 0;

    // 临时存储识别结果（与 MFC TemTeam/TemOpp 对应）
    QPoint TeamTarget[20];
    double NormalTheta[20];
    int NumTeam = 0;

    unsigned char* m_pTestBitmap = new unsigned char[m * n * 3];
    memcpy(m_pTestBitmap, m_pDispBitmap, m * n * 3);
    unsigned char* pTest = m_pTestBitmap;

    ColorDlg* pColorDlg = m_pColorDlg ? m_pColorDlg : ColorDlg::getInstance();
    const int(*HSIThreshold)[6] = pColorDlg->getHSIThreshold();

    // ── 第一遍扫描：找到所有色块 ──
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
                        // ── 对手：只记录位置，不做编号识别 ──
                        if (robotNum < MAX_ROBOT_NUM && x >= 0 && x < DISPLAY_W && y >= 0 && y < DISPLAY_H)
                        {
                            // Y 翻转：Qt top-down 像标 → MFC bottom-up 场地标
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
                        // ── 己方：记录色块中心，计算法线角度 ──
                        if (NumTeam < 20)
                        {
                            TeamTarget[NumTeam] = QPoint(x, y);

                            // 用 PCA 主方向替代边界框宽高比，支持任意角度
                            NormalTheta[NumTeam] = m_PatchAngle;
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

    // ── 第二遍：对己方色块进行编号识别 ──
    // 对应 MFC IdentiRobo（E:\bot\RobotFootball\DisplayDlg.cpp:1189）
    double m_Length = 7.5;
    m_pIdentify = m_pDispBitmap;  // FindBlackID 需要读取原始图像

    for (i = 0; i < NumTeam && robotNum < MAX_ROBOT_NUM; i++)
    {
        int RobotID = -1;
        double OrientAngle = 0;

        // 根据法线角度计算 4 个参考点位置
        // 对应 MFC：temptheta = Pi/2 - atan(0.75) - NormalTheta[i]
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

        // 根据黑色参考点组合确定朝向和编号
        if (blackID[0] && blackID[1])
        {
            // 参考点 0,1 为黑 → 朝向 = NormalTheta + π
            OrientAngle = NormalTheta[i] + M_PI;
            RobotID = FindRobotID(ReferPoint[2], ReferPoint[3]);
        }
        else if (blackID[2] && blackID[3])
        {
            // 参考点 2,3 为黑 → 朝向 = NormalTheta
            OrientAngle = NormalTheta[i];
            RobotID = FindRobotID(ReferPoint[0], ReferPoint[1]);
        }
        else if (!blackID[0] && !blackID[1])
        {
            // 参考点 0,1 非黑 → 朝向 = NormalTheta
            OrientAngle = NormalTheta[i];
            RobotID = FindRobotIDD(ReferPoint[2], ReferPoint[3]);
        }
        else if (!blackID[2] && !blackID[3])
        {
            // 参考点 2,3 非黑 → 朝向 = NormalTheta + π
            OrientAngle = NormalTheta[i] + M_PI;
            RobotID = FindRobotIDD(ReferPoint[0], ReferPoint[1]);
        }

        if (RobotID >= 0 && RobotID < MAX_ROBOT_NUM)
        {
            int px = TeamTarget[i].x();
            int py = TeamTarget[i].y();
            if (px >= 0 && px < DISPLAY_W && py >= 0 && py < DISPLAY_H)
            {
                // Y 翻转：Qt top-down 像标 → MFC bottom-up 场地标
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

// 功能：通过参考点的 MEMB1/MEMB2 颜色组合查找机器人编号
// 对应 MFC FindRobotID（E:\bot\RobotFootball\DisplayDlg.cpp:1377）
// 在参考点 RP1 和 RP2 附近各取 5x5 区域，统计 MEMB1/MEMB2/黑色像素数量
// 根据颜色组合确定机器人编号
//
// 编号对照表：
//   RPID1=0(黑)  RPID2=1(紫)  → 0号车
//   RPID1=1(紫)  RPID2=0(黑)  → 1号车
//   RPID1=1(紫)  RPID2=1(紫)  → 2号车
//   RPID1=0(黑)  RPID2=2(绿)  → 3号车
//   RPID1=2(绿)  RPID2=0(黑)  → 4号车
int DisplayDlg::FindRobotID(QPoint RP1, QPoint RP2)
{
    int roboID, RPID1, RPID2;
    int ii, jj;
    int sum1, sum2, sum0;
    int H = 0, S = 0, I = 0;

    // 检测参考点 1 附近的 5x5 区域
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

    // 检测参考点 2 附近的 5x5 区域
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

    // 根据颜色组合确定机器人编号
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

// 功能：通过参考点的 MEMB1/MEMB2 颜色组合查找机器人编号（备用方法）
// 对应 MFC FindRobotIDD（E:\bot\RobotFootball\DisplayDlg.cpp:1333）
// 与 FindRobotID 相同的检测逻辑，用于当黑色参考点检测不确定时的后备
int DisplayDlg::FindRobotIDD(QPoint RP1, QPoint RP2)
{
    // 与 FindRobotID 使用相同的颜色组合逻辑
    return FindRobotID(RP1, RP2);
}

// 功能：查找黑色区域

// 功能：检测参考点附近是否有黑色区域
// 对应 MFC FindBlackID（E:\bot\RobotFootball\DisplayDlg.cpp:1302）
// 检查以 (m,n) 为中心的 3x3 区域，统计非 MEMB1/MEMB2 且非场地背景的黑色像素
// 黑色像素 >= 5 则判定为黑色区域
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
            // 不是 MEMB1、不是 MEMB2、不是场地背景(12684) → 黑色
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

// 功能：搜索对手和足球

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

//功能：搜索队伍

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

// 功能：图像分割模式的动态测试
// 严格参考 MFC IdentifyTest（E:\bot\RobotFootball\DisplayDlg.cpp:785-833）
// 黑色背景上用绿色边框+红色十字线标记检测到的色块

void DisplayDlg::IdentifyTest()
{
    // 对应 MFC IdentifyTest（E:\bot\RobotFootball\DisplayDlg.cpp:785）
    // 每帧重新创建黑色背景，只在检测到色块的位置画绿色边框+红色十字线

    // 获取当前采色对象（使用实际的标签页实例，而非单例）
    ColorDlg* pColorDlg = m_pColorDlg ? m_pColorDlg : ColorDlg::getInstance();
    int object = pColorDlg->currentObject();
    qDebug() << "[IdentifyTest] object=" << object
             << "H=[" << pColorDlg->getHSIThreshold()[object][0]
             << "," << pColorDlg->getHSIThreshold()[object][1] << "]";
    if (object < 0 || object >= 8) return;

    // 创建黑色背景 QImage（每帧重建，对应 MFC destDC.FillSolidRect 黑色）
    QImage segImage(DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
    segImage.fill(Qt::black);

    // 拷贝 m_pDispBitmap 到临时缓冲区（泛洪填充会修改像素数据，不能改原图）
    int bufSize = DISPLAY_W * DISPLAY_H * 3;
    unsigned char* pTestBitmap = new unsigned char[bufSize];
    memcpy(pTestBitmap, m_pDispBitmap, bufSize);

    // 创建 QPainter（在循环外创建一次，对应 MFC 在循环外创建 pDC1/pDC2）
    QPainter painter(&segImage);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // 绿色画笔（2px）用于画色块边框，对应 MFC IdentiPen1(PS_SOLID, 2, RGB(0,255,0))
    QPen greenPen(QColor(0, 255, 0), 2);
    // 红色画笔（1px）用于画十字线，对应 MFC IdentiPen2(PS_SOLID, 1, RGB(255,0,0))
    QPen redPen(QColor(255, 0, 0), 1);

    int Num = 0;

    // 每隔 4 像素扫描（对应 MFC for i=0..DISPLAY_W step 4, j=0..DISPLAY_H step 4）
    for (int i = 0; i < DISPLAY_W; i += 4) {
        for (int j = 0; j < DISPLAY_H; j += 4) {
            // 检测当前像素是否匹配对象颜色
            if (FindPixel(object, i, j, pTestBitmap)) {
                // 泛洪填充找色块（面积 30~300 像素，最多 20 个）
                if (Num < 20 && IdentifySearchLUT(object, i, j, 30, 300, pTestBitmap)) {
                    // 边界有效性检查（对应 MFC m_Target 范围判断）
                    if (m_Target[object].x() < 0 || m_Target[object].x() >= DISPLAY_W ||
                        m_Target[object].y() < 0 || m_Target[object].y() >= DISPLAY_H)
                        continue;

                    // 画绿色边框（对应 MFC pDC1->MoveTo/LineTo 四条边）
                    painter.setPen(greenPen);
                    painter.setBrush(Qt::NoBrush);
                    painter.drawRect(m_xLeft, m_yTop,
                                    m_xRight - m_xLeft, m_yBottom - m_yTop);

                    // 画红色十字线（对应 MFC pDC2 水平线+垂直线）
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

    // 显示分割结果（对应 MFC SetDIBitsToDevice）
    displayLabel->setPixmap(QPixmap::fromImage(segImage));
}

//功能：开始比赛，进行目标识别和绘制

void DisplayDlg::StartGame()
{
    // 比赛开始，需要进行目标识别和绘制
    // 定时器会触发onTimer，持续更新图像
}

//功能：分析矩形区域内的颜色

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

// 功能：分析点集合内的颜色

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