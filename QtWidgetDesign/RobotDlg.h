#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QGroupBox>

class RobotDlg : public QWidget
{
    Q_OBJECT

public:
    RobotDlg(QWidget *parent = nullptr);
    ~RobotDlg();

private slots:
    void onButtonFront();
    void onButtonBack();
    void onButtonLeft();
    void onButtonRight();
    void onButtonStop();
    void onButtonChangeNum();
    void onButtonChangeFreq();

private:
    void initUI();

private:
    // 布局
    QVBoxLayout *mainLayout;
    
    // 控件
    QLineEdit *editOldNum;
    QLineEdit *editNewNum;
    QLineEdit *editNum;
    QLineEdit *editDeviceStatus;
    QPushButton *btnChangeNum;
    QPushButton *btnChangeFreq;
    QPushButton *btnFront;
    QPushButton *btnBack;
    QPushButton *btnLeft;
    QPushButton *btnRight;
    QPushButton *btnStop;
    QRadioButton *radio1_450;
    QRadioButton *radio1_460;
    QGroupBox *carNumGroup;
    QGroupBox *controlGroup;
    QGroupBox *carFreqGroup;
    QGroupBox *deviceGroup;
    
    // 数据
    int m_oldNum;
    int m_newNum;
    int m_numSet;
    bool m_carFre;
};