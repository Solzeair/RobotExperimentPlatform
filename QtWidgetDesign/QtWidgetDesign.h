#pragma once

#include <QtWidgets/QWidget>
#include "qtabwidget.h"
#include "CFrameLessWidgetBase.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QCloseEvent>

// 主窗口继承无边框基类
class QtWidgetDesign : public CFrameLessWidgetBase
{
    Q_OBJECT

public:
    QtWidgetDesign(QWidget *parent = nullptr);
    ~QtWidgetDesign();

protected:
    // 重写关闭事件，实现弹窗二次确认
    void closeEvent(QCloseEvent* event) override;

private:
	QTabWidget* tabWidget; // 分页标签容器

};
