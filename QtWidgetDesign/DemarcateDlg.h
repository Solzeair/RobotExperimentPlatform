/*
 * DemarcateDlg.h  –  场地标定对话框
 * 作者 : 李青  (修订版)
 *
 * 与原版的主要区别
 * ────────────────────────
 * 1. 移除了重复的 Ground / GroundInfo 结构体定义。
 *    唯一的权威定义现在位于 utili.h。
 *
 * 2. 真正的多项式求解器
 *    gmiv() 现在包含正确的最小二乘/SVD 实现
 *    （带部分主元的高斯消元法），因此标定计算
 *    实际上可以产生正确的坐标系数。
 *
 * 3. 透视校正
 *    在运行多项式拟合之前，使用四点透视变换
 *    （OpenCV getPerspectiveTransform + warpPerspective）
 *    将原始相机画面校正为俯视矩形。
 *    校正后的画面存储回 DisplayDlg 的位图缓冲区，
 *    以便所有下游处理都能看到校正后的图像。
 */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QMessageBox>
#include <QThread>
#include <QCoreApplication>
#include <QTimer>
#include <QElapsedTimer>
#include <QPainter>
#include <QImage>
#include <QFile>
#include <QVector>
#include <QDataStream>

#include "utili.h"          // Ground / GroundInfo / 文件名常量

 // 前向声明 – 避免与 DisplayDlg.h 循环包含
class DisplayDlg;

// ---------------------------------------------------------------
// 全局地面标定表
// （extern 声明 – 定义在 DemarcateDlg.cpp 中）
// ---------------------------------------------------------------
extern Ground ground;

// ---------------------------------------------------------------
// 多项式拟合中使用的场地控制点数量
// ---------------------------------------------------------------
static const int CALIB_POINT_COUNT = 25;

// ---------------------------------------------------------------
// 模板中保存的透视角点数量
// ---------------------------------------------------------------
static const int PERSPECTIVE_POINT_COUNT = 4;

class DemarcateDlg : public QWidget
{
    Q_OBJECT

public:
    explicit DemarcateDlg(QWidget* parent = nullptr);
    ~DemarcateDlg();

    // 当对话框处于点采集模式时，由 DisplayDlg 在用户点击图像内部时调用。
    void PushPoint(const QPoint& pt);

    // 注入 DisplayDlg 指针，以便调用 ShowSingle()、
    // SelectSetStatus() 以及读写位图缓冲区。
    void setDisplayDlg(DisplayDlg* dlg);

    // 检查是否有未保存的标定数据（用于退出时提示）
    bool hasUnsavedData() const { return !m_isSaved; }
    // 保存标定数据并返回是否成功
    bool saveCalibration();

private slots:
    // 按钮处理函数（UI 未变）
    void onButtonSet();         // 使用当前点运行标定
    void onButtonResetOne();    // 移除最后收集的点
    void onButtonReset();        // 放弃所有点，重新进入采集模式
    void onButtonLoad();        // 从磁盘加载 ground.dat
    void onButtonSave();        // 将 ground.dat 保存到磁盘
    void onButtonFlush();       // 刷新显示画面
    void onButtonShowRes();     // 渲染标定结果预览

private:
    // ── UI ──────────────────────────────────────────────────────
    void initUI();
    // paintEvent removed: result preview is now set via resultLabel->setPixmap()
    // inside onButtonShowRes(), which is the correct Qt pattern for QLabel display.

    // ── 标定计算 ───────────────────────────────────────────────
    // 使用 25 个控制点对求解将像素坐标映射到场地坐标的
    // 10 系数多项式。
    // a[m*n] = 设计矩阵（m=25 行，n=10 列）
    // b[m]   = 右侧向量（目标 X 或 Y，缩放单位）
    // x[n]   = 解系数（输出）
    // 如果方程组奇异/病态则返回 false。
    bool solvePolynomial(double* a, int m, int n,
        const double* b, double* x) const;

    // ── 透视校正 ───────────────────────────────────────────────
    // 对 DisplayDlg 中的当前画面应用四点透视变换，
    // 将其校正为 DISPLAY_W × DISPLAY_H 的矩形。
    // 四个源角点必须按以下顺序存储在 m_perspectiveCorners 中：
    // 左上、右上、右下、左下。
    // 成功返回 true。
    bool applyPerspectiveCorrection();

private:
    // 状态指示条（红 = 未完成标定，绿 = 已完成）
    QLabel* m_statusBar;
    // ── 布局/控件 ──────────────────────────────────────────────
    QVBoxLayout* mainLayout;
    QLabel* resultLabel;
    QProgressBar* progressBar;
    QPushButton* btnSet;
    QPushButton* btnResetOne;
    QPushButton* btnReset;
    QPushButton* btnLoad;
    QPushButton* btnSave;
    QPushButton* btnFlush;
    QPushButton* btnShowRes;

    // ── 状态 ───────────────────────────────────────────────────
    bool           m_isSaved;       // ground.dat 已写入
    bool           m_needResetDC;   // 结果预览需要刷新

    // ── 数据 ────────────────────────────────────────────────────
    QVector<QPoint> m_points;            // 25 个像素控制点（已点击）
    QVector<QPoint> m_perspectiveCorners;// 用于变换的 4 个角点（可选）
    DisplayDlg* m_pDispDlg;
    QImage          m_resultImage;       // 在 paintEvent 中渲染的结果预览

    // 场地边界多边形（13 个顶点，单位厘米）
    QPoint          point[13];
};
