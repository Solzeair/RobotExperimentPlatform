/*
* 摄像头调整对话框
* 提供亮度、增益、对比度、快门、红/绿/蓝等参数的滑块与输入框，并通过保存按钮持久化设置
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
    CameraDlg(QWidget* parent = nullptr);       // 构建并初始化摄像头调参界面
    ~CameraDlg();                               // 释放对话框相关资源

private slots:
    void onSliderBlackLevelChanged(int value);  // 黑电平滑块变更，实时下发并回填输入框
    void onSliderGainChanged(int value);        // 增益滑块变更
    void onSliderGammaChanged(int value);       // Gamma（对比度）滑块变更
    void onSliderShutterChanged(int value);     // 快门滑块变更
    void onSliderRedChanged(int value);         // 红色通道增益滑块变更
    void onSliderGreenChanged(int value);       // 绿色通道增益滑块变更
    void onSliderBlueChanged(int value);        // 蓝色通道增益滑块变更
    void onSaveCamera();                        // 保存当前参数到相机持久配置
    void onEditReturnPressed();                 // 输入框回车提交，反向同步滑块与相机

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void initUI();

private:
    // 界面布局
    QVBoxLayout *mainLayout;  // 主垂直布局，承载各参数行
    

    QSlider *sliderBlackLevel;  // 黑电平（亮度基准）滑块
    QSlider *sliderGain;        // 增益滑块
    QSlider *sliderGamma;       // Gamma 曲线（对比度）滑块
    QSlider *sliderShutter;     // 快门时间滑块
    QSlider *sliderRed;         // 红色通道增益滑块
    QSlider *sliderGreen;       // 绿色通道增益滑块
    QSlider *sliderBlue;        // 蓝色通道增益滑块
    
    QLineEdit *editBlackLevel;  // 黑电平数值输入，与滑块双向同步
    QLineEdit *editGain;        // 增益数值输入
    QLineEdit *editGamma;       // Gamma 数值输入
    QLineEdit *editShutter;     // 快门数值输入
    QLineEdit *editRed;         // 红色通道增益数值输入
    QLineEdit *editGreen;       // 绿色通道增益数值输入
    QLineEdit *editBlue;        // 蓝色通道增益数值输入
    
    QPushButton *btnSaveCamera; // 保存按钮
    
    // 滑块当前整型值（界面显示用）
    int slideBlackLevel;        // 黑电平滑块值
    int slideGain;              // 增益滑块值
    int slideGamma;             // Gamma 滑块值
    int slideShutter;           // 快门滑块值
    int slideRed;               // 红色通道滑块值
    int slideGreen;             // 绿色通道滑块值
    int slideBlue;              // 蓝色通道滑块值

    // 下发给相机的实际物理参数（单位与相机接口一致）
    double blackLevel;          // 黑电平
    double gain;                // 增益
    double gamma;               // Gamma
    int shutter;                // 快门时间
    double red;                 // 红色通道增益
    double green;               // 绿色通道增益
    double blue;                // 蓝色通道增益
    
    // 相机实例
    Camera * _pCamera;
    

};