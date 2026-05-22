/*
 * DemarcateDlg.cpp  –  场地标定对话框实现
 * 作者 : 李青  (修订版)
 *
 * 更改摘要
 * ──────────────────
 * 1. 使用真正的最小二乘求解器（带部分主元的高斯消元法）
 *    替换了原来的空壳 gmiv()。
 *    原来的 gmiv() 将所有解系数设为 0，所以从未生成过标定数据。
 *
 * 2. 添加了透视校正功能（applyPerspectiveCorrection）。
 *    如果操作员还提供了 4 个场地角点的像素坐标，
 *    则在校值拟合之前，将原始相机画面变换为俯视矩形。
 *    这可以消除桶形/倾斜畸变，使映射更加准确。
 *
 * 3. 移除了与 utili.h 冲突的重复 Ground / GroundInfo 结构体定义
 *    （flag 字段的 char vs bool 类型冲突）。
 *    utili.h 现在是唯一的权威定义。
 *
 */

#include "DemarcateDlg.h"
#include "DisplayDlg.h"
#include "Debug.h"

#include <cmath>
#include <cstring>
#include <algorithm>

 // OpenCV 头文件 – 仅用于透视变换
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

// ---------------------------------------------------------------
// 全局地面标定表（单一定义）
// ---------------------------------------------------------------
Ground ground;

// ---------------------------------------------------------------
// 25 个控制点的目标场地坐标（厘米）
// 行从上到下，列从左到右。
// 与原始布局完全一致。
// ---------------------------------------------------------------
static const double kBx[CALIB_POINT_COUNT] = {
    0, 55, 110, 165, 220,
    0, 55, 165, 220,
    35, 185,
    0, 110, 220,
    35, 185,
    0, 55, 165, 220,
    0, 55, 110, 165, 220
};
static const double kBy[CALIB_POINT_COUNT] = {
    0, 0, 0, 0, 0,
    30, 30, 30, 30,
    50, 50,
    90, 90, 90,
    130, 130,
    150, 150, 150, 150,
    180, 180, 180, 180, 180
};

// 场地中心，单位为厘米（用于内部缩放坐标）
static const double kCx = 110.0;  // 220 厘米宽度的一半
static const double kCy = 90.0;  // 180 厘米高度的一半
static const double kScale = 2.56; // 缩放因子（原始代码常数）

// ═══════════════════════════════════════════════════════════════
// 构造函数 / 析构函数
// ═══════════════════════════════════════════════════════════════

DemarcateDlg::DemarcateDlg(QWidget* parent)
    : QWidget(parent)
    , m_isSaved(true)
    , m_needResetDC(false)
    , m_pDispDlg(nullptr)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ── 场地边界多边形（13 个顶点，单位厘米）──────────────────
    // 与原始版本保持不变；定义场地的有效区域。
    point[0] = QPoint(0, 0);
    point[1] = QPoint(0, 70);
    point[2] = QPoint(-15, 70);
    point[3] = QPoint(-15, 110);
    point[4] = QPoint(0, 110);
    point[5] = QPoint(0, 180);
    point[6] = QPoint(220, 180);
    point[7] = QPoint(220, 110);
    point[8] = QPoint(235, 110);
    point[9] = QPoint(235, 70);
    point[10] = QPoint(220, 70);
    point[11] = QPoint(220, 0);
    point[12] = QPoint(0, 0);

    // ── 结果预览图像（黑色背景）────────────────────────────────
    // 尺寸与 onButtonShowRes() 中的俯视输出图像一致：
    // 场地宽 250 cm × 2.4 px/cm + 20 px 边距 = 620 px
    // 场地高 180 cm × 2.4 px/cm + 20 px 边距 = 452 px
    m_resultImage = QImage(620, 452, QImage::Format_RGB32);
    m_resultImage.fill(Qt::black);

    initUI();
}

DemarcateDlg::~DemarcateDlg() {}

// ═══════════════════════════════════════════════════════════════
// UI 初始化
// ═══════════════════════════════════════════════════════════════

void DemarcateDlg::initUI()
{
    QFont font("楷体", 12, QFont::Bold);
    setFont(font);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 0, 20, 20);
    mainLayout->setSpacing(15);

    QVBoxLayout* controlLayout = new QVBoxLayout();
    controlLayout->setSpacing(12);

    // 标题标签
    QLabel* titleLabel = new QLabel("标定", this);
    titleLabel->setFont(font);
    controlLayout->addWidget(titleLabel);
    controlLayout->setAlignment(titleLabel, Qt::AlignTop);

    // 结果预览区域
    // 最小尺寸足以容纳俯视图（620×452），允许随面板拉伸放大。
    resultLabel = new QLabel(this);
    resultLabel->setMinimumSize(560, 400);
    resultLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setStyleSheet("QLabel { background-color: black; border: 1px solid #555555; }");
    controlLayout->addWidget(resultLabel, 0, Qt::AlignCenter);

    // 进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, DISPLAY_W);
    progressBar->setValue(0);
    controlLayout->addWidget(progressBar);

    // 绿色状态条
    m_statusBar = new QLabel(this);
    m_statusBar->setStyleSheet("QLabel { background-color: #FF0000; border: 1px solid black; }");  // 初始红色
    m_statusBar->setFixedHeight(20);
    controlLayout->addWidget(m_statusBar);

    // ── 按钮行 1 ─────────────────────────────────────────────
    QHBoxLayout* buttonRow1Layout = new QHBoxLayout();
    buttonRow1Layout->setSpacing(20);

    btnFlush = new QPushButton("刷新图像", this);
    btnFlush->setFont(font);
    buttonRow1Layout->addWidget(btnFlush);

    btnResetOne = new QPushButton("撤销一步", this);
    btnResetOne->setFont(font);
    buttonRow1Layout->addWidget(btnResetOne);

    btnSet = new QPushButton("开始标定", this);
    btnSet->setFont(font);
    buttonRow1Layout->addWidget(btnSet);

    controlLayout->addLayout(buttonRow1Layout);

    // ── 按钮行 2 ─────────────────────────────────────────────
    QHBoxLayout* buttonRow2Layout = new QHBoxLayout();
    buttonRow2Layout->setSpacing(20);

    btnReset = new QPushButton("重新标定", this);
    btnReset->setFont(font);
    buttonRow2Layout->addWidget(btnReset);

    btnLoad = new QPushButton("加载", this);
    btnLoad->setFont(font);
    buttonRow2Layout->addWidget(btnLoad);

    btnSave = new QPushButton("保存", this);
    btnSave->setFont(font);
    buttonRow2Layout->addWidget(btnSave);

    controlLayout->addLayout(buttonRow2Layout);

    // 显示结果按钮
    btnShowRes = new QPushButton("查看当前标定结果", this);
    btnShowRes->setFont(font);
    controlLayout->addWidget(btnShowRes, 0, Qt::AlignCenter);

    mainLayout->addLayout(controlLayout);
    mainLayout->addStretch();
    mainLayout->setAlignment(Qt::AlignTop);

    // ── 初始按钮状态 ─────────────────────────────────────────
    btnSet->setEnabled(false);
    btnResetOne->setEnabled(false);

    // ── 信号-槽连接 ───────────────────────────────────────────
    connect(btnSet, SIGNAL(clicked()), this, SLOT(onButtonSet()));
    connect(btnResetOne, SIGNAL(clicked()), this, SLOT(onButtonResetOne()));
    connect(btnReset, SIGNAL(clicked()), this, SLOT(onButtonReset()));
    connect(btnLoad, SIGNAL(clicked()), this, SLOT(onButtonLoad()));
    connect(btnSave, SIGNAL(clicked()), this, SLOT(onButtonSave()));
    connect(btnFlush, SIGNAL(clicked()), this, SLOT(onButtonFlush()));
    connect(btnShowRes, SIGNAL(clicked()), this, SLOT(onButtonShowRes()));

    // ── 如果 ground.dat 已存在则自动加载 ───────────────────────
    onButtonLoad();

    // ── 尝试自动加载点模板 ────────────────────────────────────
    tryAutoLoad();

    // 添加弹性空间，使内容在垂直方向上自适应
    mainLayout->addStretch();
}

// ═══════════════════════════════════════════════════════════════
// 绘制事件 – 绘制结果预览图像
// ═══════════════════════════════════════════════════════════════

void DemarcateDlg::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    if (!m_resultImage.isNull()) {
        painter.drawImage(resultLabel->geometry(), m_resultImage);
    }
}

// ═══════════════════════════════════════════════════════════════
// 供 DisplayDlg 使用的公共接口
// ═══════════════════════════════════════════════════════════════

void DemarcateDlg::setDisplayDlg(DisplayDlg* dlg)
{
    m_pDispDlg = dlg;
}

// 当 m_setStatus == BORDER_SET 时由 DisplayDlg::mousePressEvent 调用。
void DemarcateDlg::PushPoint(const QPoint& pt)
{
    m_points.push_back(pt);

    if (!m_points.isEmpty()) {
        btnResetOne->setEnabled(true);
    }

    // 当收集完 25 个点后，启用"运行标定"按钮并关闭点采集模式。
    if (m_points.size() == CALIB_POINT_COUNT) {
        btnSet->setEnabled(true);
        if (m_pDispDlg) {
            m_pDispDlg->SelectSetStatus(DisplayDlg::SET_STATUS::NONE);
        }
    }

    // 调试输出
    std::wstring ws = QString("pt %1 : %2, %3")
        .arg(m_points.size())
        .arg(pt.x())
        .arg(pt.y())
        .toStdWString();
    Debug::get()->print(ws.c_str());
}

// ═══════════════════════════════════════════════════════════════
// 多项式求解器（替换原来的空壳 gmiv）
//
// 使用带部分主元的高斯消元法求解超定线性方程组 A * x ≈ b
// 的正规方程 (A^T A) x = A^T b。
//
// 参数
//   a   : 设计矩阵，行主序，大小 m×n（输入，会被修改）
//   m   : 方程数（行）  = 25
//   n   : 未知数个数（列）  = 10
//   b   : 右侧向量，长度 m
//   x   : 解向量，长度 n（输出）
//
// 成功返回 true，法方程奇异返回 false。
// ═══════════════════════════════════════════════════════════════

bool DemarcateDlg::solvePolynomial(double* a, int m, int n,
    const double* b, double* x) const
{
    // ── 构建正规方程：N = A^T A，rhs = A^T b ──────────────────
    // N 为 n×n，rhs 为 n×1。
    std::vector<std::vector<double>> N(n, std::vector<double>(n + 1, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < m; ++k) {
                sum += a[k * n + i] * a[k * n + j];
            }
            N[i][j] = sum;
        }
        // 右侧列
        double rhs = 0.0;
        for (int k = 0; k < m; ++k) {
            rhs += a[k * n + i] * b[k];
        }
        N[i][n] = rhs;
    }

    // ── 带部分主元的高斯消元 ───────────────────────────────────
    for (int col = 0; col < n; ++col) {
        // 查找主元行
        int pivotRow = col;
        double maxVal = std::abs(N[col][col]);
        for (int row = col + 1; row < n; ++row) {
            if (std::abs(N[row][col]) > maxVal) {
                maxVal = std::abs(N[row][col]);
                pivotRow = row;
            }
        }

        // 奇异检查
        if (maxVal < 1e-12) {
            // 方程组奇异或接近奇异；将解向量置零
            for (int i = 0; i < n; ++i) x[i] = 0.0;
            return false;
        }

        // 行交换
        if (pivotRow != col) {
            std::swap(N[pivotRow], N[col]);
        }

        // 消元
        for (int row = col + 1; row < n; ++row) {
            double factor = N[row][col] / N[col][col];
            for (int j = col; j <= n; ++j) {
                N[row][j] -= factor * N[col][j];
            }
        }
    }

    // ── 回代 ─────────────────────────────────────────────────
    for (int i = n - 1; i >= 0; --i) {
        double sum = N[i][n];
        for (int j = i + 1; j < n; ++j) {
            sum -= N[i][j] * x[j];
        }
        x[i] = sum / N[i][i];
    }

    return true;
}

// ═══════════════════════════════════════════════════════════════
// 透视校正
//
// 对当前相机画面进行变换，使场地呈现为俯视矩形。
// 需要恰好 4 个角点，按以下顺序存储在 m_perspectiveCorners 中：
//   [0] 左上, [1] 右上, [2] 右下, [3] 左下
//
// 变换结果写回 m_pDispDlg 的内部位图缓冲区
//（通过 friend/访问器模式暴露的 m_pDispSingle / m_pDispBitmap）。
// 如果可用角点少于 4 个，函数不执行任何操作并返回 false。
// ═══════════════════════════════════════════════════════════════

bool DemarcateDlg::applyPerspectiveCorrection()
{
    if (m_perspectiveCorners.size() < PERSPECTIVE_POINT_COUNT) {
        // 角点不足 – 静默跳过校正
        return false;
    }
    if (!m_pDispDlg) {
        return false;
    }

    // 源角点（来自操作员选择的像素点）
    std::vector<cv::Point2f> src(4);
    for (int i = 0; i < 4; ++i) {
        src[i] = cv::Point2f(
            static_cast<float>(m_perspectiveCorners[i].x()),
            static_cast<float>(m_perspectiveCorners[i].y()));
    }

    // 目标角点（完整图像矩形）
    std::vector<cv::Point2f> dst = {
        cv::Point2f(0.f,             0.f),
        cv::Point2f(DISPLAY_W - 1.f, 0.f),
        cv::Point2f(DISPLAY_W - 1.f, DISPLAY_H - 1.f),
        cv::Point2f(0.f,             DISPLAY_H - 1.f)
    };

    // 计算 3×3 单应性矩阵
    cv::Mat H = cv::getPerspectiveTransform(src, dst);

    // 将原始位图包装为 cv::Mat（BGR，3 通道）
    // DisplayDlg 将像素存储为 RGB 格式的 m_pDispSingle；
    // 对于 OpenCV 视为 BGR（对于纯几何操作通道顺序无关）。
    unsigned char* pSrc = m_pDispDlg->getDispSingle();
    if (!pSrc) return false;

    cv::Mat srcMat(DISPLAY_H, DISPLAY_W, CV_8UC3, pSrc);
    cv::Mat dstMat(DISPLAY_H, DISPLAY_W, CV_8UC3);

    cv::warpPerspective(srcMat, dstMat, H,
        cv::Size(DISPLAY_W, DISPLAY_H),
        cv::INTER_LINEAR);

    // 将校正后的图像写回 DisplayDlg 的缓冲区
    std::memcpy(pSrc, dstMat.data,
        static_cast<size_t>(DISPLAY_W) * DISPLAY_H * 3);

    return true;
}

// ═══════════════════════════════════════════════════════════════
// 按钮槽函数 – 运行标定
// ═══════════════════════════════════════════════════════════════

void DemarcateDlg::onButtonSet()
{
    if (!m_pDispDlg) {
        QMessageBox::warning(this, "Error", "DisplayDlg not initialised!");
        return;
    }

    if (m_points.size() < CALIB_POINT_COUNT) {
        QMessageBox::warning(this, "Error",
            QString("Need %1 points, only %2 collected.")
            .arg(CALIB_POINT_COUNT)
            .arg(m_points.size()));
        return;
    }

    m_pDispDlg->SelectSetStatus(DisplayDlg::SET_STATUS::NONE);

    // ── 步骤 1：透视校正（如果有角点可用）──────────────────────
    if (m_perspectiveCorners.size() == PERSPECTIVE_POINT_COUNT) {
        if (!applyPerspectiveCorrection()) {
            Debug::get()->print(L"[Demarcate] Perspective correction failed "
                L"(using raw frame).");
        }
        else {
            Debug::get()->print(L"[Demarcate] Perspective correction applied.");
        }
    }

    // ── 步骤 2：构建设计矩阵 A（25 行 × 10 列）───────────────
    // 多项式基函数（以像素 (320, 240) 为中心）：
    //   列 0: 1
    //   列 1: dx
    //   列 2: dy
    //   列 3: dx*dy
    //   列 4: dx²
    //   列 5: dy²
    //   列 6: dx²·dy
    //   列 7: dx·dy²
    //   列 8: dx³
    //   列 9: dy³
    // 其中 dx = pixel_x - 320，dy = pixel_y - 240。

    const int M = CALIB_POINT_COUNT;   // 25 行
    const int N = 10;                  // 10 个系数列

    std::vector<double> A(M * N);

    for (int j = 0; j < M; ++j) {
        double dx = m_points[j].x() - 320.0;
        double dy = m_points[j].y() - 240.0;

        A[j * N + 0] = 1.0;
        A[j * N + 1] = dx;
        A[j * N + 2] = dy;
        A[j * N + 3] = dx * dy;
        A[j * N + 4] = dx * dx;
        A[j * N + 5] = dy * dy;
        A[j * N + 6] = dx * dx * dy;
        A[j * N + 7] = dx * dy * dy;
        A[j * N + 8] = dx * dx * dx;
        A[j * N + 9] = dy * dy * dy;
    }

    // 保留副本用于 Y 求解（solvePolynomial 会修改矩阵）
    std::vector<double> A2(A);

    // ── 步骤 3：在缩放坐标中构建右侧向量 ───────────────────────
    // 原始代码将目标坐标缩放为：(coord - centre) * kScale
    std::vector<double> Bx(M), By(M);
    for (int i = 0; i < M; ++i) {
        Bx[i] = (kBx[i] - kCx) * kScale;
        By[i] = (kBy[i] - kCy) * kScale;
    }

    // ── 步骤 4：求解 X 和 Y 的多项式系数 ──────────────────────
    std::vector<double> px(N, 0.0), py(N, 0.0);

    bool okX = solvePolynomial(A.data(), M, N, Bx.data(), px.data());
    bool okY = solvePolynomial(A2.data(), M, N, By.data(), py.data());

    if (!okX || !okY) {
        QMessageBox::critical(this, "Calibration Failed",
            "The linear system is singular.\n"
            "Please check that the 25 clicked points are not collinear "
            "and cover the full field area.");
        return;
    }

    // ── 步骤 5：为每个像素填充地面表 ──────────────────────────
    progressBar->setValue(0);

    for (int i = 0; i < DISPLAY_W; ++i) {
        for (int j = 0; j < DISPLAY_H; ++j) {
            double dx = i - 320.0;
            double dy = j - 240.0;

            double fx = px[0]
                + px[1] * dx + px[2] * dy
                + px[3] * dx * dy
                + px[4] * dx * dx + px[5] * dy * dy
                + px[6] * dx * dx * dy + px[7] * dx * dy * dy
                + px[8] * dx * dx * dx + px[9] * dy * dy * dy;

            double fy = py[0]
                + py[1] * dx + py[2] * dy
                + py[3] * dx * dy
                + py[4] * dx * dx + py[5] * dy * dy
                + py[6] * dx * dx * dy + py[7] * dx * dy * dy
                + py[8] * dx * dx * dx + py[9] * dy * dy * dy;

            // 从缩放坐标转换回厘米
            ground.groundInfo[i][j].x = static_cast<float>(fx / kScale + kCx);
            ground.groundInfo[i][j].y = static_cast<float>(fy / kScale + kCy);

            // 边界测试：该像素是否在场地多边形内？
            QPolygon polygon;
            for (int k = 0; k < 13; ++k) {
                polygon << point[k];
            }
            QPoint fieldPt(
                static_cast<int>(ground.groundInfo[i][j].x),
                static_cast<int>(ground.groundInfo[i][j].y));

            ground.groundInfo[i][j].flag =
                polygon.containsPoint(fieldPt, Qt::OddEvenFill) ? 1 : 0;
        }

        progressBar->setValue(i);
        // 在扫描过程中保持 UI 响应
        QCoreApplication::processEvents();
    }
    m_statusBar->setStyleSheet(
        "QLabel { background-color: #00FF00; border: 1px solid black; }");
    m_needResetDC = true;
    onButtonShowRes();

    btnSet->setEnabled(false);
    btnResetOne->setEnabled(false);
    m_isSaved = false;
}

// ═══════════════════════════════════════════════════════════════
// 按钮槽函数 – 撤销上一步
// ═══════════════════════════════════════════════════════════════

void DemarcateDlg::onButtonResetOne()
{
    if (!m_points.isEmpty()) {
        m_points.pop_back();
        // Keep the marker overlay in sync
        if (m_pDispDlg) m_pDispDlg->clearCalibPoints();
        for (const QPoint& p : m_points)
            if (m_pDispDlg) m_pDispDlg->addCalibPoint(p);

        btnSet->setEnabled(false);
        if (m_points.isEmpty()) {
            btnResetOne->setEnabled(false);
        }
    }

    if (m_pDispDlg) {
        m_pDispDlg->ShowSingle(); // redraws with updated marker list
    }

    Debug::get()->print(L"[Demarcate] Last point removed.");
}

// ═══════════════════════════════════════════════════════════════
// 按钮槽函数 – 重置标定
// ═══════════════════════════════════════════════════════════════

void DemarcateDlg::onButtonReset()
{
    if (m_pDispDlg) {
        m_pDispDlg->clearCalibPoints(); // remove all red cross markers
        m_pDispDlg->SelectSetStatus(DisplayDlg::SET_STATUS::BORDER_SET);
        m_pDispDlg->ShowSingle();       // refresh display without markers
    }

    m_points.clear();
    m_perspectiveCorners.clear();
    progressBar->setValue(0);
    m_statusBar->setStyleSheet(
        "QLabel { background-color: #FF0000; border: 1px solid black; }");  // 重置为红色
    btnSet->setEnabled(false);
    btnResetOne->setEnabled(false);

    m_resultImage.fill(Qt::black);
    resultLabel->setPixmap(QPixmap::fromImage(m_resultImage)); // clear preview
    update();

    Debug::get()->print(L"[Demarcate] Reset. Click 25 field points to recalibrate.");
}

// ═══════════════════════════════════════════════════════════════
// 按钮槽函数 – 加载 ground.dat
// ═══════════════════════════════════════════════════════════════

void DemarcateDlg::onButtonLoad()
{
    // 如果标定数据将被丢弃则发出警告
    if (!m_isSaved) {
        int ret = QMessageBox::question(
            this, "Field Calibration",
            "Unsaved calibration data will be lost. Continue?",
            QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No) return;
    }

    QFile file(kGroundDataFile);
    if (file.open(QIODevice::ReadOnly)) {
        file.read(reinterpret_cast<char*>(&ground), sizeof(Ground));
        file.close();
        m_needResetDC = true;
        m_isSaved = true;

        // 更新状态条为绿色，表示标定数据已加载
        m_statusBar->setStyleSheet(
            "QLabel { background-color: #00FF00; border: 1px solid black; }");

        Debug::get()->print(L"[Demarcate] ground.dat loaded successfully.");
    }
    else {
        // 首次运行时不是错误 – 文件尚不存在
        Debug::get()->print(L"[Demarcate] ground.dat not found "
            L"(will be created after first calibration).");
    }
}

// ═══════════════════════════════════════════════════════════════
// 按钮槽函数 – 保存 ground.dat
// ═══════════════════════════════════════════════════════════════

bool DemarcateDlg::saveCalibration()
{
    // ── 保存地面坐标表 ─────────────────────────────────────────
    QFile file(kGroundDataFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(reinterpret_cast<const char*>(&ground), sizeof(Ground));
        file.close();
        m_isSaved = true;
        Debug::get()->print(L"[Demarcate] ground.dat saved.");
        return true;
    }
    else {
        QMessageBox::warning(this, "Error",
            QString("Cannot write %1").arg(kGroundDataFile));
        return false;
    }
}

void DemarcateDlg::onButtonSave()
{
    if (saveCalibration()) {
        QMessageBox::information(this, "Saved",
            "Calibration data has been saved.");
    }
}

// ═══════════════════════════════════════════════════════════════
// 按钮槽函数 – 刷新显示画面
// ═══════════════════════════════════════════════════════════════

void DemarcateDlg::onButtonFlush()
{
    if (m_pDispDlg) {
        m_pDispDlg->ShowSingle();
    }
}

// ═══════════════════════════════════════════════════════════════
// 按钮槽函数 – 显示标定结果预览
// ═══════════════════════════════════════════════════════════════

void DemarcateDlg::onButtonShowRes()
{
    if (!m_needResetDC && !m_resultImage.isNull()) {
        // 已是最新，只需刷新 pixmap（例如控件被重绘后）
        resultLabel->setPixmap(QPixmap::fromImage(m_resultImage)
            .scaled(resultLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        return;
    }

    // ════════════════════════════════════════════════════════════
    // 输出图像坐标系（俯视场地坐标系）
    //   场地 X：-15 ~ 235 cm（含左右边线延伸区，总宽 250 cm）
    //   场地 Y：  0 ~ 180 cm（总高 180 cm）
    //   缩放  ：2.4 px/cm；四周各留 10 px 边距
    // ════════════════════════════════════════════════════════════
    const double kPxPerCm = 2.4;
    const int    kPadding = 10;
    const int    kFieldXMin = -15;
    const int    kFieldXMax = 235;
    const int    kFieldYMin = 0;
    const int    kFieldYMax = 180;
    const int kImgW = static_cast<int>((kFieldXMax - kFieldXMin) * kPxPerCm) + 2 * kPadding;
    const int kImgH = static_cast<int>((kFieldYMax - kFieldYMin) * kPxPerCm) + 2 * kPadding;

    // 场地原点 (0 cm, 0 cm) 在输出图像中的像素偏移
    const int kOffX = static_cast<int>(-kFieldXMin * kPxPerCm) + kPadding;
    const int kOffY = kPadding;

    unsigned char* pCam = m_pDispDlg ? m_pDispDlg->getDispBitmap() : nullptr;

    m_resultImage = QImage(kImgW, kImgH, QImage::Format_RGB32);
    m_resultImage.fill(Qt::black);


    progressBar->setValue(0);

    if (pCam) {
        // ════════════════════════════════════════════════════════
        // 模式 A：摄像头图像重投影（前向映射）
        //
        // 对每个 flag=1 的摄像头像素 (i, j)：
        //   - 读取 ground 表中对应的场地坐标 (fx, fy)（单位 cm）
        //   - 将 (fx, fy) 映射为输出图像像素坐标
        //   - 用 2×2 色块填充，减少前向映射留下的空洞
        //
        // 结果为俯视展开的真实摄像头纹理，直观反映标定质量。
        // ════════════════════════════════════════════════════════
        for (int i = 0; i < DISPLAY_W; ++i) {
            for (int j = 0; j < DISPLAY_H; ++j) {
                if (!ground.groundInfo[i][j].flag) continue;

                float fx = ground.groundInfo[i][j].x;  // cm
                float fy = ground.groundInfo[i][j].y;  // cm
                int outX = static_cast<int>(fx * kPxPerCm) + kOffX;
                int outY = static_cast<int>(fy * kPxPerCm) + kOffY;

                if (outX < 0 || outX + 1 >= kImgW ||
                    outY < 0 || outY + 1 >= kImgH)
                    continue;

                int srcIdx = (j * DISPLAY_W + i) * 3;
                QRgb color = qRgb(pCam[srcIdx], pCam[srcIdx + 1], pCam[srcIdx + 2]);

                // 2×2 色块填充，减少空洞
                m_resultImage.setPixel(outX, outY, color);
                m_resultImage.setPixel(outX + 1, outY, color);
                m_resultImage.setPixel(outX, outY + 1, color);
                m_resultImage.setPixel(outX + 1, outY + 1, color);
            }
            progressBar->setValue(i);
            QCoreApplication::processEvents();
        }
    }
    else {
        // ════════════════════════════════════════════════════════
        // 模式 B：无摄像头帧（后备显示）
        // 将 flag=1 的像素绘制为绿点，展示有效标定区域
        // ════════════════════════════════════════════════════════
        QPainter painter(&m_resultImage);
        painter.setPen(QColor(0, 200, 0));
        for (int i = 0; i < DISPLAY_W; ++i) {
            for (int j = 0; j < DISPLAY_H; ++j) {
                if (!ground.groundInfo[i][j].flag) continue;
                float fx = ground.groundInfo[i][j].x;
                float fy = ground.groundInfo[i][j].y;
                int outX = static_cast<int>(fx * kPxPerCm) + kOffX;
                int outY = static_cast<int>(fy * kPxPerCm) + kOffY;
                if (outX >= 0 && outX < kImgW && outY >= 0 && outY < kImgH)
                    painter.drawPoint(outX, outY);
            }
            progressBar->setValue(i);
            QCoreApplication::processEvents();
        }
    }

    // ── 叠加场地边界多边形（白色，2 px 线宽）────────────────
    {
        QPainter painter(&m_resultImage);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(Qt::white, 2));
        QPolygon polygon;
        for (int k = 0; k < 13; ++k) {
            polygon << QPoint(
                static_cast<int>(point[k].x() * kPxPerCm) + kOffX,
                static_cast<int>(point[k].y() * kPxPerCm) + kOffY);
        }
        painter.drawPolyline(polygon);
    }

    // ── 叠加 25 个标定控制点理论位置（黄色十字，6 px 臂长）─
    {
        QPainter painter(&m_resultImage);
        painter.setPen(QPen(Qt::yellow, 1));
        const int arm = 5;
        for (int idx = 0; idx < CALIB_POINT_COUNT; ++idx) {
            int cx = static_cast<int>(kBx[idx] * kPxPerCm) + kOffX;
            int cy = static_cast<int>(kBy[idx] * kPxPerCm) + kOffY;
            painter.drawLine(cx - arm, cy, cx + arm, cy);
            painter.drawLine(cx, cy - arm, cx, cy + arm);
        }
    }

    m_needResetDC = false;

    resultLabel->setPixmap(QPixmap::fromImage(m_resultImage)
        .scaled(resultLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
