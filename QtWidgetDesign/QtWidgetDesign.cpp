#include "QtWidgetDesign.h"

QtWidgetDesign::QtWidgetDesign(QWidget* parent)
    : CFrameLessWidgetBase(parent)
{
    this->setMinimumSize(900, 600);
    // 调用父类的方法设置标题栏文本
    this->setWindowTitleText("XSYU Football Robot Experimental Platform"); // XSYU 足球机器人 实验平台

    // 创建 QTabWidget 类为主页面容器
    QTabWidget* myTabWidget = new QTabWidget(this);
    myTabWidget->setFixedSize(900, 600 - 32);

    myTabWidget->setStyleSheet(      
        "QTabBar::tab {"
        "   height: 2em;"             // 设置逻辑高度
        "   width: 6em;"              // 设置逻辑宽度
        "   background: transparent;" // 强制背景保持透明
        "   border: none;"            // 去掉可能存在的边框
        "}"

        "QTabBar::tab:selected {"     // 选中标签加深颜色
        "   font-weight: bold;"
        "}"
    );

    // 2. 添加标签  
    myTabWidget->addTab(new QWidget(), "Camera");      // 摄像头
    myTabWidget->addTab(new QWidget(), "Frequency");   // 频率
    myTabWidget->addTab(new QWidget(), "Demarcate");   // 标定
    myTabWidget->addTab(new QWidget(), "Color");       // 采色
    myTabWidget->addTab(new QWidget(), "competition"); // 比赛

    // 3. 核心最后一步：调用基类提供的接口，让基类把它加到主界面布局中去
    this->setCentralWidget(myTabWidget);
}

QtWidgetDesign::~QtWidgetDesign()
{
}