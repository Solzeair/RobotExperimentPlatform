/*
 * DemarcateDlg – 场地标定对话框
 *
 * 设计目标：将相机像素坐标映射为场地坐标。
 * 处理流程为先做四点透视校正（俯视矩形化），
 * 再用 25 个控制点求解 10 系数多项式。
 * Ground/GroundInfo 的权威定义统一收敛在 utili.h，
 * 此处仅负责采集点、驱动计算与持久化。
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

#include "utili.h"          // 场地结构体与文件名常量的唯一权威定义

 // 前向声明，切断与 DisplayDlg 的循环包含依赖
class DisplayDlg;

// 全局地面标定表，extern 声明（定义在 DemarcateDlg.cpp），供全局共享标定结果
extern Ground ground;

// 多项式拟合的控制点数，需与场地标定点位设计一致
static const int CALIB_POINT_COUNT = 25;

// 透视校正使用的源角点数，固定为矩形四角
static const int PERSPECTIVE_POINT_COUNT = 4;

class DemarcateDlg : public QWidget
{
    Q_OBJECT

public:
    explicit DemarcateDlg(QWidget* parent = nullptr);
    ~DemarcateDlg();

    // 采集模式下由 DisplayDlg 在用户点击图像时回调，逐点收集控制点
    void PushPoint(const QPoint& pt);

    // 注入 DisplayDlg，用于复用其显示、状态切换与位图缓冲区
    void setDisplayDlg(DisplayDlg* dlg);

    // 退出前判断是否有未保存标定数据，触发提示
    bool hasUnsavedData() const { return !m_isSaved; }
    // 持久化标定数据，返回是否写入成功
    bool saveCalibration();

private slots:
    void onButtonSet();         // 基于已采集点运行标定计算
    void onButtonResetOne();    // 撤销最近一个控制点
    void onButtonReset();        // 清空全部点并回到采集模式
    void onButtonLoad();        // 从 ground.dat 读回历史标定
    void onButtonSave();        // 将当前标定写入 ground.dat
    void onButtonFlush();       // 刷新画面显示
    void onButtonShowRes();     // 渲染并预览标定结果

private:
    // ── UI ──────────────────────────────────────────────────────
    void initUI();
    // 结果预览不再走 paintEvent，改为在 onButtonShowRes() 中通过
    // resultLabel->setPixmap() 设置，符合 QLabel 的标准显示方式。

    // ── 标定计算 ───────────────────────────────────────────────
    // 最小二乘求解像素到场地坐标的 10 系数多项式映射：
    // a[m*n] 为设计矩阵（m=25 行，n=10 列），b[m] 为目标向量（缩放后的 X 或 Y），
    // x[n] 输出解系数。方程组奇异或病态时返回 false。
    bool solvePolynomial(double* a, int m, int n,
        const double* b, double* x) const;

    // ── 透视校正 ───────────────────────────────────────────────
    // 对当前画面做四点透视变换并校正为 DISPLAY_W × DISPLAY_H 的俯视矩形。
    // m_perspectiveCorners 须按 左上、右上、右下、左下 顺序存放源角点，
    // 成功返回 true。
    bool applyPerspectiveCorrection();

private:
    // 状态指示条：红表示未完成标定，绿表示已完成
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
    bool           m_isSaved;       // 标定数据是否已写入磁盘
    bool           m_needResetDC;   // 结果预览是否需要重绘

    // ── 数据 ────────────────────────────────────────────────────
    QVector<QPoint> m_points;            // 已点击的像素控制点
    QVector<QPoint> m_perspectiveCorners;// 透视变换的源角点（可选）
    DisplayDlg* m_pDispDlg;
    QImage          m_resultImage;       // 结果预览位图，由 onButtonShowRes 渲染

    // 场地边界多边形，13 个顶点，单位为厘米
    QPoint          point[13];
};
