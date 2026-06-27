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

class DemarcateDlg;
class ColorDlg;

// 主窗口：组合相机显示区与功能标签页，负责标签页切换时的显示模式调度，
// 并在关闭前拦截未保存的标定/采色数据。继承无边框基类以获得自绘标题栏。
class QtWidgetDesign : public CFrameLessWidgetBase
{
    Q_OBJECT

public:
    QtWidgetDesign(QWidget *parent = nullptr);
    ~QtWidgetDesign();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    // 仅持有需要在关闭时做未保存检查的两个对话框；其余标签页由 QTabWidget 父子关系托管，无需显式管理生命周期
    DemarcateDlg* m_pDemarcateDlg = nullptr;
    ColorDlg*     m_pColorDlg     = nullptr;

};
