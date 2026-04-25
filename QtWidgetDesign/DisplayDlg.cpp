// DisplayDlg.cpp - 左侧显示区域实现文件
// 功能：实现摄像头图像显示、目标识别、颜色分析和足球机器人比赛相关功能
//代码存在问题未修改
#include "DisplayDlg.h"
#include "Camera.h"
#include "Debug.h"
#include "ColorDlg.h"
#include <cmath>

// RGB -> H 转换表
int HLUT[256][256][256];    //RGB-H 转换表，S,I值分别用公式计算

// 机器人形状坐标
int robot_xy[361][12][2];             //机器人方向图像关键点坐标

//功能：初始化显示区域，设置图像数据、机器人形状坐标和颜色转换表
 
DisplayDlg::DisplayDlg(QWidget *parent)
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

    // 初始化RGB-H转换表
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

    // 初始化定时器
    m_grabTimer = new QTimer(this);
    connect(m_grabTimer, &QTimer::timeout, this, &DisplayDlg::onTimer);

    // 初始化帧率更新定时器
    fpsTimer = new QTimer(this);
    connect(fpsTimer, &QTimer::timeout, this, &DisplayDlg::updateFPS);
    fpsTimer->start(500); // 每500ms更新一次，与MFC版本保持一致

    // 加载场地图像
    m_groundImage.load("resources/ground.bmp");
    if (m_groundImage.isNull()) {
        // 如果图像加载失败，创建一个默认的绿色场地
        m_groundImage = QImage(DISPLAY_W, DISPLAY_H, QImage::Format_RGB32);
        m_groundImage.fill(QColor(0, 128, 0)); // 绿色
    }
}

// 功能：从摄像头获取一帧图像并显示
 
void DisplayDlg::ShowSingle()
{
    if (GrabSingle()) {
        QImage image(m_pDispBitmap, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(image.rgbSwapped());
        displayLabel->setPixmap(pixmap);
    }
    m_status = STATUS::Display;
}

//功能：启动定时器，持续从摄像头获取图像并显示
 
void DisplayDlg::ShowDynamic()
{
    if (m_status != STATUS::Game) {
        if (m_status == STATUS::RunTest || m_status == STATUS::Prepare || m_status == STATUS::Stop) {
            Stop();
        }
        m_status = STATUS::Display;
        // 立即获取并显示一张图像，确保切换标签页时能立即看到摄像头图像
        if (GrabSingle()) {
            QImage image(m_pDispBitmap, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
            QPixmap pixmap = QPixmap::fromImage(image.rgbSwapped());
            displayLabel->setPixmap(pixmap);
            m_DisplayWatch.start();
        }
        m_grabTimer->start(500); // 与MFC版本TIMER_SPACE_NUM保持一致
    }
}

/**
 * @brief 显示车号
 * 功能：加载并显示车号图像
 */
void DisplayDlg::ShowCarNum()
{
    Camera *pCamera = Camera::GetInstance();
    if (pCamera->IsGrabbing()) {
        this->Stop();
    }
    m_status = STATUS::Stop;
    // 加载并显示车号图像
    QImage carNumImage("resources/carnum.bmp");
    if (!carNumImage.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(carNumImage);
        displayLabel->setPixmap(pixmap);
    } else {
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
    unsigned char *pOrigin = m_pDispSingle;
    unsigned char *pTest = m_pTestBitmap;

    int i, j, R, G, B, H, S, I;
    if (HSI[object][1] > HSI[object][0])
    {
        for (j = 0; j < DISPLAY_H; j++)
            for (i = 0; i < DISPLAY_W; i++) {
                R = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2);
                G = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1);
                B = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0);
                H = 10 * HLUT[R][G][B];
                S = int(100 * (1 - 3.0 * MIN(R, G, B, 3) / (R + G + B)));
                I = int((R + G + B) / 3);
                if (H >= HSI[object][0] && H <= HSI[object][1] && S >= HSI[object][2] && S <= HSI[object][3] && I >= HSI[object][4] && I <= HSI[object][5])
                {
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2) = *(pOrigin + (i + (DISPLAY_H - j) * DISPLAY_W) * 3 + 2);
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1) = *(pOrigin + (i + (DISPLAY_H - j) * DISPLAY_W) * 3 + 1);
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0) = *(pOrigin + (i + (DISPLAY_H - j) * DISPLAY_W) * 3 + 0);
                }
                else
                {
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2) = 255;
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1) = 255;
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0) = 255;
                }
            }
    }
    else if (HSI[object][1] < HSI[object][0])
    {
        for (j = 0; j < DISPLAY_H; j++)
            for (i = 0; i < DISPLAY_W; i++) {
                R = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2);
                G = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1);
                B = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0);
                H = 10 * HLUT[R][G][B];
                S = int(100 * (1 - 3.0 * MIN(R, G, B, 3) / (R + G + B)));
                I = int((R + G + B) / 3);
                if (H >= HSI[object][0] || H <= HSI[object][1] && S >= HSI[object][2] && S <= HSI[object][3] && I >= HSI[object][4] && I <= HSI[object][5])
                {
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2) = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2);
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1) = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1);
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0) = *(pOrigin + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0);
                }
                else
                {
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 2) = 255;
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 1) = 255;
                    *(pTest + (i + (DISPLAY_H - 1 - j) * DISPLAY_W) * 3 + 0) = 255;
                }
            }
    }

    QImage image(m_pTestBitmap, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image.rgbSwapped());
    displayLabel->setPixmap(pixmap);
    delete m_pTestBitmap;
}

// 功能：启动测试模式，定时获取并处理图像
 
void DisplayDlg::ShowRunTest(bool ImageSeg)
{
    if (ImageSeg)
        m_status = STATUS::RunTestSeg;
    else
        m_status = STATUS::RunTest;
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
    m_pIdentify = m_pDispSingle;
    IdentifyAll();
    m_status = STATUS::Prepare;
    update();
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
        bool needWait = false;
        if (m_status == STATUS::Game || m_status == STATUS::RunTest || m_status == STATUS::Display || m_status == STATUS::RunTestSeg) {
            m_status = STATUS::PrepareStop;
            needWait = true;
            m_grabTimer->stop();
        }
        if (needWait) {
        }
        m_status = STATUS::Stop;
    }
}

//功能：设置当前操作状态
 
void DisplayDlg::SelectSetStatus(SET_STATUS s)
{
    m_setStatus = s;
}

// 功能：绘制足球场背景和机器人
 
void DisplayDlg::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // 只在比赛相关状态下绘制足球场背景和机器人
    if (m_status == STATUS::Game || m_status == STATUS::Prepare) {
        DrawAll(&painter);
    }
    // 其他状态不绘制背景，保持displayLabel的内容
}

// 功能：处理颜色设置时的鼠标拖拽操作
 
void DisplayDlg::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    if (m_setStatus == SET_STATUS::COLOR_SET && (event->buttons() & Qt::LeftButton)) {
        QPainter painter(displayLabel);
        painter.setPen(QPen(Qt::red, 1));
        painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        painter.drawRect(m_Rect);
        m_Rect.setRight(pos.x());
        m_Rect.setBottom(pos.y());
        painter.drawRect(m_Rect);
    }
}

//功能：处理颜色设置时的鼠标按下操作
 
void DisplayDlg::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    if (m_setStatus == SET_STATUS::COLOR_SET) {
        m_Rect.setLeft(pos.x());
        m_Rect.setTop(pos.y());
        m_Rect.setRight(pos.x() + 1);
        m_Rect.setBottom(pos.y() + 1);
        QPainter painter(displayLabel);
        painter.setPen(QPen(Qt::red, 1));
        painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        painter.drawRect(m_Rect);
    }
}

//功能：定时获取并处理图像
 
void DisplayDlg::onTimer()
{
    if (m_status == STATUS::Display || m_status == STATUS::RunTest || m_status == STATUS::RunTestSeg)
    {
        if (GrabSingle())
        {
            ProcessImage(m_pDispBitmap);
            update();
        }
    }
    else if (m_status == STATUS::Game)
    {
        if (GrabSingle())
        {
            ProcessImage(m_pDispBitmap);
            update();
        }
    }
}

//功能：计算并显示实时帧率
 
void DisplayDlg::updateFPS()
{
    double avg = m_DisplayAvg.Avg();
    m_fps = avg == 0 ? 0.0 : 1.0 / avg;
    m_DisplayAvg.Reset();
    fpsLabel->setText(QString("FPS: %1").arg(m_fps, 0, 'f', 2));
}

//功能：根据当前状态处理图像并显示
 
void DisplayDlg::ProcessImage(unsigned char *pBmp)
{
    // 使用Camera类的ConvertBitmap方法，与MFC版本保持一致
    Camera *pCamera = Camera::GetInstance();
    pCamera->ConvertBitmap(m_pDispBitmap, pBmp, DISPLAY_W, DISPLAY_H);

    switch (m_status) {
    case STATUS::Display:
        {
            QImage image(m_pDispBitmap, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
            QPixmap pixmap = QPixmap::fromImage(image.rgbSwapped());
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
        }
        break;
    case STATUS::RunTestSeg:
        {
            this->StartTest();
            QImage image(m_pDispBitmap, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
            QPixmap pixmap = QPixmap::fromImage(image.rgbSwapped());
            displayLabel->setPixmap(pixmap);
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

//功能：从摄像头获取一帧图像
 
bool DisplayDlg::GrabSingle()
{
    Camera *pCamera = Camera::GetInstance();
    if (pCamera->IsGrabbing()) {
        pCamera->GrabOne(m_pDispBitmap);
        return true;
    }
    return false;
}

//功能：从栈中弹出一个坐标点
 
bool DisplayDlg::pop(int &x, int &y)
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
 
void DisplayDlg::RGBToHS(int m, int n, unsigned char *P, int &H, int &S, int &I)
{
    int R, G, B;
    int index = (n * m_ImageSize.width() + m) * 3;
    R = *(P + index + 2);
    G = *(P + index + 1);
    B = *(P + index + 0);
    H = 10 * HLUT[R][G][B];
    S = int(100 * (1 - 3.0 * MIN(R, G, B, 3) / (R + G + B)));
    I = (int)(R + G + B) / 3;
}

// 功能：绘制足球场背景、机器人和足球
 
void DisplayDlg::DrawAll(QPainter *painter)
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
 
void DisplayDlg::DrawOpp(QPainter *painter)
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

void DisplayDlg::DrawBall(QPainter *painter)
{
    painter->setPen(QPen(Qt::red, 1));
    painter->setBrush(QBrush(Qt::red));

    if (ballInfor.found)
    {
        int x = (int)(ballInfor.x * 2.5) + 45;
        int y = (int)(ballInfor.y * 2.5) + 15;
        painter->drawEllipse(x - 4, y - 4, 8, 8);
    }
}

//功能：绘制己方机器人
 
void DisplayDlg::DrawRobot(QPainter *painter)
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
int DisplayDlg::screenBuffer(int m, int n, unsigned char *P)
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
    ColorDlg *pColorDlg = ColorDlg::getInstance();
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
 
int DisplayDlg::JudgeColor(int a, int b, int c)
{
    int obj = -1;
    int H, S, I;
    int minobj = 100;
    ColorDlg *pColorDlg = ColorDlg::getInstance();
    const int(*HSIThreshold)[6] = pColorDlg->getHSIThreshold();
    for (int i = 0; i <= 10; i++)
    {
        I = (a + b + c) / 3;
        S = (100 * (abs(a - b) + abs(b - c) + abs(c - a))) / (2 * (a + b + c));
        if (S < 10)
            S = 0;
        if (I < 20)
        {
            if (S <= 20 && S >= 15)
            {
                obj = 10;
                return obj;
            }
            else
                return -1;
        }
        for (int j = 0; j < 6; j++)
        {
            if (HSIThreshold[j][1] > HSIThreshold[j][0])
            {
                if (H >= HSIThreshold[j][0] && H <= HSIThreshold[j][1] &&
                    S >= HSIThreshold[j][2] && S <= HSIThreshold[j][3] &&
                    I >= HSIThreshold[j][4] && I <= HSIThreshold[j][5])
                {
                    if (minobj > j)
                    {
                        minobj = j;
                    }
                }
            }
            else if (HSIThreshold[j][1] < HSIThreshold[j][0])
            {
                if ((H >= HSIThreshold[j][0] || H <= HSIThreshold[j][1]) &&
                    S >= HSIThreshold[j][2] && S <= HSIThreshold[j][3] &&
                    I >= HSIThreshold[j][4] && I <= HSIThreshold[j][5])
                {
                    if (minobj > j)
                    {
                        minobj = j;
                    }
                }
            }
        }
    }
    obj = minobj;
    return obj;
}

// 功能：查找符合目标对象颜色的像素
 
bool DisplayDlg::FindPixel(int object, int m, int n, unsigned char *P)
{
    int H, S, I;
    RGBToHS(m, n, P, H, S, I);
    return JudgePixel(object, H, S, I);
}

// 功能：使用LUT搜索并识别目标对象
 
bool DisplayDlg::IdentifySearchLUT(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char *pStart)
{
    int x, y;
    int startindex, endindex;
    int count = 0;
    int color = 0;
    int m = m_ImageSize.width();
    int n = m_ImageSize.height();
    emptyStack();
    startindex = Starty * m + Startx;
    endindex = startindex;
    if (pStart[startindex] != 0)
        return false;
    push(Startx, Starty);
    while (pop(x, y))
    {
        startindex = y * m + x;
        pStart[startindex] = 255;
        count++;

        if (x < m_xLeft + 4)
        {
            m_xLeft = x - 4;
            color++;
        }
        if (x > m_xRight - 4)
        {
            m_xRight = x + 4;
            color++;
        }
        if (y < m_yTop + 4)
        {
            m_yTop = y - 4;
            color++;
        }
        if (y > m_yBottom - 4)
        {
            m_yBottom = y + 4;
            color++;
        }

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
    if (count < SizeMin || count > SizeMax || color < 2)
    {
        return false;
    }
    else
    {
        return true;
    }
}

//功能：启动目标识别

void DisplayDlg::StartTest()
{
    IdentifyAll();
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

    unsigned char *m_pTestBitmap = new unsigned char[m * n * 3];
    memcpy(m_pTestBitmap, m_pDispBitmap, m * n * 3);
    unsigned char *pTest = m_pTestBitmap;

    ColorDlg *pColorDlg = ColorDlg::getInstance();
    const int(*HSIThreshold)[6] = pColorDlg->getHSIThreshold();

    for (j = 0; j < n; j++)
    {
        for (i = 0; i < m; i++)
        {
            RGBToHS(i, j, pTest, H, S, I);

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

                if (IdentifySearchLUT(0, i, j, 30, 200, pTest))
                {
                    int x = (m_xLeft + m_xRight) / 2;
                    int y = (m_yTop + m_yBottom) / 2;
                    ballInfor.x = (double)(x - 45) / 2.5;
                    ballInfor.y = (double)(y - 15) / 2.5;
                    ballInfor.found = true;
                    ballInfor.theta = 0;
                    ObjectFound[10] = true;
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

    delete m_pTestBitmap;
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

void DisplayDlg::IdentiRobo(int ObjectCount)
{
    int i, j;
    int m = m_ImageSize.width();
    int n = m_ImageSize.height();
    int index;
    int H, S, I;
    int xLeftTem, xRightTem, yTopTem, yBottomTem;
    int robotNum = 0;

    for (i = 0; i < MAX_ROBOT_NUM; i++)
    {
        robotInfor[i].found = false;
        OpprobotInfor[i].found = false;
    }

    unsigned char *m_pTestBitmap = new unsigned char[m * n * 3];
    memcpy(m_pTestBitmap, m_pDispBitmap, m * n * 3);
    unsigned char *pTest = m_pTestBitmap;

    ColorDlg *pColorDlg = ColorDlg::getInstance();
    const int(*HSIThreshold)[6] = pColorDlg->getHSIThreshold();

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

                    if (ObjectCount == 1)
                    {
                        if (robotNum < MAX_ROBOT_NUM)
                        {
                            robotInfor[robotNum].x = (double)(x - 45) / 2.5;
                            robotInfor[robotNum].y = (double)(y - 15) / 2.5;
                            robotInfor[robotNum].found = true;
                            robotInfor[robotNum].theta = 0;
                            robotInfor[robotNum].num = robotNum;
                            ObjectFound[robotNum] = true;
                            robotNum++;
                        }
                    }
                    else if (ObjectCount == 2)
                    {
                        if (m_IdenOp && robotNum < MAX_ROBOT_NUM)
                        {
                            OpprobotInfor[robotNum].x = (double)(x - 45) / 2.5;
                            OpprobotInfor[robotNum].y = (double)(y - 15) / 2.5;
                            OpprobotInfor[robotNum].found = true;
                            OpprobotInfor[robotNum].num = robotNum;
                            robotNum++;
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

    delete m_pTestBitmap;
}

// 功能：根据位置变化查找机器人ID
 
int DisplayDlg::FindRobotID(QPoint RP1, QPoint RP2)
{
    int i;
    double theta1 = 0, theta2 = 0;
    double deltx, delty;

    for (i = 0; i < MAX_ROBOT_NUM; i++)
    {
        if (robotBk[i].found)
        {
            deltx = robotInfor[i].x - robotBk[i].x;
            delty = robotInfor[i].y - robotBk[i].y;
            theta2 = atan2(delty, deltx) * 180 / 3.1415926;
            if (theta2 < 0) theta2 += 360;
            theta1 = robotBk[i].theta;
            if (abs((int)(theta1 - theta2)) < 90)
            {
                return i;
            }
        }
    }
    return -1;
}

//功能：根据距离查找机器人ID
 
int DisplayDlg::FindRobotIDD(QPoint RP1, QPoint RP2)
{
    int i;
    double deltx, delty;

    for (i = 0; i < MAX_ROBOT_NUM; i++)
    {
        if (robotInfor[i].found)
        {
            deltx = robotInfor[i].x - robotBk[i].x;
            delty = robotInfor[i].y - robotBk[i].y;
            m_theta = atan2(delty, deltx);
            m_Length = sqrt(deltx * deltx + delty * delty);
            if (m_Length < 2)
            {
                return i;
            }
        }
    }
    return -1;
}

// 功能：查找黑色区域
 
bool DisplayDlg::FindBlackID(int m, int n, int Num)
{
    int i;
    int R, G, B;
    int index = (n * m_ImageSize.width() + m) * 3;
    R = m_pDispBitmap[index + 2];
    G = m_pDispBitmap[index + 1];
    B = m_pDispBitmap[index + 0];
    int I = (R + G + B) / 3;
    int S = (100 * (abs(R - G) + abs(G - B) + abs(B - R))) / (2 * (R + G + B));

    if (S < 20 && I < 30)
    {
        BlackSum[Num]++;
        return true;
    }
    else
    {
        return false;
    }
}

// 功能：搜索对手和足球
 
bool DisplayDlg::SeachOppAndBall(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char *pStart)
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
 
bool DisplayDlg::SearchTeam(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char *pStart)
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

//功能：测试识别功能
 
void DisplayDlg::IdentifyTest()
{
}

//功能：开始比赛，进行目标识别和绘制
 
void DisplayDlg::StartGame()
{
    // 比赛开始，需要进行目标识别和绘制
    // 定时器会触发onTimer，持续更新图像
}

//功能：分析矩形区域内的颜色
 
void DisplayDlg::ColorAnalyse(const QRect &rect, int yi[], std::vector<QPoint> &vecColorSet)
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
 
void DisplayDlg::ColorAnalyse(const std::vector<QPoint> &pts, int yi[], std::vector<QPoint> &vecColorSet)
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