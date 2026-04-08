#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QMessageBox>

class DemarcateDlg : public QWidget
{
    Q_OBJECT

public:
    DemarcateDlg(QWidget *parent = nullptr);
    ~DemarcateDlg();

private slots:
    void onButtonSet();
    void onButtonResetOne();
    void onButtonReset();
    void onButtonLoad();
    void onButtonSave();
    void onButtonFlush();
    void onButtonShowRes();

private:
    void initUI();

private:
    // 布局
    QVBoxLayout *mainLayout;
    
    // 控件
    QLabel *resultLabel;
    QProgressBar *progressBar;
    QPushButton *btnSet;
    QPushButton *btnResetOne;
    QPushButton *btnReset;
    QPushButton *btnLoad;
    QPushButton *btnSave;
    QPushButton *btnFlush;
    QPushButton *btnShowRes;
    
    // 数据
    bool m_isSaved;
    bool m_needResetDC;
    QVector<QPoint> m_points;
    
    // 场地边界点
    QPoint point[13];
};