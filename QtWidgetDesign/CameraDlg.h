/*
* 摄像头调整对话框头文件
* 写作人 李青
* 功能 声明摄像头调整界面的类，包含参数滑块、输入框及相关槽函数的声明。
* 未完成
*/
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QKeyEvent>
#include <QEvent>
#include <QTimer>
#include <QElapsedTimer>

// 前向声明
class Camera;

class CameraDlg : public QWidget
{
    Q_OBJECT

public:
    CameraDlg(QWidget* parent = nullptr);       // 初始化摄像头界面的UI与数据
    ~CameraDlg();                               // 清理资源

private slots:
    void onSliderBlackLevelChanged(int value);  // 亮度滑块值改变
    void onSliderGainChanged(int value);        // 增益滑块值改变
    void onSliderGammaChanged(int value);       // 对比度滑块值改变
    void onSliderShutterChanged(int value);     // 快门滑块值改变
    void onSliderRedChanged(int value);         // 红色滑块值改变
    void onSliderGreenChanged(int value);       // 绿色滑块值改变
    void onSliderBlueChanged(int value);        // 蓝色滑块值改变
    void onSaveCamera();                        // 保存设置按钮点击
    void onEditReturnPressed();                 // 编辑框回车事件，用于同步滑块

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void initUI();

private:
    // 布局
    QVBoxLayout *mainLayout;  // 主垂直布局容器
    

    QSlider *sliderBlackLevel;  // 亮度滑块
    QSlider *sliderGain;        // 增益滑块
    QSlider *sliderGamma;       // 对比度滑块
    QSlider *sliderShutter;     // 快门滑块
    QSlider *sliderRed;         // 红色滑块
    QSlider *sliderGreen;       // 绿色滑块
    QSlider *sliderBlue;        // 蓝色滑块
    
    QLineEdit *editBlackLevel;  // 亮度输入框
    QLineEdit *editGain;        // 增益输入框
    QLineEdit *editGamma;       // 对比度输入框
    QLineEdit *editShutter;     // 快门输入框
    QLineEdit *editRed;         // 红色输入框
    QLineEdit *editGreen;       // 绿色输入框
    QLineEdit *editBlue;        // 蓝色输入框
    
    QPushButton *btnSaveCamera; // 保存按钮
    
    // --- 数据变量 ---
    int slideBlackLevel;        // 亮度滑块
    int slideGain;              // 增益滑块
    int slideGamma;             // 对比度滑块
    int slideShutter;           // 快门滑块
    int slideRed;               // 红色滑块
    int slideGreen;             // 绿色滑块
    int slideBlue;              // 蓝色滑块

    double blackLevel;          // 亮度参数
    double gain;                // 增益参数
    double gamma;               // 对比度参数
    int shutter;                // 快门参数
    double red;                 // 红色参数
    double green;               // 绿色参数
    double blue;                // 蓝色参数
    
    // 相机实例
    Camera * _pCamera;
    

};