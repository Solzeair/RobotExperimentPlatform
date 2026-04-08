#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QGroupBox>

class ColorDlg : public QWidget
{
    Q_OBJECT

public:
    ColorDlg(QWidget *parent = nullptr);
    ~ColorDlg();

private slots:
    void onButtonColorTest();
    void onButtonRunTest();
    void onButtonStopTest();
    void onButtonSave();
    void onButtonLoad();
    void onSegCheckBoxStateChanged(int);
    void onScrollBarChanged();

private:
    void initUI();
    void UpdateHSIThreshold();
    void drawHSIRing();

private:
    // 布局
    QVBoxLayout *mainLayout;
    
    // 控件
    QLabel *localLabel;        // 局部放大区域
    QLabel *HSIdisplayLabel;   // HSI颜色环
    QPushButton *stopButton;
    QPushButton *colorTestButton;
    QPushButton *runTestButton;
    QPushButton *colorLoadButton;
    QPushButton *colorSaveButton;
    QCheckBox *segCheckBox;
    
    // 滚动条
    QScrollBar *scrollBarHMin;
    QScrollBar *scrollBarSMin;
    QScrollBar *scrollBarIMin;
    
    // 数据
    bool m_SelectRect;
    int m_H_High;
    int m_H_Low;
    int m_S_High;
    int m_S_Low;
    int m_I_High;
    int m_I_Low;
    int m_object;
    bool m_ImageSeg;
    bool m_isSaved;
    int HSIThreshold[8][6];
    QVector<QPoint> m_vecColorSet;
    int yi[255];
};