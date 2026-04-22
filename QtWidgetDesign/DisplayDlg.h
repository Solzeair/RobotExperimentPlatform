#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QPainter>
#include <vector>

// 常量定义
const int DISPLAY_W = 640;
const int DISPLAY_H = 480;
const int MAX_ROBOT_NUM = 5;
const int BALL = 0; // 球的对象类型标识

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
    DisplayDlg(QWidget *parent = nullptr);
    ~DisplayDlg();

    void ShowSingle();
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
    void ColorAnalyse(const QRect &rect, int yi[], std::vector<QPoint> &vecColorSet);
    void ColorAnalyse(const std::vector<QPoint> &pts, int yi[], std::vector<QPoint> &vecColorSet);
    int MIN(int R, int G, int B, int N);
    void RGBToHS(int m, int n, unsigned char *P, int &H, int &S, int &I);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onTimer();
    void updateFPS();

private:
    void initUI();
    void ProcessImage(unsigned char *pBmp);
    void StartGame();
    bool GrabSingle();
    void IdentifyTest();
    void StartTest();

    bool FindPixel(int object, int m, int n, unsigned char *P);
    bool IdentifySearchLUT(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char *pStart);
    void IdentifyAll();
    void IdentiRobo(int ObjectCount);

    void BallPosFilter();
    int FindRobotID(QPoint RP1, QPoint RP2);
    int FindRobotIDD(QPoint RP1, QPoint RP2);
    bool FindBlackID(int m, int n, int Num);

    bool SeachOppAndBall(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char *pStart);
    bool SearchTeam(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char *pStart);

    int screenBuffer(int m, int n, unsigned char *P);
    bool JudgePixel(int object, int H, int S, int I);
    int JudgeColor(int a, int b, int c);

    void DrawAll(QPainter *painter);
    void DrawOpp(QPainter *painter);
    void DrawBall(QPainter *painter);
    void DrawRobot(QPainter *painter);

private:
    // 图像
    QSize m_ImageSize;
    unsigned char *m_pDispBitmap, *m_pDispSingle;
    unsigned char *m_pIdentify; // 图像数据指针

    // 线程
    QTimer *m_grabTimer;
    bool m_bErrorSign;

    // 状态
    STATUS m_status;
    SET_STATUS m_setStatus;

    // 显示
    QElapsedTimer m_DisplayWatch;
    double m_DisplayAvg[20];
    int m_DisplayAvgCount;



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
    bool pop(int &x, int &y);
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
    int frameCount;
    QElapsedTimer lastTime;
    QTimer *fpsTimer;
    QLabel *fpsLabel;

    // 界面控件
    QLabel *displayLabel;
    QVBoxLayout *mainLayout;
};