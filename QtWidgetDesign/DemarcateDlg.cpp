/*
 * DemarcateDlg.cpp  –  场地标定对话框实现
 *
 * 模块职责
 * ──────────────────
 * 1. 采集 25 个控制点的像素坐标，与场地理论坐标做最小二乘拟合，
 *    得到像素→场地坐标的多项式映射（三次二元多项式，10 个系数）。
 *    求解采用带部分主元的高斯消元法，处理超定方程的正规方程。
 *
 * 2. 可选透视校正：若操作员提供 4 个场地角点像素坐标，在拟合前
 *    先把相机画面变换为俯视矩形，消除桶形/倾斜畸变。
 *
 * 3. 拟合结果按像素索引写入全局 ground 表，供其它模块查表获得
 *    场地坐标及有效区域标记（flag）。
 *
 */

#include "DemarcateDlg.h"
#include "DisplayDlg.h"
#include "Debug.h"

#include <cmath>
#include <cstring>
#include <algorithm>

 // OpenCV 仅用于透视校正，不参与标定拟合本身
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

// ---------------------------------------------------------------
// 全局地面标定表：每个像素索引对应一个场地坐标 + 有效区域标记
// ---------------------------------------------------------------
Ground ground;

// ---------------------------------------------------------------
// 25 个控制点的场地理论坐标（厘米），与场地实际布置一一对应
// 行从上到下、列从左到右；标定质量取决于点击点与该布局的吻合度
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

// 场地几何中心与缩放因子：拟合在以中心为原点的缩放坐标系中进行，
// 改善数值条件；求出系数后还原回厘米坐标
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
    // 描述场地有效区域，用于判定像素是否落在场内（标定时置 flag）
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
    // 尺寸与 onButtonShowRes() 俯视输出一致，构造时即分配以避免后续重绘抖动
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

    QLabel* titleLabel = new QLabel("标定", this);
    titleLabel->setFont(font);
    controlLayout->addWidget(titleLabel);
    controlLayout->setAlignment(titleLabel, Qt::AlignTop);

    // 结果预览区域：最小尺寸容纳俯视图，策略允许随面板拉伸放大
    resultLabel = new QLabel(this);
    resultLabel->setMinimumSize(560, 400);
    resultLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setStyleSheet("QLabel { background-color: black; border: 1px solid #555555; }");
    controlLayout->addWidget(resultLabel, 0, Qt::AlignCenter);

    // 进度条：以像素列扫描进度为刻度，标定/预览耗时操作时反馈
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, DISPLAY_W);
    progressBar->setValue(0);
    controlLayout->addWidget(progressBar);

    // 状态条：红色=未标定，绿色=已标定/数据已加载
    m_statusBar = new QLabel(this);
    m_statusBar->setStyleSheet("QLabel { background-color: #FF0000; border: 1px solid black; }");
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

    btnShowRes = new QPushButton("查看当前标定结果", this);
    btnShowRes->setFont(font);
    controlLayout->addWidget(btnShowRes, 0, Qt::AlignCenter);

    mainLayout->addLayout(controlLayout);
    mainLayout->addStretch();
    mainLayout->setAlignment(Qt::AlignTop);

    // ── 初始按钮状态：未采集点前"开始/撤销"均不可用
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

    mainLayout->addStretch();
}

// ═══════════════════════════════════════════════════════════════
// 供 DisplayDlg 使用的公共接口
// ═══════════════════════════════════════════════════════════════

void DemarcateDlg::setDisplayDlg(DisplayDlg* dlg)
{
    m_pDispDlg = dlg;
}

// 点采集入口：DisplayDlg 在 BORDER_SET 状态下点击时回调本函数
void DemarcateDlg::PushPoint(const QPoint& pt)
{
    m_points.push_back(pt);

    if (!m_points.isEmpty()) {
        btnResetOne->setEnabled(true);
    }

    // 收满 25 个点即激活标定按钮，并退出采集模式避免误点
    if (m_points.size() == CALIB_POINT_COUNT) {
        btnSet->setEnabled(true);
        if (m_pDispDlg) {
            m_pDispDlg->SelectSetStatus(DisplayDlg::SET_STATUS::NONE);
        }
    }

    std::wstring ws = QString("pt %1 : %2, %3")
        .arg(m_points.size())
        .arg(pt.x())
        .arg(pt.y())
        .toStdWString();
    Debug::get()->print(ws.c_str());
}

// ═══════════════════════════════════════════════════════════════
// 最小二乘求解器：对超定方程 A*x≈b 构造正规方程 (AᵀA)x=Aᵀb，
// 再用带部分主元的高斯消元法求解。部分主元保证数值稳定性。
//
//   a : 设计矩阵（行主序 m×n），输入会被原地修改
//   b : 右侧向量（长度 m）
//   x : 解向量（长度 n，输出）
// 法方程奇异（点共线等）返回 false 并将 x 置零。
// ═══════════════════════════════════════════════════════════════

bool DemarcateDlg::solvePolynomial(double* a, int m, int n,
    const double* b, double* x) const
{
    // ── 构建增广正规方程：N=[AᵀA | Aᵀb]，n×(n+1)
    std::vector<std::vector<double>> N(n, std::vector<double>(n + 1, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < m; ++k) {
                sum += a[k * n + i] * a[k * n + j];
            }
            N[i][j] = sum;
        }
        // 右侧列 Aᵀb
        double rhs = 0.0;
        for (int k = 0; k < m; ++k) {
            rhs += a[k * n + i] * b[k];
        }
        N[i][n] = rhs;
    }

    // ── 带部分主元的高斯消元 ───────────────────────────────────
    for (int col = 0; col < n; ++col) {
        // 选列中绝对值最大者为主元
        int pivotRow = col;
        double maxVal = std::abs(N[col][col]);
        for (int row = col + 1; row < n; ++row) {
            if (std::abs(N[row][col]) > maxVal) {
                maxVal = std::abs(N[row][col]);
                pivotRow = row;
            }
        }

        // 主元过小视为奇异，置零并失败返回
        if (maxVal < 1e-12) {
            for (int i = 0; i < n; ++i) x[i] = 0.0;
            return false;
        }

        if (pivotRow != col) {
            std::swap(N[pivotRow], N[col]);
        }

        for (int row = col + 1; row < n; ++row) {
            double factor = N[row][col] / N[col][col];
            for (int j = col; j <= n; ++j) {
                N[row][j] -= factor * N[col][j];
            }
        }
    }

    // ── 回代求解 x
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
// 透视校正：用 4 个场地角点求单应矩阵，把相机画面校正为俯视矩形，
// 结果就地写回 DisplayDlg 的位图缓冲区，使后续多项式拟合在无畸变
// 图像上进行。
// 角点顺序须为 [左上, 右上, 右下, 左下]；不足 4 个则跳过并返回 false。
// ═══════════════════════════════════════════════════════════════

bool DemarcateDlg::applyPerspectiveCorrection()
{
    if (m_perspectiveCorners.size() < PERSPECTIVE_POINT_COUNT) {
        // 角点不足，静默跳过校正
        return false;
    }
    if (!m_pDispDlg) {
        return false;
    }

    // 源角点：操作员选取的场地四角像素坐标
    std::vector<cv::Point2f> src(4);
    for (int i = 0; i < 4; ++i) {
        src[i] = cv::Point2f(
            static_cast<float>(m_perspectiveCorners[i].x()),
            static_cast<float>(m_perspectiveCorners[i].y()));
    }

    // 目标角点：映射到完整图像矩形，得到无透视的俯视画面
    std::vector<cv::Point2f> dst = {
        cv::Point2f(0.f,             0.f),
        cv::Point2f(DISPLAY_W - 1.f, 0.f),
        cv::Point2f(DISPLAY_W - 1.f, DISPLAY_H - 1.f),
        cv::Point2f(0.f,             DISPLAY_H - 1.f)
    };

    // 3×3 单应性矩阵
    cv::Mat H = cv::getPerspectiveTransform(src, dst);

    // 把 DisplayDlg 的位图包装成 cv::Mat；仅做几何 warp，
    // 通道顺序不影响结果，故按 BGR 视图复用
    unsigned char* pSrc = m_pDispDlg->getDispSingle();
    if (!pSrc) return false;

    cv::Mat srcMat(DISPLAY_H, DISPLAY_W, CV_8UC3, pSrc);
    cv::Mat dstMat(DISPLAY_H, DISPLAY_W, CV_8UC3);

    cv::warpPerspective(srcMat, dstMat, H,
        cv::Size(DISPLAY_W, DISPLAY_H),
        cv::INTER_LINEAR);

    // 校正结果就地写回，供后续标定直接使用
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

    // ── 步骤 1：若有角点则先做透视校正，消除镜头畸变
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
    // 三次二元多项式基，以图像中心 (320,240) 为原点减均值，
    // 改善条件数。10 个系数：1, dx, dy, dxdy, dx², dy²,
    // dx²dy, dxdy², dx³, dy³（dx=pixel_x-320, dy=pixel_y-240）。

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

    // 矩阵会被求解器原地修改，故备份用于 Y 方程
    std::vector<double> A2(A);

    // ── 步骤 3：目标坐标转入缩放坐标系 (coord - 中心) * kScale
    std::vector<double> Bx(M), By(M);
    for (int i = 0; i < M; ++i) {
        Bx[i] = (kBx[i] - kCx) * kScale;
        By[i] = (kBy[i] - kCy) * kScale;
    }

    // ── 步骤 4：分别拟合 X、Y 两套多项式系数
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

    // ── 步骤 5：用拟合系数反算每个像素的场地坐标并填表
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

            // 还原缩放坐标到厘米
            ground.groundInfo[i][j].x = static_cast<float>(fx / kScale + kCx);
            ground.groundInfo[i][j].y = static_cast<float>(fy / kScale + kCy);

            // 用场地多边形判定像素是否有效，写入 flag
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
        // 逐列让出事件循环，避免长循环冻结界面
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
        // 同步刷新画面上的十字标记覆盖层
        if (m_pDispDlg) m_pDispDlg->clearCalibPoints();
        for (const QPoint& p : m_points)
            if (m_pDispDlg) m_pDispDlg->addCalibPoint(p);

        btnSet->setEnabled(false);
        if (m_points.isEmpty()) {
            btnResetOne->setEnabled(false);
        }
    }

    if (m_pDispDlg) {
        m_pDispDlg->ShowSingle();
    }

    Debug::get()->print(L"[Demarcate] Last point removed.");
}

// ═══════════════════════════════════════════════════════════════
// 按钮槽函数 – 重置标定
// ═══════════════════════════════════════════════════════════════

void DemarcateDlg::onButtonReset()
{
    if (m_pDispDlg) {
        m_pDispDlg->clearCalibPoints();
        m_pDispDlg->SelectSetStatus(DisplayDlg::SET_STATUS::BORDER_SET);
        m_pDispDlg->ShowSingle();
    }

    m_points.clear();
    m_perspectiveCorners.clear();
    progressBar->setValue(0);
    m_statusBar->setStyleSheet(
        "QLabel { background-color: #FF0000; border: 1px solid black; }");
    btnSet->setEnabled(false);
    btnResetOne->setEnabled(false);

    m_resultImage.fill(Qt::black);
    resultLabel->setPixmap(QPixmap::fromImage(m_resultImage));
    update();

    Debug::get()->print(L"[Demarcate] Reset. Click 25 field points to recalibrate.");
}

// ═══════════════════════════════════════════════════════════════
// 按钮槽函数 – 加载 ground.dat
// ═══════════════════════════════════════════════════════════════

void DemarcateDlg::onButtonLoad()
{
    // 未保存数据将被覆盖，先确认
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

        // 状态条转绿，表示已有可用标定数据
        m_statusBar->setStyleSheet(
            "QLabel { background-color: #00FF00; border: 1px solid black; }");

        Debug::get()->print(L"[Demarcate] ground.dat loaded successfully.");
    }
    else {
        // 首次运行文件尚不存在属正常情况，非错误
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
        // 保存成功后清除画面标记并刷新，提示用户标定已固化
        if (m_pDispDlg) {
            m_pDispDlg->clearCalibPoints();
            m_pDispDlg->ShowSingle();
        }
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
        // 数据未变化，仅按控件尺寸刷新 pixmap（如窗口缩放后）
        resultLabel->setPixmap(QPixmap::fromImage(m_resultImage)
            .scaled(resultLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        return;
    }

    // ════════════════════════════════════════════════════════════
    // 俯视输出图像坐标系：场地 cm 经 2.4 px/cm 缩放，四周留 10 px 边距
    //   X: -15~235 cm（含左右边线延伸），Y: 0~180 cm
    // ════════════════════════════════════════════════════════════
    const double kPxPerCm = 2.4;
    const int    kPadding = 10;
    const int    kFieldXMin = -15;
    const int    kFieldXMax = 235;
    const int    kFieldYMin = 0;
    const int    kFieldYMax = 180;
    const int kImgW = static_cast<int>((kFieldXMax - kFieldXMin) * kPxPerCm) + 2 * kPadding;
    const int kImgH = static_cast<int>((kFieldYMax - kFieldYMin) * kPxPerCm) + 2 * kPadding;

    // 场地原点 (0,0)cm 在输出图像中的像素偏移
    const int kOffX = static_cast<int>(-kFieldXMin * kPxPerCm) + kPadding;
    const int kOffY = kPadding;

    unsigned char* pCam = m_pDispDlg ? m_pDispDlg->getDispBitmap() : nullptr;

    m_resultImage = QImage(kImgW, kImgH, QImage::Format_RGB32);
    m_resultImage.fill(Qt::black);


    progressBar->setValue(0);

    if (pCam) {
        // ════════════════════════════════════════════════════════
        // 模式 A：摄像头图像前向重投影。
        // 对每个 flag=1 像素，查表得到场地坐标后映射到输出图像，
        // 用 2×2 色块填补以减少前向映射的空洞，得到真实俯视纹理。
        // ════════════════════════════════════════════════════════
        for (int i = 0; i < DISPLAY_W; ++i) {
            for (int j = 0; j < DISPLAY_H; ++j) {
                if (!ground.groundInfo[i][j].flag) continue;

                float fx = ground.groundInfo[i][j].x;
                float fy = ground.groundInfo[i][j].y;
                int outX = static_cast<int>(fx * kPxPerCm) + kOffX;
                int outY = static_cast<int>(fy * kPxPerCm) + kOffY;

                if (outX < 0 || outX + 1 >= kImgW ||
                    outY < 0 || outY + 1 >= kImgH)
                    continue;

                int srcIdx = (j * DISPLAY_W + i) * 3;
                QRgb color = qRgb(pCam[srcIdx], pCam[srcIdx + 1], pCam[srcIdx + 2]);

                // 2×2 色块填充，缓解前向映射空洞
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
        // 模式 B：无摄像头帧时以绿点描绘有效标定区域作为后备显示
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

    // ── 叠加场地边界多边形（白线），便于核对有效区域
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

    // ── 叠加 25 个控制点理论位置（黄色十字），用于比对重投影偏差
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
