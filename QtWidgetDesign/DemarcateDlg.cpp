/*
* 场地标定对话框源文件
* 写作人 李青
* 功能 标定界面逻辑实现，包含加载/保存场地参数、坐标撤销与标定结果确认操作
* 未完成
*/
#include "DemarcateDlg.h"
#include "DisplayDlg.h"
#include "Debug.h"

// 全局变量
Ground ground;

// 常量定义
const char* confGroundData = "ground.dat";

// 多项式拟合函数（简化版）
void gmiv(double* a, int m, int n, double* b, double* x, double* aa, double eps, double* u, double* v, int ka)
{
    // 这里实现多项式拟合算法
    // 简化版本，实际应用中需要使用完整的矩阵运算
    for (int i = 0; i < n; i++) {
        x[i] = 0.0;
    }
}

DemarcateDlg::DemarcateDlg(QWidget *parent)
    : QWidget(parent)
    , m_isSaved(true)
    , m_needResetDC(false)
    , m_pDispDlg(nullptr)
{
    // 设置大小策略为可伸缩
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 初始化边界点
    point[0].setX(0); point[0].setY(0);
    point[1].setX(0); point[1].setY(70);
    point[2].setX(-15); point[2].setY(70);
    point[3].setX(-15); point[3].setY(110);
    point[4].setX(0); point[4].setY(110);
    point[5].setX(0); point[5].setY(180);
    point[6].setX(220); point[6].setY(180);
    point[7].setX(220); point[7].setY(110);
    point[8].setX(235); point[8].setY(110);
    point[9].setX(235); point[9].setY(70);
    point[10].setX(220); point[10].setY(70);
    point[11].setX(220); point[11].setY(0);
    point[12].setX(0); point[12].setY(0);
    
    // 初始化结果图像
    m_resultImage = QImage(350, 250, QImage::Format_RGB32);
    m_resultImage.fill(Qt::black);
    
    initUI();
}

DemarcateDlg::~DemarcateDlg()
{}

void DemarcateDlg::initUI()
{
    // 设置字体为楷体，12号，加粗
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // 创建主控制布局
    QVBoxLayout *controlLayout = new QVBoxLayout();
    controlLayout->setSpacing(15);
    
    // 标题 "标定"
    QLabel *titleLabel = new QLabel("标定", this);
    titleLabel->setFont(font);
    controlLayout->addWidget(titleLabel);
    
    // 进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, DISPLAY_W);
    progressBar->setValue(0);
    controlLayout->addWidget(progressBar);
    
    // 右侧上方白色图像显示区域
    resultLabel = new QLabel(this);
    resultLabel->setFixedSize(340, 200);
    resultLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid black; }");
    controlLayout->addWidget(resultLabel, 0, Qt::AlignCenter);
    
    // 绿色进度条
    QLabel *greenBarLabel = new QLabel(this);
    greenBarLabel->setStyleSheet("QLabel { background-color: #00FF00; border: 1px solid black; }");
    greenBarLabel->setFixedHeight(20);
    controlLayout->addWidget(greenBarLabel);
    
    // 按钮组 - 第一行
    QHBoxLayout *buttonRow1Layout = new QHBoxLayout();
    buttonRow1Layout->setSpacing(20);
    
    // 刷新图像按钮
    btnFlush = new QPushButton("刷新图像", this);
    btnFlush->setFont(font);
    buttonRow1Layout->addWidget(btnFlush);
    
    // 撤销一步按钮
    btnResetOne = new QPushButton("撤销一步", this);
    btnResetOne->setFont(font);
    buttonRow1Layout->addWidget(btnResetOne);
    
    // 开始标定按钮
    btnSet = new QPushButton("开始标定", this);
    btnSet->setFont(font);
    buttonRow1Layout->addWidget(btnSet);
    
    controlLayout->addLayout(buttonRow1Layout);
    
    // 按钮组 - 第二行
    QHBoxLayout *buttonRow2Layout = new QHBoxLayout();
    buttonRow2Layout->setSpacing(20);
    
    // 重新标定按钮
    btnReset = new QPushButton("重新标定", this);
    btnReset->setFont(font);
    buttonRow2Layout->addWidget(btnReset);
    
    // 加载按钮
    btnLoad = new QPushButton("加载", this);
    btnLoad->setFont(font);
    buttonRow2Layout->addWidget(btnLoad);
    
    // 保存按钮
    btnSave = new QPushButton("保存", this);
    btnSave->setFont(font);
    buttonRow2Layout->addWidget(btnSave);
    
    controlLayout->addLayout(buttonRow2Layout);
    
    // 查看当前标定结果按钮
    btnShowRes = new QPushButton("查看当前标定结果", this);
    btnShowRes->setFont(font);
    controlLayout->addWidget(btnShowRes, 0, Qt::AlignCenter);
    
    mainLayout->addLayout(controlLayout);
    
    // 输出区域
    QLabel *outputLabel = new QLabel(this);
    outputLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid black; font-family: Consolas; font-size: 10pt; }");
    outputLabel->setFont(font);
    outputLabel->setText("输出区域:");
    outputLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    outputLabel->setFixedHeight(100);
    mainLayout->addWidget(outputLabel);
    
    // 禁用部分按钮
    btnSet->setEnabled(false);
    btnResetOne->setEnabled(false);
    
    // 连接信号槽
    connect(btnSet, SIGNAL(clicked()), this, SLOT(onButtonSet()));
    connect(btnResetOne, SIGNAL(clicked()), this, SLOT(onButtonResetOne()));
    connect(btnReset, SIGNAL(clicked()), this, SLOT(onButtonReset()));
    connect(btnLoad, SIGNAL(clicked()), this, SLOT(onButtonLoad()));
    connect(btnSave, SIGNAL(clicked()), this, SLOT(onButtonSave()));
    connect(btnFlush, SIGNAL(clicked()), this, SLOT(onButtonFlush()));
    connect(btnShowRes, SIGNAL(clicked()), this, SLOT(onButtonShowRes()));
    
    // 初始化加载
    onButtonLoad();
}

void DemarcateDlg::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    if (!m_resultImage.isNull()) {
        painter.drawImage(resultLabel->geometry(), m_resultImage);
    }
}

void DemarcateDlg::setDisplayDlg(DisplayDlg *dlg)
{
    m_pDispDlg = dlg;
}

void DemarcateDlg::PushPoint(const QPoint &pt)
{
    m_points.push_back(pt);
    if (!m_points.isEmpty()) {
        btnResetOne->setEnabled(true);
    }
    if (m_points.size() == 25) {
        btnSet->setEnabled(true);
        if (m_pDispDlg) {
            m_pDispDlg->SelectSetStatus(DisplayDlg::SET_STATUS::NONE);
        }
    }
    
    Debug::get()->print(QString("pt %1 : %2, %3").arg(m_points.size()).arg(pt.x()).arg(pt.y()).toStdWString().c_str());
}

void DemarcateDlg::onButtonSet()
{
    if (!m_pDispDlg) {
        QMessageBox::warning(this, "错误", "DisplayDlg未初始化！");
        return;
    }
    
    m_pDispDlg->SelectSetStatus(DisplayDlg::SET_STATUS::NONE);
    int ka = 26;
    double eps = 0.0000001;
    int m = 25; // 行数
    int n = 10; // 列数
    double a[250], a1[250];
    double px[10], py[10], aa[250], u[625], v[100];
    
    // 计算矩阵a的值
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            switch (i) {
            case 0:
                a[j*n + i] = 1;
                break;
            case 1:
                a[j*n + i] = (m_points[j].x() - 320);
                break;
            case 2:
                a[j*n + i] = (m_points[j].y() - 240);
                break;
            case 3:
                a[j*n + i] = (m_points[j].x() - 320) * (m_points[j].y() - 240);
                break;
            case 4:
                a[j*n + i] = (m_points[j].x() - 320) * (m_points[j].x() - 320);
                break;
            case 5:
                a[j*n + i] = (m_points[j].y() - 240) * (m_points[j].y() - 240);
                break;
            case 6:
                a[j*n + i] = (m_points[j].x() - 320) * (m_points[j].x() - 320) * (m_points[j].y() - 240);
                break;
            case 7:
                a[j*n + i] = (m_points[j].x() - 320) * (m_points[j].y() - 240) * (m_points[j].y() - 240);
                break;
            case 8:
                a[j*n + i] = (m_points[j].x() - 320) * (m_points[j].x() - 320) * (m_points[j].x() - 320);
                break;
            case 9:
                a[j*n + i] = (m_points[j].y() - 240) * (m_points[j].y() - 240) * (m_points[j].y() - 240);
                break;
            }
        }
    }
    
    for (int i = 0; i < 250; i++) {
        a1[i] = a[i];
    }
    
    // 目标坐标
    double bx[] = {
        0, 55, 110, 165, 220,
        0, 55, 165, 220,
        35, 185,
        0, 110, 220,
        35, 185,
        0, 55, 165, 220,
        0, 55, 110, 165, 220
    };
    double by[] = {
        0, 0, 0, 0, 0,
        30, 30, 30, 30,
        50, 50,
        90, 90, 90,
        130, 130,
        150, 150, 150, 150,
        180, 180, 180, 180, 180
    };
    
    for (int i = 0; i < 25; ++i) {
        bx[i] = (bx[i] - 110) * 2.56;
        by[i] = (by[i] - 90) * 2.56;
    }
    
    // 求解方程组
    gmiv(a, m, n, bx, px, aa, eps, u, v, ka);
    gmiv(a1, m, n, by, py, aa, eps, u, v, ka);
    
    // 计算场地信息
    for (int i = 0; i < DISPLAY_W; i++) {
        for (int j = 0; j < DISPLAY_H; j++) {
            int x2 = i - 320;
            int y2 = j - 240;
            int x1 = int(px[0] + px[1] * x2 + px[2] * y2 + px[3] * x2*y2 + px[4] * x2*x2 + px[5] * y2*y2 +
                px[6] * x2*x2*y2 + px[7] * x2*y2*y2 + px[8] * x2*x2*x2 + px[9] * y2*y2*y2);
            int y1 = int(py[0] + py[1] * x2 + py[2] * y2 + py[3] * x2*y2 + py[4] * x2*x2 + py[5] * y2*y2 +
                py[6] * x2*x2*y2 + py[7] * x2*y2*y2 + py[8] * x2*x2*x2 + py[9] * y2*y2*y2);
            
            // 坐标转换
            ground.groundInfo[i][j].x = (float)(x1 / 2.56 + 110);
            ground.groundInfo[i][j].y = (float)(y1 / 2.56 + 90);
            
            // 边界判断
            bool inRegion = false;
            QPolygon polygon;
            for (int k = 0; k < 13; k++) {
                polygon << point[k];
            }
            if (polygon.containsPoint(QPoint((int)(ground.groundInfo[i][j].x), (int)(ground.groundInfo[i][j].y)), Qt::OddEvenFill)) {
                ground.groundInfo[i][j].flag = 1;
            } else {
                ground.groundInfo[i][j].flag = 0;
            }
        }
        progressBar->setValue(i);
        QCoreApplication::processEvents();
    }
    
    m_needResetDC = true;
    onButtonShowRes();
    QMessageBox::information(this, "标定完成", "场地标定完成！");
    
    btnSet->setEnabled(false);
    btnResetOne->setEnabled(false);
    m_isSaved = false;
}

void DemarcateDlg::onButtonResetOne()
{
    if (!m_points.isEmpty()) {
        m_points.pop_back();
        if (m_points.isEmpty()) {
            btnResetOne->setEnabled(false);
        }
    }
    if (m_pDispDlg) {
        m_pDispDlg->ShowSingle();
    }
}

void DemarcateDlg::onButtonReset()
{
    if (m_pDispDlg) {
        m_pDispDlg->SelectSetStatus(DisplayDlg::SET_STATUS::BORDER_SET);
    }
    m_points.clear();
    progressBar->setValue(0);
    btnSet->setEnabled(false);
    btnResetOne->setEnabled(false);
    m_resultImage.fill(Qt::black);
    update();
}

void DemarcateDlg::onButtonLoad()
{
    int ret = 0;
    if (!m_isSaved) {
        ret = QMessageBox::question(this, "场地标定", "正在标定，所有的场地标定信息将丢失，是否继续？",
                                  QMessageBox::Yes | QMessageBox::No);
    }
    if (ret == QMessageBox::No) {
        return;
    }
    
    QFile file(confGroundData);
    if (file.open(QIODevice::ReadOnly)) {
        file.read((char*)&ground, sizeof(Ground));
        file.close();
        m_needResetDC = true;
        Debug::get()->print(L"场地标定信息已加载");
        m_isSaved = true;
    } else {
        QMessageBox::warning(this, "错误", "无法加载标定数据文件！");
    }
}

void DemarcateDlg::onButtonSave()
{
    QFile file(confGroundData);
    if (file.open(QIODevice::WriteOnly)) {
        file.write((char*)&ground, sizeof(Ground));
        file.close();
        m_isSaved = true;
        Debug::get()->print(L"场地标定信息已保存");
        QMessageBox::information(this, "保存完成", "标定数据已保存！");
    } else {
        QMessageBox::warning(this, "错误", "无法保存标定数据文件！");
    }
}

void DemarcateDlg::onButtonFlush()
{
    if (m_pDispDlg) {
        m_pDispDlg->ShowSingle();
    }
}

void DemarcateDlg::onButtonShowRes()
{
    if (!m_needResetDC) {
        update();
        return;
    }
    
    // 重新生成结果图像
    m_resultImage = QImage(350, 250, QImage::Format_RGB32);
    m_resultImage.fill(Qt::black);
    
    QPainter painter(&m_resultImage);
    painter.setPen(Qt::red);
    
    // 绘制标定结果
    progressBar->setValue(0);
    for (int i = 0; i < DISPLAY_W; i++) {
        for (int j = 0; j < DISPLAY_H; j++) {
            if (ground.groundInfo[i][j].flag) {
                int x = int(ground.groundInfo[i][j].x + 60);
                int y = int(ground.groundInfo[i][j].y + 40);
                if (x >= 0 && x < 350 && y >= 0 && y < 250) {
                    // 这里应该从DisplayDlg获取像素颜色，简化处理
                    painter.setPen(QColor(0, 255, 0));
                    painter.drawPoint(x, y);
                }
            }
        }
        progressBar->setValue(i);
        QCoreApplication::processEvents();
    }
    
    // 绘制边界
    QPolygon polygon;
    for (int i = 0; i < 13; i++) {
        polygon << QPoint(point[i].x() + 60, point[i].y() + 40);
    }
    painter.setPen(Qt::red);
    painter.drawPolyline(polygon);
    
    m_needResetDC = false;
    update();
}
