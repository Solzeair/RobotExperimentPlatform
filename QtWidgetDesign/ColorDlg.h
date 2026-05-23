/*
* 采色对话框头文件
* 写作人 李青
* 功能 采色界面的类定义，包含HSI阈值调节滑块、颜色测试按钮和色环显示等属性和方法
* 未完成
*/
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QButtonGroup>
#include <QPainter>
#include <QColor>
#include <cmath>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QPoint>
#include <QPixmap>
#include <QPainterPath>

// --- 范围常量 ---
static const int H_RANGE_MAX   = 3600;
static const int S_RANGE_MAX   = 100;
static const int I_RANGE_MAX   = 255;
static const int H_SAMPLE_OFFSET = 100;  // H 采样容差 (对应 10°)
static const int S_SAMPLE_OFFSET = 20;   // S 采样容差 20%
static const int I_SAMPLE_OFFSET = 20;   // I 采样容差 20

class ColorDlg : public QWidget
{
    Q_OBJECT
public:
    ColorDlg(QWidget *parent = nullptr); // 初始化采色界面
    ~ColorDlg();
    static ColorDlg* getInstance(); // 获取单例实例
    const int(*getHSIThreshold())[6] { return HSIThreshold; } // 获取HSI阈值
    int currentObject() const { return m_object; }            // 获取当前选中对象
    bool hasUnsavedData() const { return !m_isSaved; }        // 是否有未保存的修改
    void saveData();                                          // 保存颜色数据（供外部调用）

public slots:
    void updateDisplayImage(const QPixmap& pixmap);
    void onZoom();
    void onSample();
    void onClearSamples();


private slots:
    void onButtonColorTest();             // 颜色测试按钮
    void onButtonRunTest();               // 动态测试按钮
    void onButtonStopTest();              // 单帧图像按钮
    void onButtonSave();                  // 保存按钮
    void onButtonLoad();                  // 加载按钮
    void onSegCheckBoxStateChanged(int);  // 图像分割复选框状态改变
    void onScrollBarChanged();            // 各项滚动条值改变

private:
    void initUI();                        // UI初始化
    void UpdateHSIThreshold();            // 更新HSI阈值（滑块→数组）
    void loadThresholdForObject(int obj); // 切换对象时加载阈值（数组→滑块）
    void drawHSIRing();                   // 绘制HSI颜色环（位图+叠加层）
    void drawHSIRingFallback();           // 程序绘制色环（位图加载失败时的回退）
    void drawBrightnessHistogram();       // 绘制亮度直方图

private:
    // --- 布局部件 ---
    QVBoxLayout* mainLayout;              // 主垂直布局容器指针

    // --- 控件部件 ---
    QLabel* localLabel;                   // 局部放大区域显示标签
    QLabel* HSIdisplayLabel;              // HSI颜色环显示标签
    QPushButton* stopButton;              // 单帧图像按钮
    QPushButton* colorTestButton;         // 颜色测试按钮
    QPushButton* runTestButton;           // 动态测试按钮
    QPushButton* colorLoadButton;         // 加载按钮
    QPushButton* colorSaveButton;         // 保存按钮
    QCheckBox* segCheckBox;               // 图像分割复选框
    QButtonGroup* m_objectGroup;          // 对象单选按钮组

    // 滚动条
    QScrollBar* scrollBarHMin;            // 色调下限调节滚动条
    QScrollBar* scrollBarHMax;            // 色调上限调节滚动条
    QScrollBar* scrollBarSMin;            // 饱和度下限调节滚动条
    QScrollBar* scrollBarSMax;            // 饱和度上限调节滚动条
    QScrollBar* scrollBarIMin;            // 亮度下限调节滚动条
    QScrollBar* scrollBarIMax;            // 亮度上限调节滚动条

    // --- 数据变量 ---
    bool m_SelectRect;                    // 是否处于矩形选择状态
    int m_H_High;                         // 色调上限
    int m_H_Low;                          // 色调下限
    int m_S_High;                         // 饱和度上限
    int m_S_Low;                          // 饱和度下限
    int m_I_High;                         // 亮度上限
    int m_I_Low;                          // 亮度下限
    int m_object;                         // 当前操作对象的标识
    bool m_ImageSeg;                      // 是否开启图像分割功能
    bool m_isSaved;                       // 参数是否已保存
    int HSIThreshold[8][6];               // 各颜色HSI阈值
    QVector<QPoint> m_vecColorSet;        // 采样颜色点集合
    QVector<QPoint> m_points;             // 点选模式采集的点（放大区坐标）
    int yi[255];                          // 特定映射数值
     // 声明成员变量
    QLabel* m_pDisplayLabel;
    QLabel* brightnessGraphLabel;       // 亮度直方图显示标签
    QPixmap m_lastFrame;                  // 最近一帧（未缩放），用于准确采样
    QRect   m_zoomSourceRect;             // 放大操作对应的原始图像矩形
    // 交互选择状态
    bool m_selecting = false;             // 正在拖拽选择矩形
    QPoint m_selectStart;                 // 选择起点（label 坐标）
    QRect m_currentRect;                  // 当前选择矩形（label 坐标）

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void redrawPreview();                 // 在 m_pDisplayLabel 上重绘预览及覆盖层
    void sampleAtImageRect(const QRect& imgRect); // 在原始图像坐标的矩形上采样
    void sampleAtImagePoint(const QPoint& imgPt); // 在原始图像坐标的点附近采样
    bool isPixelMatchingThreshold(int R, int G, int B) const; // 判断像素是否匹配当前 HSI 阈值

};