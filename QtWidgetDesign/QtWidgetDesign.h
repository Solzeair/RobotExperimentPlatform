#pragma once

#include <QtWidgets/QWidget>
#include "CFrameLessWidgetBase.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QTabWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QCloseEvent>

// 前向声明
class DemarcateDlg;

// 主窗口继承无边框基类
class QtWidgetDesign : public CFrameLessWidgetBase
{
    Q_OBJECT

public:
    QtWidgetDesign(QWidget *parent = nullptr);
    ~QtWidgetDesign();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    DemarcateDlg* m_pDemarcateDlg = nullptr;

};
