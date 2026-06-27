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
* DisplayDlg - 相机画面显示与目标识别对话框
* 负责抓帧显示、颜色采样、场地标定与机器人/球识别，
* 覆盖实时显示、动态测试与比赛等运行状态。
*/

// CMovingAvg：定长滑动窗口求均值，平滑抖动较大的实时测量（如帧率）
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

// 显示分辨率与目标标识常量（可用宏在编译期覆盖默认值）
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

// 视觉识别得到的机器人位姿（场地坐标 + 编号 + 是否检测到）
typedef struct {
    double x;
    double y;
    double theta;
    int num;
    bool found;
} RobotInford;

// 对手机器人位姿（无需朝向，故省略 theta）
typedef struct {
    double x;
    double y;
    int num;
} OppInf;

// ========================================================================
// OverlayWidget - 透明覆盖层控件
// ========================================================================
// 叠在 displayLabel 上方，专门在采色模式（COLOR_SET）下绘制鼠标框选红框。
//
// 设计动机：MFC 用 SetROP2(R2_NOTXORPEN) 在屏幕 DC 上以 XOR 模式画框，
// 框与画面同层互不干扰；Qt 中 QLabel 先绘 pixmap 再绘控件内容，直接在其上
// 画框会被图像覆盖。故引入独立透明层，将"图像显示"与"框选绘制"分离：
// displayLabel 只显示画面，OverlayWidget 只画红框，互不污染。
//
// 流程：SelectSetStatus(COLOR_SET) 显示覆盖层 → 鼠标拖动框选并 update() 重绘
// → paintEvent 以 XOR 模式画红框 → ColorDlg::onZoom() 经 GetRect() 取框
// → 从 m_pDispSingle 裁切放大；切走标签页时 SelectSetStatus(NONE) 隐藏。
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
    // 重绘事件：以 XOR 模式绘制红色框选线框，重绘即自动擦旧画新
    void paintEvent(QPaintEvent* event) override;

    // 鼠标按下：记录框选起点，初始化一个 1×1 的矩形
    void mousePressEvent(QMouseEvent* event) override;

    // 鼠标拖动：更新矩形终点并 update() 重绘；paintEvent 每次从干净状态重画，
    // 故单次重绘即达 MFC"擦旧画新"两步的视觉效果
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
    void clearOverlaySelection();

    // 设置采色对话框实例指针（由 QtWidgetDesign 构造函数调用）
    void setColorDlg(class ColorDlg* dlg);

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
    bool IdentifySearchLUT(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char* pStart, bool isBall = false);
    void IdentifyAll();
    void IdentiRobo(int ObjectCount);
    void IdentiRoboFromTargets(QPoint targets[], double normalTheta[], int count, bool isOpponent);

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
    unsigned char* m_pIdentify;

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

    // 泛洪填充（区域生长）显式栈：用定长数组+指针替代递归，规避爆栈
    static const int StackSize = 200;
    int stackx[StackSize];
    int stacky[StackSize];
    int stackPointer;
    int m_lastBlobCount = 0;  // IdentifySearchLUT 最近一次泛洪填充的像素计数（对应 MFC sum）
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

    // 采色对话框实际实例指针（标签页中的实例，非单例）
    // 由 QtWidgetDesign 构造函数通过 setColorDlg() 设置
    class ColorDlg* m_pColorDlg = nullptr;

};