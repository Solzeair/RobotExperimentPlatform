// DisplayDlg.cpp : 实现文件
//

#include "DisplayDlg.h"
#include "Camera.h"
#include "Debug.h"
#include "ColorDlg.h"
#include <cmath>

// RGB -> H 转换表
int HLUT[256][256][256];    //RGB-H 转换表，S,I值分别用公式计算

// 机器人形状坐标
int robot_xy[361][12][2];             //机器人方向图像关键点坐标

DisplayDlg::DisplayDlg(QWidget *parent)
    : QWidget(parent)
    , m_ImageSize(DISPLAY_W, DISPLAY_H)
    , m_bErrorSign(false)
    , m_status(STATUS::Stop)
    , m_setStatus(SET_STATUS::NONE)
    , m_DisplayAvgCount(0)
    , m_IdenOp(true)
    , stackPointer(0)
    , m_xLeft(0)
    , m_xRight(DISPLAY_W)
    , m_yTop(0)
    , m_yBottom(DISPLAY_H)
    , patchConnect(false)
    , m_BalLo(false)
    , frameCount(0)
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
    fpsLabel->setStyleSheet("QLabel { background-color: rgba(0, 0, 0, 128); color: white; font-size: 12px; padding: 2px; }");
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
    fpsTimer->start(1000); // 每秒更新一次

    // 启动计时器
    lastTime.start();
}

void DisplayDlg::ShowSingle()
{
    if (GrabSingle()) {
        QImage image(m_pDispSingle, DISPLAY_W, DISPLAY_H, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(image.rgbSwapped());
        displayLabel->setPixmap(pixmap);
    }
}

void DisplayDlg::ShowDynamic()
{
    if (m_status != STATUS::Game) {
        if (m_status == STATUS::RunTest) {
            Stop();
        }
        m_status = STATUS::Display;
        m_grabTimer->start(33);
        m_DisplayWatch.start();
    }
}

void DisplayDlg::ShowCarNum()
{
    Camera *pCamera = Camera::GetInstance();
    if (pCamera->IsGrabbing()) {
        this->Stop();
    }
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

void DisplayDlg::ShowRunTest(bool ImageSeg)
{
    if (ImageSeg)
        m_status = STATUS::RunTestSeg;
    else
        m_status = STATUS::RunTest;
    m_grabTimer->start(33);
}

void DisplayDlg::ShowInitGame()
{
    m_status = STATUS::Prepare;
    // 清空显示区域
    displayLabel->setText("准备开始");
}

void DisplayDlg::ShowStartGame()
{
    m_status = STATUS::Game;
    StartGame();
}

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

void DisplayDlg::SelectSetStatus(SET_STATUS s)
{
    m_setStatus = s;
}

void DisplayDlg::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    DrawAll(&painter);
}

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

void DisplayDlg::updateFPS()
{
    qint64 elapsed = lastTime.elapsed();
    double fps = (frameCount * 1000.0) / elapsed;
    fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 12));
    frameCount = 0;
    lastTime.restart();
}

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
            m_DisplayWatch.restart();
        }
        break;
    case STATUS::RunTest:
        {
            this->StartTest();
            m_DisplayWatch.restart();
        }
        break;
    case STATUS::RunTestSeg:
        {
            this->IdentifyTest();
            m_DisplayWatch.restart();
        }
        break;
    case STATUS::Game:
        {
            this->StartGame();
            m_DisplayWatch.restart();
        }
        break;
    }
}

void DisplayDlg::StartGame()
{
    m_pIdentify = m_pDispBitmap;
    for (int i = 0; i < 12; i++) {
        ObjectFound[i] = false;
    }
    for (int i = 0; i < MAX_ROBOT_NUM; i++) {
        robotBk[i] = robotInfor[i];
        OpprobotBk[i] = OpprobotInfor[i];
    }
    ballBk = ballInfor;
    IdentifyAll();
    QImage image(DISPLAY_W, DISPLAY_H, QImage::Format_RGB32);
    QPainter painter(&image);
    DrawAll(&painter);
    displayLabel->setPixmap(QPixmap::fromImage(image));
}

bool DisplayDlg::GrabSingle()
{
    Camera *pCamera = Camera::GetInstance();
    if (pCamera->IsGrabbing()) {
        this->Stop();
    }
    pCamera->Close();
    pCamera->Open();
    // 这里应该实现抓取一帧的功能，与MFC版本保持一致
    // 由于Qt版本的Camera类可能没有GrabOne方法，暂时返回true
    pCamera->Close();
    return true;
}

void DisplayDlg::IdentifyTest()
{
    QImage image(DISPLAY_W, DISPLAY_H, QImage::Format_RGB32);
    QPainter painter(&image);
    painter.setPen(Qt::green);
    painter.drawText(10, 20, "识别测试");
    displayLabel->setPixmap(QPixmap::fromImage(image));
}

void DisplayDlg::StartTest()
{
    m_pIdentify = m_pDispBitmap;
    for (int i = 0; i < MAX_ROBOT_NUM; i++) {
        ObjectFound[i] = false;
        robotBk[i] = robotInfor[i];
        OpprobotBk[i] = OpprobotInfor[i];
    }
    ballBk = ballInfor;
    IdentifyAll();
    QImage image(DISPLAY_W, DISPLAY_H, QImage::Format_RGB32);
    QPainter painter(&image);
    DrawAll(&painter);
    displayLabel->setPixmap(QPixmap::fromImage(image));
}

bool DisplayDlg::FindPixel(int object, int m, int n, unsigned char *P)
{
    ColorDlg* pColorDlg = ColorDlg::getInstance();
    if (!pColorDlg) return false;
    const int(*HSIThreshold)[6] = pColorDlg->getHSIThreshold();
    int R, G, B;
    int H, S, I;
    int index = (m + (DISPLAY_H - 1 - n) * DISPLAY_W) * 3;
    R = *(P + index + 2);
    G = *(P + index + 1);
    B = *(P + index + 0);
    if (R < 0 || R > 255 || G < 0 || G > 255 || B < 0 || B > 255)
        return false;
    H = 10 * HLUT[R][G][B];
    if (H <= 0)
        return false;
    if (HSIThreshold[object][1] >= HSIThreshold[object][0])
    {
        if (H >= HSIThreshold[object][0] && H <= HSIThreshold[object][1])
        {
            S = int(100 * (1 - 3.0 * MIN(R, G, B, 3) / (R + G + B)));
            if (S >= HSIThreshold[object][2] && S <= HSIThreshold[object][3])
            {
                I = (int)((R + G + B) / 3);
                if (I >= HSIThreshold[object][4] && I <= HSIThreshold[object][5])
                    return true;
                else
                    return false;
            }
            else
                return false;
        }
        else
            return false;
    }
    else if (HSIThreshold[object][1] < HSIThreshold[object][0])
    {
        if (H >= HSIThreshold[object][0] || H <= HSIThreshold[object][1])
        {
            S = int(100 * (1 - 3.0 * MIN(R, G, B, 3) / (R + G + B)));
            if (S >= HSIThreshold[object][2] && S <= HSIThreshold[object][3])
            {
                I = (int)((R + G + B) / 3);
                if (I >= HSIThreshold[object][4] && I <= HSIThreshold[object][5])
                    return true;
                else
                    return false;
            }
            else
                return false;
        }
        else
            return false;
    }
    else
        return false;
}

bool DisplayDlg::IdentifySearchLUT(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char *pStart)
{
    int sum, sumx, sumy;
    int x, y, y1;
    bool spanLeft, spanRight;
    sum = sumx = sumy = 0;
    emptyStack();
    x = Startx;
    y = Starty;
    m_xLeft = m_xRight = x;
    m_yTop = m_yBottom = y;
    if (!push(x, y)) return 0;
    while (pop(x, y))
    {
        y1 = y;
        while (y1 >= 0) {
            if (!FindPixel(tab, x, y1, pStart))
                break;
            y1--;
        }
        y1++;
        spanLeft = spanRight = 0;
        while (y1 < DISPLAY_H)
        {
            if (!FindPixel(tab, x, y1, pStart))
                break;

            int index = (x + (DISPLAY_H - 1 - y1) * DISPLAY_W) * 3;
            *(pStart + index + 2) = 100;
            *(pStart + index + 1) = 100;
            *(pStart + index + 0) = 100;
            sum++;
            sumx = sumx + x;
            sumy = sumy + y1;
            if (x <= m_xLeft) m_xLeft = x;
            else if (x >= m_xRight) m_xRight = x;
            if (y1 <= m_yTop) m_yTop = y1;
            else if (y1 >= m_yBottom) m_yBottom = y1;
            if (!spanLeft && x > 0 && FindPixel(tab, x - 1, y1, pStart))
            {
                if (!push(x - 1, y1)) return 0;
                spanLeft = 1;
            }
            else if (spanLeft && x > 0 && !FindPixel(tab, x - 1, y1, pStart))
            {
                spanLeft = 0;
            }
            if (!spanRight && x < DISPLAY_W && FindPixel(tab, x + 1, y1, pStart))
            {
                if (!push(x + 1, y1)) return 0;
                spanRight = 1;
            }
            else if (spanRight && x < DISPLAY_W && !FindPixel(tab, x + 1, y1, pStart))
            {
                spanRight = 0;
            }
            y1++;
        }
    }

    if ((sum > SizeMin - 1) && (sum < SizeMax + 1))
    {
        if (qAbs((m_xRight - m_xLeft) - (m_yBottom - m_yTop)) > 20)
            return 0;
        m_Target[tab].setX(sumx / sum);
        m_Target[tab].setY(sumy / sum);
        return 1;
    }
    return 0;
}

void DisplayDlg::IdentifyAll()
{
    int NumOpp = 0, NumBall = 0, NumTeam = 0;
    OppInf TemOpp[30], TemBall[10], TemTeam[30], Tem;
    
    for (int i = 0; i < DISPLAY_W; i += 4) {
        for (int j = 0; j < DISPLAY_H; j += 4) {
            if (m_IdenOp && FindPixel(7, i, j, m_pIdentify)) {
                if (NumOpp < 11 && SeachOppAndBall(7, i, j, 30, 300, m_pIdentify)) {
                    if ((m_TargetN.x >= 0) && (m_TargetN.x < DISPLAY_W) && (m_TargetN.y >= 0) && (m_TargetN.y < DISPLAY_H)) {
                        TemOpp[NumOpp] = m_TargetN;
                        NumOpp++;
                    }
                }
            }
            else if (FindPixel(0, i, j, m_pIdentify)) {
                if (NumTeam < 20 && SearchTeam(0, i, j, 50, 300, m_pIdentify)) {
                    if (!patchConnect) {
                        if ((m_TargetN.x >= 0) && (m_TargetN.x < DISPLAY_W) && (m_TargetN.y >= 0) && (m_TargetN.y < DISPLAY_H)) {
                            TemTeam[NumTeam] = m_TargetN;
                            NormalTheta[NumTeam] = m_theta;
                            NumTeam++;
                        }
                    } else {
                        if ((m_TargetN1.x >= 0) && (m_TargetN1.x < DISPLAY_W) && (m_TargetN1.y >= 0) && (m_TargetN1.y < DISPLAY_H) &&
                            (m_TargetN2.x >= 0) && (m_TargetN2.x < DISPLAY_W) && (m_TargetN2.y >= 0) && (m_TargetN2.y < DISPLAY_H)) {
                            TemTeam[NumTeam] = m_TargetN1;
                            NormalTheta[NumTeam] = m_theta;
                            NumTeam++;
                            TemTeam[NumTeam] = m_TargetN2;
                            NormalTheta[NumTeam] = m_theta;
                            NumTeam++;
                        }
                    }
                }
            }
            else if (NumBall < 5 && FindPixel(6, i, j, m_pIdentify)) {
                if (SeachOppAndBall(6, i, j, 30, 300, m_pIdentify)) {
                    if ((m_TargetN.x >= 0) && (m_TargetN.x < DISPLAY_W) && (m_TargetN.y >= 0) && (m_TargetN.y < DISPLAY_H)) {
                        TemBall[NumBall] = m_TargetN;
                        NumBall++;
                    }
                }
            }
        }
    }
    
    for (int i = 0; i < NumTeam; i++) {
        TeamTarget[i].setX((int)TemTeam[i].x);
        TeamTarget[i].setY((int)TemTeam[i].y);
    }
    
    IdentiRobo(NumTeam);
    
    if (NumOpp <= MAX_ROBOT_NUM) {
        for (int k = 0; k < NumOpp; k++) {
            OpprobotInfor[k].x = TemOpp[k].x;
            OpprobotInfor[k].y = TemOpp[k].y;
            OpprobotInfor[k].found = true;
        }
    } else {
        for (int k = 0; k < NumOpp - 1; k++) {
            for (int n = k + 1; n < NumOpp; n++) {
                if (TemOpp[k].num < TemOpp[n].num) {
                    Tem = TemOpp[k];
                    TemOpp[k] = TemOpp[n];
                    TemOpp[n] = Tem;
                }
            }
        }
        for (int k = 0; k < MAX_ROBOT_NUM; k++) {
            OpprobotInfor[k].x = TemOpp[k].x;
            OpprobotInfor[k].y = TemOpp[k].y;
            OpprobotInfor[k].found = true;
        }
    }
    
    if (NumBall >= 1) {
        for (int i = 1; i < NumBall; i++) {
            if (TemBall[0].num < TemBall[i].num) {
                TemBall[0] = TemBall[i];
            }
        }
        ballInfor.x = TemBall[0].x;
        ballInfor.y = TemBall[0].y;
        ballInfor.found = true;
    } else {
        if (m_BalLo) {
            BallPosFilter();
        }
    }
}

void DisplayDlg::IdentiRobo(int ObjectCount)
{
    int i, j;
    double temptheta, OrientAngle;
    QPoint ReferPoint[4];
    bool blackID[4];
    int RobotID;
    
    for (i = 0; i < ObjectCount; i++) {
        RobotID = -1;
        temptheta = 3.1415926 / 2 - atan(0.75) - NormalTheta[i];
        ReferPoint[0].setX((int)(TeamTarget[i].x() + m_Length * cos(temptheta)));
        ReferPoint[0].setY((int)(TeamTarget[i].y() + m_Length * sin(temptheta)));
        ReferPoint[1].setX((int)(TeamTarget[i].x() + m_Length * cos(temptheta + 2 * atan(0.75))));
        ReferPoint[1].setY((int)(TeamTarget[i].y() + m_Length * sin(temptheta + 2 * atan(0.75))));
        ReferPoint[2].setX((int)(TeamTarget[i].x() + m_Length * cos(temptheta + 3.1415926)));
        ReferPoint[2].setY((int)(TeamTarget[i].y() + m_Length * sin(temptheta + 3.1415926)));
        ReferPoint[3].setX((int)(TeamTarget[i].x() + m_Length * cos(temptheta + 2 * atan(0.75) + 3.1415926)));
        ReferPoint[3].setY((int)(TeamTarget[i].y() + m_Length * sin(temptheta + 2 * atan(0.75) + 3.1415926)));
        
        for (j = 0; j < 4; j++) {
            blackID[j] = false;
        }
        
        for (j = 0; j < 4; j++) {
            blackID[j] = FindBlackID(ReferPoint[j].x(), ReferPoint[j].y(), j);
        }
        
        if (blackID[0] && blackID[1]) {
            OrientAngle = NormalTheta[i] + 3.1415926;
            RobotID = FindRobotID(ReferPoint[2], ReferPoint[3]);
        } else if (blackID[2] && blackID[3]) {
            OrientAngle = NormalTheta[i];
            RobotID = FindRobotID(ReferPoint[0], ReferPoint[1]);
        } else if (!blackID[0] && !blackID[1]) {
            OrientAngle = NormalTheta[i];
            RobotID = FindRobotIDD(ReferPoint[2], ReferPoint[3]);
        } else if (!blackID[2] && !blackID[3]) {
            OrientAngle = NormalTheta[i] + 3.1415926;
            RobotID = FindRobotIDD(ReferPoint[0], ReferPoint[1]);
        }
        
        if (RobotID >= 0 && RobotID < MAX_ROBOT_NUM) {
            robotInfor[RobotID].theta = OrientAngle * 180 / 3.1415926;
            robotInfor[RobotID].x = TeamTarget[i].x();
            robotInfor[RobotID].y = TeamTarget[i].y();
            robotInfor[RobotID].found = true;
            ObjectFound[RobotID] = true;
        }
    }
}

bool DisplayDlg::SeachOppAndBall(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char *pStart)
{
    int sum, sumx, sumy, x, y, y1;
    bool spanLeft, spanRight;
    sum = sumx = sumy = 0;
    emptyStack();
    x = Startx;
    y = Starty;
    m_xLeft = m_xRight = x;
    m_yTop = m_yBottom = y;
    if (!push(x, y)) return 0;
    while (pop(x, y))
    {
        y1 = y;
        while (FindPixel(tab, x, y1, pStart) && y1 >= 0) y1--;
        y1++;
        spanLeft = 0;
        spanRight = 0;
        while (FindPixel(tab, x, y1, pStart) && y1 < DISPLAY_H)
        {
            int index = (x + (DISPLAY_H - 1 - y1) * DISPLAY_W) * 3;
            *(pStart + index + 2) = 100;
            *(pStart + index + 1) = 100;
            *(pStart + index + 0) = 100;
            if (x <= m_xLeft) m_xLeft = x;
            else if (x >= m_xRight) m_xRight = x;
            if (y1 <= m_yTop) m_yTop = y1;
            else if (y1 >= m_yBottom) m_yBottom = y1;
            sum++;
            sumx = sumx + x;
            sumy = sumy + y1;
            if (!spanLeft && x > 0 && FindPixel(tab, x - 1, y1, pStart))
            {
                if (!push(x - 1, y1)) return 0;
                spanLeft = 1;
            }
            else if (spanLeft && x > 0 && !FindPixel(tab, x - 1, y1, pStart))
            {
                spanLeft = 0;
            }
            if (!spanRight && x < DISPLAY_W && FindPixel(tab, x + 1, y1, pStart))
            {
                if (!push(x + 1, y1)) return 0;
                spanRight = 1;
            }
            else if (spanRight && x < DISPLAY_W && !FindPixel(tab, x + 1, y1, pStart))
            {
                spanRight = 0;
            }
            y1++;
        }
    }
    if ((sum > SizeMin - 1) && (sum < SizeMax + 1))
    {
        if (tab == BALL)
        {
            if ((m_xRight - m_xLeft) < 2 || (m_xRight - m_xLeft) > 15 || (m_yBottom - m_yTop) < 2 || (m_yBottom - m_yTop) > 15)
                return 0;
        }
        m_TargetN.x = sumx / sum;
        m_TargetN.y = sumy / sum;
        return 1;
    }
    return 0;
}

bool DisplayDlg::SearchTeam(int tab, int Startx, int Starty, int SizeMin, int SizeMax, unsigned char *pStart)
{
    int sum, sumx, sumy, x, y, y1;
    bool spanLeft, spanRight;
    sum = sumx = sumy = 0;
    emptyStack();
    x = Startx;
    y = Starty;
    m_xLeft = m_xRight = x;
    m_yTop = m_yBottom = y;
    m_TargetN1.x = 0;
    m_TargetN1.y = 0;
    m_TargetN2.x = 0;
    m_TargetN2.y = 0;
    m_theta = 0;
    if (!push(x, y)) return 0;
    while (pop(x, y))
    {
        y1 = y;
        while (FindPixel(tab, x, y1, pStart) && y1 >= 0) y1--;
        y1++;
        spanLeft = 0;
        spanRight = 0;
        while (FindPixel(tab, x, y1, pStart) && y1 < DISPLAY_H)
        {
            int index = (x + (DISPLAY_H - 1 - y1) * DISPLAY_W) * 3;
            *(pStart + index + 2) = 100;
            *(pStart + index + 1) = 100;
            *(pStart + index + 0) = 100;
            if (x <= m_xLeft) m_xLeft = x;
            else if (x >= m_xRight) m_xRight = x;
            if (y1 <= m_yTop) m_yTop = y1;
            else if (y1 >= m_yBottom) m_yBottom = y1;
            sum++;
            sumx = sumx + x;
            sumy = sumy + y1;
            if (!spanLeft && x > 0 && FindPixel(tab, x - 1, y1, pStart))
            {
                if (!push(x - 1, y1)) return 0;
                spanLeft = 1;
            }
            else if (spanLeft && x > 0 && !FindPixel(tab, x - 1, y1, pStart))
            {
                spanLeft = 0;
            }
            if (!spanRight && x < DISPLAY_W && FindPixel(tab, x + 1, y1, pStart))
            {
                if (!push(x + 1, y1)) return 0;
                spanRight = 1;
            }
            else if (spanRight && x < DISPLAY_W && !FindPixel(tab, x + 1, y1, pStart))
            {
                spanRight = 0;
            }
            y1++;
        }
    }
    if ((sum > SizeMin - 1) && (sum < SizeMax + 1))
    {
        if (abs((m_xRight - m_xLeft) - (m_yBottom - m_yTop)) > 20)
            return 0;
        if ((m_xRight - m_xLeft) > 3 * (m_yBottom - m_yTop))
        {
            patchConnect = true;
            m_TargetN1.x = sumx / sum;
            m_TargetN1.y = (m_yTop + m_yBottom) / 2;
            m_TargetN2.x = sumx / sum;
            m_TargetN2.y = (m_yTop + m_yBottom) / 2;
        }
        else if ((m_yBottom - m_yTop) > 3 * (m_xRight - m_xLeft))
        {
            patchConnect = true;
            m_TargetN1.x = (m_xLeft + m_xRight) / 2;
            m_TargetN1.y = sumy / sum;
            m_TargetN2.x = (m_xLeft + m_xRight) / 2;
            m_TargetN2.y = sumy / sum;
        }
        else
        {
            patchConnect = false;
            m_TargetN.x = sumx / sum;
            m_TargetN.y = sumy / sum;
        }
        m_theta = atan2((double)(m_yBottom - m_yTop), (double)(m_xRight - m_xLeft));
        return 1;
    }
    patchConnect = false;
    return 0;
}

bool DisplayDlg::FindBlackID(int m, int n, int Num)
{
    bool IDCode;
    int ii, jj;
    int sumblack;
    int H = 0;
    int S = 0;
    int I = 0;
    sumblack = 0;
    for (jj = n - 1; jj <= n + 1; jj++)
        for (ii = m - 1; ii <= m + 1; ii++)
        {
            if (jj < 0 || jj > DISPLAY_H - 1 || ii < 0 || ii > DISPLAY_W - 1) continue;
            RGBToHS(ii, jj, m_pIdentify, H, S, I);
            if (!JudgePixel(1, H, S, I)
                && !JudgePixel(2, H, S, I)
                && (screenBuffer(ii, jj, m_pIdentify) != 12684))
            {
                sumblack++;
                if (sumblack >= 5) break;
            }
        }
    if (sumblack >= 5) IDCode = 1;
    else
    {
        IDCode = 0;
        BlackSum[Num] = sumblack;
    }
    return IDCode;
}

int DisplayDlg::FindRobotID(QPoint RP1, QPoint RP2)
{
    int roboID, RPID1, RPID2;
    int ii, jj;
    int sum1, sum2, sum0;
    int H = 0;
    int S = 0;
    int I = 0;
    sum1 = sum2 = sum0 = 0;
    for (jj = RP1.y() - 2; jj <= RP1.y() + 2; jj++)
        for (ii = RP1.x() - 2; ii <= RP1.x() + 2; ii++)
        {
            if (jj < 0 || jj > DISPLAY_H - 1 || ii < 0 || ii > DISPLAY_W - 1) continue;
            RGBToHS(ii, jj, m_pIdentify, H, S, I);
            if (JudgePixel(1, H, S, I))
                sum1++;
            else if (JudgePixel(2, H, S, I))
                sum2++;
            else if (screenBuffer(ii, jj, m_pIdentify) != 12684)
                sum0++;
        }
    RPID1 = JudgeColor(sum1, sum2, sum0);
    sum1 = sum2 = sum0 = 0;
    for (jj = RP2.y() - 2; jj <= RP2.y() + 2; jj++)
        for (ii = RP2.x() - 2; ii <= RP2.x() + 2; ii++)
        {
            if (jj < 0 || jj > DISPLAY_H - 1 || ii < 0 || ii > DISPLAY_W - 1) continue;
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

int DisplayDlg::FindRobotIDD(QPoint RP1, QPoint RP2)
{
    int roboID, RPID1, RPID2;
    int ii, jj;
    int sum1, sum2, sum0;
    int H = 0;
    int S = 0;
    int I = 0;
    sum1 = sum2 = sum0 = 0;
    for (jj = RP1.y() - 2; jj <= RP1.y() + 2; jj++)
        for (ii = RP1.x() - 2; ii <= RP1.x() + 2; ii++)
        {
            if (jj < 0 || jj > DISPLAY_H - 1 || ii < 0 || ii > DISPLAY_W - 1) continue;
            RGBToHS(ii, jj, m_pIdentify, H, S, I);
            if (JudgePixel(1, H, S, I))
                sum1++;
            else if (JudgePixel(2, H, S, I))
                sum2++;
            else if (screenBuffer(ii, jj, m_pIdentify) != 12684)
                sum0++;
        }
    RPID1 = JudgeColor(sum1, sum2, sum0);
    sum1 = sum2 = sum0 = 0;
    for (jj = RP2.y() - 2; jj <= RP2.y() + 2; jj++)
        for (ii = RP2.x() - 2; ii <= RP2.x() + 2; ii++)
        {
            if (jj < 0 || jj > DISPLAY_H - 1 || ii < 0 || ii > DISPLAY_W - 1) continue;
            RGBToHS(ii, jj, m_pIdentify, H, S, I);
            if (JudgePixel(1, H, S, I))
                sum1++;
            else if (JudgePixel(2, H, S, I))
                sum2++;
            else if (screenBuffer(ii, jj, m_pIdentify) != 12684)
                sum0++;
        }
    RPID2 = JudgeColor(sum1, sum2, sum0);
    if (RPID1 == 1 && RPID2 == 0) roboID = 8;
    else if (RPID1 == 0 && RPID2 == 1) roboID = 9;
    else if (RPID1 == 2 && RPID2 == 0) roboID = 10;
    else roboID = -1;
    return roboID;
}

void DisplayDlg::BallPosFilter()
{
    if (ballInfor.x > ballBk.x)
    {
        ballBk.x = ballInfor.x;
        ballBk.y = ballInfor.y;
    }
    else
    {
        ballInfor.x = ballBk.x;
        ballInfor.y = ballBk.y;
    }
}

int DisplayDlg::screenBuffer(int m, int n, unsigned char *P)
{
    int r, g, b, data;
    int index = (m + (DISPLAY_H - 1 - n) * DISPLAY_W) * 3;
    r = *(P + index + 2) / 8;
    g = *(P + index + 1) / 8;
    b = *(P + index + 0) / 8;
    data = ((r & 0x1f) << 10 | (g & 0x1f) << 5 | (b & 0x1f));
    return data;
}

bool DisplayDlg::JudgePixel(int object, int H, int S, int I)
{
    ColorDlg* pColorDlg = ColorDlg::getInstance();
    if (!pColorDlg) return false;
    const int(*HSIThreshold)[6] = pColorDlg->getHSIThreshold();
    if (H == 0)
        return false;
    else
        if (HSIThreshold[object][1] >= HSIThreshold[object][0])
        {
            if (H >= HSIThreshold[object][0] && H <= HSIThreshold[object][1] && S >= HSIThreshold[object][2] && S <= HSIThreshold[object][3] && I >= HSIThreshold[object][4] && I <= HSIThreshold[object][5])
                return true;
            else
                return false;
        }
        else
            if (HSIThreshold[object][1] < HSIThreshold[object][0])
            {
                if (H >= HSIThreshold[object][0] || H <= HSIThreshold[object][1] && S >= HSIThreshold[object][2] && S <= HSIThreshold[object][3] && I >= HSIThreshold[object][4] && I <= HSIThreshold[object][5])
                    return true;
                else
                    return false;
            }
            else
                return false;
}

int DisplayDlg::JudgeColor(int a, int b, int c)
{
    if (a > b)
        return 0;
    else
        return 1;
}

void DisplayDlg::emptyStack()
{
    int x, y;
    while (pop(x, y));
}

bool DisplayDlg::push(int x, int y)
{
    if (stackPointer < StackSize - 1)
    {
        stackPointer++;
        stackx[stackPointer] = x;
        stacky[stackPointer] = y;
        return 1;
    }
    else
    {
        return 0;
    }
}

bool DisplayDlg::pop(int &x, int &y)
{
    if (stackPointer > 0)
    {
        x = stackx[stackPointer];
        y = stacky[stackPointer];
        stackPointer--;
        return 1;
    }
    else
    {
        return 0;
    }
}

void DisplayDlg::RGBToHS(int m, int n, unsigned char *P, int &H, int &S, int &I)
{
    int R, G, B;
    int index = (m + (DISPLAY_H - 1 - n) * DISPLAY_W) * 3;
    R = *(P + index + 2);
    G = *(P + index + 1);
    B = *(P + index + 0);
    H = 10 * HLUT[R][G][B];
    S = int(100 * (1 - 3.0 * MIN(R, G, B, 3) / (R + G + B)));
    I = (int)(R + G + B) / 3;
}

void DisplayDlg::DrawAll(QPainter *painter)
{
    // 绘制机器人
    DrawRobot(painter);
    
    // 绘制对手
    DrawOpp(painter);
    
    // 绘制球
    DrawBall(painter);
}

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

void DisplayDlg::DrawBall(QPainter *painter)
{
    painter->setPen(QPen(QColor(255, 128, 0), 1));
    painter->setBrush(QBrush(QColor(255, 128, 0)));
    
    if (ballInfor.found)
    {
        int x = (int)(ballInfor.x * 2.5) + 45;
        int y = (int)(ballInfor.y * 2.5) + 15;
        painter->drawEllipse(x - 4, y - 4, 8, 8);
    }
}

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

void DisplayDlg::ColorAnalyse(const QRect &rect, int yi[], std::vector<QPoint> &vecColorSet)
{
    int i, j;
    int R, G, B;
    double S, H;
    int I;

    unsigned char* pRGB;
    this->ShowSingle();
    pRGB = m_pDispSingle;
    QRect rect1;

    rect1.setLeft((m_Rect.left() + 1) + (int)((double)(m_Rect.right() - m_Rect.left() - 1) / 250.0 * rect.left()));
    rect1.setRight((m_Rect.left() + 1) + (int)((double)(m_Rect.right() - m_Rect.left() - 1) / 250.0 * rect.right()));
    rect1.setTop((m_Rect.top() + 1) + (int)((double)(m_Rect.bottom() - m_Rect.top() - 1) / 220.0 * rect.top()));
    rect1.setBottom((m_Rect.top() + 1) + (int)((double)(m_Rect.bottom() - m_Rect.top() - 1) / 220.0 * rect.bottom()));
    
    for (j = rect1.top(); j < rect1.bottom(); j++)
    {
        for (i = rect1.left(); i < rect1.right(); i++)
        {
            int index = (j * DISPLAY_W + i) * 3;
            R = pRGB[index + 2];
            G = pRGB[index + 1];
            B = pRGB[index];
            S = 1 - 3.0 * MIN(R, G, B, 3) / (R + G + B);
            H = HLUT[R][G][B] * 3.1415926 / 180.0;
            I = (int)(R + G + B) / 3;
            yi[I]++;
            vecColorSet.push_back(QPoint(int(100 * S * cos(H) + 100), int(100 * S * sin(H) + 100)));
        }
    }
}

void DisplayDlg::ColorAnalyse(const std::vector<QPoint> &pts, int yi[], std::vector<QPoint> &vecColorSet)
{
    if (pts.size() < 3)
    {
        return;
    }
    int i, j;
    int R, G, B;
    double S, H;
    int I;
    unsigned char* pRGB;
    this->GrabSingle();
    pRGB = m_pDispSingle;
    
    for (j = 0; j < DISPLAY_H; j++)
    {
        for (i = 0; i < DISPLAY_W; i++)
        {
            int index = (j * DISPLAY_W + i) * 3;
            R = pRGB[index + 2];
            G = pRGB[index + 1];
            B = pRGB[index];
            S = 1 - 3.0 * MIN(R, G, B, 3) / (R + G + B);
            H = HLUT[R][G][B] * 3.1415926 / 180.0;
            I = (int)(R + G + B) / 3;
            yi[I]++;
            vecColorSet.push_back(QPoint(int(100 * S * cos(H) + 100), int(100 * S * sin(H) + 100)));
        }
    }
}

int DisplayDlg::MIN(int R, int G, int B, int N)
{
    int temp;
    if (N == 3)
    {
        if (R <= G && R <= B)
            temp = R;
        else if (G <= R && G <= B)
            temp = G;
        else if (B <= R && B <= G)
            temp = B;
    }
    else if (N == 2)
    {
        if (R <= G) temp = R;
        else temp = G;
    }
    return temp;
}
