#include "QtWidgetDesign.h"
#include "CameraDlg.h"
#include "RobotDlg.h"
#include "DemarcateDlg.h"
#include "ColorDlg.h"
#include "MatchDlg_5vs5.h"
#include "DisplayDlg.h"
#include "Debug.h"
#include <QTextEdit>
#include <QPushButton>
#include <memory>

QtWidgetDesign::QtWidgetDesign(QWidget* parent)
    : CFrameLessWidgetBase(parent)
{
    // 保持主窗口的宽度不变，只减小高度
    this->setMinimumSize(1200, 700);

    // 将窗口移动到屏幕正中央
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        // 获取屏幕的可用几何尺寸（即扣除 Windows 底部任务栏后的实际可用范围）
        QRect screenRect = screen->availableGeometry();
        // 计算居中坐标：(屏幕宽高 - 窗口宽高) / 2
        int x = (screenRect.width() - this->width()) / 2;
        int y = (screenRect.height() - this->height()) / 2;
        // 移动窗口到计算出的中心坐标
        this->move(x, y);
    }

    // 调用父类的方法设置标题栏文本
    this->setWindowTitleText("XSYU Football Robot Experimental Platform"); // XSYU 足球机器人 实验平台

    // 创建主布局
    QHBoxLayout* mainLayout = new QHBoxLayout();

    // 左侧显示区域 - 与MFC版本保持一致
    DisplayDlg* displayDlg = new DisplayDlg(this);
    displayDlg->setFixedSize(640, 512);  // 480显示区域 + 32帧率标签（与右侧标签栏高度一致）

    // 创建垂直布局，使DisplayDlg在垂直方向上偏上
    QVBoxLayout* leftLayout = new QVBoxLayout();
    // 移除顶部的stretch，减少顶部空白
    leftLayout->addWidget(displayDlg);
    leftLayout->addStretch(2);  // 底部空白（较大比例）

    mainLayout->addLayout(leftLayout);

    // 右侧标签页控件
    QTabWidget* myTabWidget = new QTabWidget(this);

    // 设置标签页为可伸缩，充满右侧整个界面
    myTabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 移除最小尺寸设置，避免遮挡左侧显示区域
    // myTabWidget->setMinimumSize(560, 800);

    myTabWidget->setStyleSheet(
        "QTabBar::tab {"
        "   height: 2em;"             // 设置逻辑高度
        "   width: 6em;"              // 设置逻辑宽度
        "   background: transparent;" // 强制背景保持透明
        "   border: none;"            // 去掉可能存在的边框
        "}"
        "\n"
        "QTabBar::tab:selected {"     // 选中标签加深颜色
        "   font-weight: bold;"
        "}"
    );

    // 添加标签  
    CameraDlg* cameraDlg = new CameraDlg(myTabWidget);
    myTabWidget->addTab(cameraDlg, "Camera");      // 摄像头

    RobotDlg* robotDlg = new RobotDlg(myTabWidget);
    myTabWidget->addTab(robotDlg, "Frequency");   // 频率

    DemarcateDlg* demarcateDlg = new DemarcateDlg(myTabWidget);
    demarcateDlg->setDisplayDlg(displayDlg);  // Give Demarcate access to the display
    displayDlg->setDemarcateDlg(demarcateDlg); // Give Display access to Demarcate (for click forwarding)
    myTabWidget->addTab(demarcateDlg, "Demarcate");   // 标定
    m_pDemarcateDlg = demarcateDlg;  // 保存指针，用于退出时检查未保存数据

    ColorDlg* colorDlg = new ColorDlg(myTabWidget);
    myTabWidget->addTab(colorDlg, "Color");       // 采色

    MatchDlg_5vs5* matchDlg = new MatchDlg_5vs5(myTabWidget);
    matchDlg->setDisplayDlg(displayDlg);  // 设置DisplayDlg指针
    myTabWidget->addTab(matchDlg, "competition"); // 比赛

    // 连接标签页切换信号（防重入守卫）
    auto tabSwitching = std::make_shared<bool>(false);
    connect(myTabWidget, &QTabWidget::currentChanged, [=](int index) {
        if (*tabSwitching) return;
        *tabSwitching = true;

        // 先停止所有定时器，避免冲突
        displayDlg->Stop();

        switch (index) {
        case 0: // Camera标签页
            displayDlg->ShowDynamic();
            break;
        case 1: // Frequency标签页
            displayDlg->ShowCarNum();
            break;
        case 2: // Demarcate标签页
            displayDlg->ShowSingle();
            break;
        case 3: // Color标签页
            displayDlg->ShowSingle();
            displayDlg->SelectSetStatus(DisplayDlg::SET_STATUS::COLOR_SET);
            break;
        case 4: // competition标签页
            displayDlg->ShowInitGame();
            break;
        }
        displayDlg->update();

        *tabSwitching = false;
        });

    // 将标签页添加到主布局
    mainLayout->addWidget(myTabWidget);

    // 创建调试信息文本框
    QTextEdit* debugText = new QTextEdit(this);
    debugText->setFixedSize(640, 100);
    debugText->setReadOnly(true);
    debugText->setStyleSheet("font-family: Consolas; font-size: 10pt;");

    // 创建清除调试信息按钮
    QPushButton* cleanDebugButton = new QPushButton("Clean", this);
    cleanDebugButton->setFixedSize(80, 30);

    // 连接清除按钮信号
    connect(cleanDebugButton, &QPushButton::clicked, [=]() {
        Debug::get()->clean();
        });

    // 创建调试信息布局
    QVBoxLayout* debugLayout = new QVBoxLayout();
    debugLayout->addWidget(debugText);
    debugLayout->addWidget(cleanDebugButton, 0, Qt::AlignRight);

    // 将调试布局添加到左侧布局
    leftLayout->addLayout(debugLayout);

    // 初始化Debug类
    Debug::get()->init(debugText);

    // 创建一个中心部件来容纳主布局
    QWidget* centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);

    // 3. 核心最后一步：调用基类提供的接口，让基类把它加到主界面布局中去
    this->setCentralWidget(centralWidget);

    // 手动触发一次下标为0的槽函数，确保相机在软件启动时开始抓流
    emit myTabWidget->currentChanged(0);
}

QtWidgetDesign::~QtWidgetDesign()
{
    Debug::get()->init(nullptr);
}

void QtWidgetDesign::closeEvent(QCloseEvent* event)
{
    // 检查标定页面是否有未保存的数据
    if (m_pDemarcateDlg && m_pDemarcateDlg->hasUnsavedData()) {
        QMessageBox::StandardButton ret = QMessageBox::question(
            this,
            "提示",
            "标定数据尚未保存，是否保存？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);

        if (ret == QMessageBox::Save) {
            // 保存标定数据
            m_pDemarcateDlg->saveCalibration();
            // 保存后继续执行基类关闭事件
            CFrameLessWidgetBase::closeEvent(event);
        }
        else if (ret == QMessageBox::Discard) {
            // 不保存，继续关闭
            CFrameLessWidgetBase::closeEvent(event);
        }
        else {
            // Cancel：取消关闭
            event->ignore();
        }
    }
    else {
        // 没有未保存数据，正常执行基类关闭事件
        CFrameLessWidgetBase::closeEvent(event);
    }
}
