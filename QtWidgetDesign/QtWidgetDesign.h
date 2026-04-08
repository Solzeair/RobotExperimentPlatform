#pragma once

#include <QtWidgets/QWidget>
#include "CFrameLessWidgetBase.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QTabWidget>
#include <QScreen>
#include <QGuiApplication>

// 主窗口继承无边框基类
class QtWidgetDesign : public CFrameLessWidgetBase
{
    Q_OBJECT

public:
    QtWidgetDesign(QWidget *parent = nullptr);
    ~QtWidgetDesign();

private:
	// Qm_pCentralWidget* m_pCentralWidget; // 分页标签容器

};
