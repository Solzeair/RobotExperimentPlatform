#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QPainter>
#include <QFile>
#include <QIODevice>
#include <vector>
#include <algorithm>
/*
* DisplayDlg.h - 显示对话框头文件
*
* 功能：
* 1. 定义显示对话框的接口和功能
* 2. 管理左侧显示区域的图像显示
* 3. 处理相机图像的获取和显示
* 4. 实现帧率计算和显示
* 5. 支持用户交互和状态管理
* 6. 实现目标识别和图像处理
* 7. 支持足球比赛场景的显示
*/

// CMovingAvg模板类 - 用于计算移动平均
template <class T, unsigned int span = 20>
class CMovingAvg
{
public:
    CMovingAvg() : m_v(span), m_n(0), m_Sum(0) {}

    void Add(T sample)
    {
        int idx = m_n % span;
        if (m_n < span)
        {
            m_v[idx] = sample;
            m_Sum += sample;
        }
        else
        {
            m_Sum -= m_v[idx];
            m_Sum += sample;
            m_v[idx] = sample;
        }
        m_n++;
    }

    double Avg()
    {
        if (m_n == 0)
            return 0;
        return m_n < span ? m_Sum / (double)m_n : m_Sum / (double)span;
    }

    void Reset()
    {
        m_n = 0;
        m_Sum = 0;
    }

private:
    std::vector<T> m_v;
    unsigned int m_n;
    T m_Sum;
};

// RGB -> H 转换表（DisplayDlg.cpp 定义，供 ColorDlg 采样使用）
extern int HLUT[256][256][256];

// 常量定义
#ifndef DISPLAY_W
static const int DISPLAY_W = 640;
#endif
#ifndef DISPLAY_H
static const int DISPLAY_H = 480;
#endif
#ifndef MAX_ROBOT_NUM
static const int MAX_ROBOT_NUM = 5;
#endif
static const int BALL = 0; // Object type identifier for the ball

// 机器人信息结构体
typedef struct {
    double x;
    double y;
    double theta;
    int num;
    bool found;
} RobotInford;

// 对手信息结构体
typedef struct {
    double x;
    double y;
    int num;
} OppInf;

// ========================================================================
// OverlayWidget - 透明覆盖层控件
// ========================================================================
// 功能说明：
//   此类是一个透明的 QWidget，叠在 displayLabel（摄像头画面）上方，
//   专门用于在采色模式（COLOR_SET）下绘制鼠标框选的红框矩形。
//
// 设计动机（对比 MFC 版本）：
//   MFC 中使用 CDC::SetROP2(R2_NOTXORPEN) 直接在屏幕 DC 上以 XOR
//   模式画红框，矩形和摄像头画面在同一个像素层，互不干扰。
//   Qt 中若在 QLabel 上直接用 QPainter 画矩形，会被 setPixmap()
//   设置的图像覆盖（QLabel 绘制顺序：先画 pixmap，后画控件内容）。
//   因此引入独立的透明覆盖层，将"图像显示"和"框选绘制"分离到
//   两个独立的控件层，达到与 MFC XOR 模式相同的效果：
//   - displayLabel 只负责显示摄像头画面，不被框选操作污染
//   - OverlayWidget 只负责画红框，不影响底层图像数据
//
// 工作流程：
//   1. 用户切换到"采色"标签页 → SelectSetStatus(COLOR_SET)
//      → 显示覆盖层并启用鼠标追踪
//   2. 用户在覆盖层上按下鼠标 → 记录框选起点
//   3. 用户拖动鼠标 → 实时更新矩形范围，通过 update() 触发重绘
//   4. paintEvent 中用 XOR 组合模式绘制红色矩形线框
//   5. ColorDlg::onZoom() 通过 DisplayDlg::GetRect() 获取框选矩形
//      → 从原始帧缓冲 m_pDispSingle 中裁切放大区域
//   6. 用户切换到其他标签页 → SelectSetStatus(NONE)
//      → 隐藏覆盖层
// ========================================================================
class OverlayWidget : public QWidget
{
public:
    explicit OverlayWidget(QWidget* parent = nullptr);

    // 设置框选矩形（图像坐标），标记为有效并触发重绘
    void setSelectionRect(const QRect& r);

    // 清除框选矩形，标记为无效并触发重绘
    void clearSelectionRect();

    // 获取当前框选矩形（图像坐标）
    QRect getSelectionRect() const { return m_selectionRect; }

    // 框选矩形是否有效（用户是否已完成至少一次框选）
    bool hasSelection() const { return m_hasSelection; }

protected:
    // 重绘事件：用 XOR 模式绘制红色矩形线框
    // 每次调用 update() 时自动触发，先清除旧矩形再画新矩形
    void paintEvent(QPaintEvent* event) override;

    // 鼠标按下：记录框选起点，初始化一个 1×1 的矩形
    void mousePressEvent(QMouseEvent* event) override;

    // 鼠标拖动：更新矩形终点，调用 update() 触发重绘
    // 由于 paintEvent 每次从干净状态重画 XOR 矩形，
    // 视觉效果等同于 MFC 的"擦旧画新"两步操作
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QRect m_selectionRect;   // 框选矩形（覆盖层控件坐标系）
    bool m_hasSelection;     // 框选矩形是否有效
};


class DisplayDlg : public QWidget
{
    Q_OBJECT

public:
    enum class STATUS {
        PrepareStop,  // 准备停止
        Stop,         // 停止状态
        Display,      // 实时显示
        RunTest,      // 动态测试
        RunTestSeg,   // 分割后的动态测试
        Prepare,      // 初始准备
        Game          // 比赛开始
    };

    enum class SET_STATUS {
        NONE,
        BORDER_SET,
        COLOR_SET
    };

public:
    DisplayDlg(QWidget* parent = nullptr);
    ~DisplayDlg();

    void ShowSingle();

    // Add a calibration point marker drawn on top of the live image.
    // Called by DemarcateDlg::PushPoint() so the operator sees where
    // each click landed.  Markers persist until clearCalibPoints().
    void addCalibPoint(const QPoint& pt) { m_calibPoints.push_back(pt); }

    // Remove all calibration point markers (called on reset).
    void clearCalibPoints() { m_calibPoints.clear(); }

    // 清除采色模式覆盖层上的框选矩形
    // 供 ColorDlg 在切换测试模式时调用，清除左侧显示区的红框
    void clearOverlaySelection() { if (m_overlayWidget) m_overlayWidget->clearSelectionRect(); }

    // Accessor for the single-grab pixel buffer.
    // Used by DemarcateDlg::applyPerspectiveCorrection() to read and
    // write back the camera frame before the polynomial fit is run.
    // Returns a pointer to the raw RGB24 buffer (DISPLAY_W * DISPLAY_H * 3 bytes).
    unsigned char* getDispSingle() { return m_pDispSingle; }
    unsigned char* getDispBitmap() { return m_pDispBitmap; }

    // Register the DemarcateDlg so that BORDER_SET mouse clicks
    // are forwarded to DemarcateDlg::PushPoint().
    // Called from QtWidgetDesign.cpp after both objects are created.
    void setDemarcateDlg(class DemarcateDlg* dlg) { m_pDemarcateDlg = dlg; }
    void ShowDynamic();
    void ShowCarNum();
    void ShowColorTest(int(*HSI)[6], int object);
    void ShowRunTest(bool ImageSeg);
    void ShowInitGame();
    void ShowStartGame();
    void Stop();

    void SelectSetStatus(SET_STATUS s);

    // 颜色分析
    // 获取当前框选矩形（图像坐标）。
    // 采色模式下从 OverlayWidget 读取（覆盖层坐标系，已对齐图像）；
    // 其他模式下返回 DisplayDlg 自身的 m_Rect（如 BORDER_SET 标定模式）。
    QRect GetRect() const;
    void ColorAnalyse(const QRect& rect, int yi[], std::vector<QPoint>& vecColorSet);
    void ColorAnalyse(const std::vector<QPoint>& pts, int yi[], std::vector<QPoint>& vecColorSet);
    int MINS(int R, int G, int B, int N);
    int GetMinValue(int val1, int val2, int val3, int val4);
    void RGBToHS(int m, int n, unsigned char* P, int& H, int& S, int& I);
    void ClearBallTrail();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void updateFPS();
    void onTimer();           // 定时抓帧+处理循环（主线程）

private:
    void initUI();
    void ProcessImage(unsigned char* pBmp);
    void StartGame();
    bool GrabSingle();
    void IdentifyTest();
    void StartTest();

    bool FindPixel(int object, int m, int n, unsigned char* P);
    bool IdentifySearchLUT(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char* pStart);
    void IdentifyAll();
    void IdentiRobo(int ObjectCount);

    void BallPosFilter();
    int FindRobotID(QPoint RP1, QPoint RP2);
    int FindRobotIDD(QPoint RP1, QPoint RP2);
    bool FindBlackID(int m, int n, int Num);

    bool SeachOppAndBall(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char* pStart);
    bool SearchTeam(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char* pStart);

    int screenBuffer(int m, int n, unsigned char* P);
    bool JudgePixel(int object, int H, int S, int I);
    int JudgeColor(int a, int b, int c);

    void DrawAll(QPainter* painter);
    void DrawOpp(QPainter* painter);
    void DrawBall(QPainter* painter);
    void DrawRobot(QPainter* painter);

private:
    // 图像
    QSize m_ImageSize;
    unsigned char* m_pDispBitmap, * m_pDispSingle;
    unsigned char* m_pIdentify; // image data pointer

    // Pointer to the calibration dialog; set via setDemarcateDlg().
    // When m_setStatus == BORDER_SET mouse clicks are forwarded here.
    class DemarcateDlg* m_pDemarcateDlg = nullptr;

    // Calibration point markers overlaid on the image.
    // Populated by addCalibPoint(), cleared by clearCalibPoints().
    std::vector<QPoint> m_calibPoints;

    // 定时器
    QTimer* m_grabTimer;     // 抓帧 + 处理定时器（主线程驱动）
    bool m_bErrorSign;

    // 状态
    STATUS m_status;
    SET_STATUS m_setStatus;

    // 显示
    QElapsedTimer m_DisplayWatch;

    // 绘图
    QImage m_groundImage;
    QPixmap m_carNumPixmap; // 车号图像（预加载）

    // 目标识别
    bool ObjectFound[12];

    // 球轨迹
    static const int MAX_TRAIL_LENGTH = 100;
    std::vector<QPoint> m_ballTrail;

    // 颜色选择区域
    QRect m_Rect;

    // 机器人信息
    RobotInford fullrobotinfor[11];
    RobotInford robotInfor[MAX_ROBOT_NUM], robotBk[MAX_ROBOT_NUM];
    RobotInford OpprobotInfor[MAX_ROBOT_NUM], OpprobotBk[MAX_ROBOT_NUM];
    RobotInford ballInfor, ballBk;

    // 绘图临时数据
    QPoint TeamTarget[20];
    QPoint m_Target[9];
    OppInf m_TargetN, m_TargetN1, m_TargetN2;
    OppInf OpprobotInforTem[20];    //临时存储对手信息

    // 栈操作
    static const int StackSize = 200;
    int stackx[StackSize];
    int stacky[StackSize];
    int stackPointer;
    bool pop(int& x, int& y);
    bool push(int x, int y);
    void emptyStack();

    int m_xLeft, m_xRight, m_yTop, m_yBottom;
    bool m_IdenOp;
    int BlackSum[4];
    double NormalTheta[20];
    double m_theta;
    double m_Length;
    bool patchConnect;    // 小块是否连接
    bool m_BalLo;

    // 帧率计算相关
    CMovingAvg<double, 20> m_DisplayAvg;
    double m_fps;
    QTimer* fpsTimer;
    QLabel* fpsLabel;

    // 界面控件
    QLabel* displayLabel;
    QVBoxLayout* mainLayout;

    // 采色模式的透明覆盖层控件，叠在 displayLabel 上方
    // 仅在 m_setStatus == COLOR_SET 时可见，用于绘制鼠标框选红框
    OverlayWidget* m_overlayWidget;

};