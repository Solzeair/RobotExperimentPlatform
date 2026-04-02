#include "QtWidgetDesign.h"

QtWidgetDesign::QtWidgetDesign(QWidget *parent)
    : CFrameLessWidgetBase(parent)
{
    // 修改基类的标题
    setWindowTitleText("My Software V1.0");
    this->setMinimumSize(800, 600);

    // 获取基类的中心画板
    QWidget* pCenter = this->getCentralWidget();
    pCenter->setStyleSheet("background-color: #F0F0F0;"); // 设置你的主窗口底色

    // 在中心画板上添加你的业务 UI
    QVBoxLayout* pLay = new QVBoxLayout(pCenter);
    QLabel* pLabel = new QLabel("这里放置 MainWidget 的具体内容！", pCenter);
    pLabel->setAlignment(Qt::AlignCenter);
    pLay->addWidget(pLabel);





    /*
	tabWidget = new QTabWidget(this);  // 创建分页标签容器
	tabWidget->setFixedSize(800, 600); // 设置分页标签容器的大小 // 修改

    tabWidget->setStyleSheet(         // 设置标签页样式表，注意单位em
        "QTabBar::tab {"
        "   height: 2em;"             // 设置固定高度
        "   width: 6em;"              // 设置固定宽度
        "   background: transparent;" // 强制背景保持透明
        "   border: none;"            // 去掉可能存在的边框
        "}"

		"QTabBar::tab:selected {"     // 选中标签加深颜色
        "   font-weight: bold;"
        "}"
    );

	tabWidget->addTab(new QWidget(), "Camera"); // 摄像头
	tabWidget->addTab(new QWidget(), "Frequency"); // 频率
	tabWidget->addTab(new QWidget(), "Demarcate"); // 标定
	tabWidget->addTab(new QWidget(), "Color"); // 采色
	tabWidget->addTab(new QWidget(), "competition"); // 比赛
    */

}

QtWidgetDesign::~QtWidgetDesign()
{}

void QtWidgetDesign::closeEvent(QCloseEvent* event)
{
    QMessageBox::StandardButton _exit = QMessageBox::warning(this, "tip", "make sure to exit?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (_exit == QMessageBox::Yes) {
        event->accept(); // 同意关闭程序
    }
    else {
        event->ignore(); // 拦截关闭指令
    }
}