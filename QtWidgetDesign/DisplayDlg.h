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
#include<algorithm>
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

    // Accessor for the single-grab pixel buffer.
    // Used by DemarcateDlg::applyPerspectiveCorrection() to read and
    // write back the camera frame before the polynomial fit is run.
    // Returns a pointer to the raw RGB24 buffer (DISPLAY_W * DISPLAY_H * 3 bytes).
    unsigned char* getDispSingle() { return m_pDispSingle; }

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
    QRect GetRect() const { return m_Rect; }
    void ColorAnalyse(const QRect& rect, int yi[], std::vector<QPoint>& vecColorSet);
    void ColorAnalyse(const std::vector<QPoint>& pts, int yi[], std::vector<QPoint>& vecColorSet);
    int MINS(int R, int G, int B, int N);
    int GetMinValue(int val1, int val2, int val3, int val4);
    void RGBToHS(int m, int n, unsigned char* P, int& H, int& S, int& I);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onTimer();
    void updateFPS();

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
    void ClearBallTrail(); // 清除足球轨迹

private:
    // 图像
    QSize m_ImageSize;
    unsigned char* m_pDispBitmap, * m_pDispSingle;
    unsigned char* m_pIdentify; // image data pointer

    // Pointer to the calibration dialog; set via setDemarcateDlg().
    // When m_setStatus == BORDER_SET mouse clicks are forwarded here.
    class DemarcateDlg* m_pDemarcateDlg = nullptr;

    // 线程
    QTimer* m_grabTimer;
    bool m_bErrorSign;

    // 状态
    STATUS m_status;
    SET_STATUS m_setStatus;

    // 显示
    QElapsedTimer m_DisplayWatch;

    // 绘图
    QImage m_groundImage;
    QPixmap m_carNumPixmap; // 车号图像（预加载）

    // 足球轨迹
    std::vector<QPoint> m_ballTrail;
    static const int MAX_TRAIL_LENGTH = 50; // 轨迹最大长度

    // 目标识别
    bool ObjectFound[12];

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
};