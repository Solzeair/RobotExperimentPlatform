#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

class CameraDlg : public QWidget
{
    Q_OBJECT

public:
    CameraDlg(QWidget *parent = nullptr);
    ~CameraDlg();

private slots:
    void onSliderBlackLevelChanged(int value);
    void onSliderGainChanged(int value);
    void onSliderGammaChanged(int value);
    void onSliderShutterChanged(int value);
    void onSliderRedChanged(int value);
    void onSliderGreenChanged(int value);
    void onSliderBlueChanged(int value);
    void onSaveCamera();
    void onEditReturnPressed();

private:
    void initUI();

private:
    // 布局
    QVBoxLayout *mainLayout;
    
    // 控件
    QLabel *cameraViewLabel;   // 摄像头显示区域
    QLabel *outputLabel;       // 输出区域
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
    
    // 数据
    int slideBlackLevel;
    int slideGain;
    int slideGamma;
    int slideShutter;
    int slideRed;
    int slideGreen;
    int slideBlue;
    
    double blackLevel;
    double gain;
    double gamma;
    int shutter;
    double red;
    double green;
    double blue;
};