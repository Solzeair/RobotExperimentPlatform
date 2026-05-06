#include "QtWidgetDesign.h"
#include "CameraDlg.h"
#include "RobotDlg.h"
#include "MatchDlg_5vs5.h"
#include "DisplayDlg.h"
#include "Debug.h"
#include "PluginManager.h"
#include "PluginInterface.h"
#include <QTextEdit>
#include <QPushButton>
#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>

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
    
    // 获取插件管理器实例
    PluginManager* pluginManager = PluginManager::getInstance();
    
    // 创建标定标签页（界面由插件提供）
    QWidget* demarcateWidget = new QWidget(myTabWidget);
    myTabWidget->addTab(demarcateWidget, "Demarcate");   // 标定标签始终显示
    
    // 尝试加载标定插件
    if (!pluginManager->isPluginLoaded(PluginType::DEMARCATE)) {
        qDebug() << "Loading Demarcate plugin...";
        pluginManager->loadPlugin("DemarcatePlugin.dll", PluginType::DEMARCATE);
    }
    
    // 如果插件加载成功，替换为插件界面
    QWidget* demarcatePluginWidget = pluginManager->createPluginWidget(PluginType::DEMARCATE, demarcateWidget);
    if (demarcatePluginWidget) {
        // 移除空白widget，使用插件widget
        delete demarcateWidget;
        int index = myTabWidget->indexOf(demarcateWidget);
        myTabWidget->removeTab(index);
        myTabWidget->insertTab(index, demarcatePluginWidget, "Demarcate");
    } else {
        // 插件未加载，显示提示信息
        QVBoxLayout* layout = new QVBoxLayout(demarcateWidget);
        QLabel* label = new QLabel("标定插件未加载，请确保 DemarcatePlugin.dll 存在", demarcateWidget);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: red; font-size: 14px;");
        layout->addWidget(label);
        demarcateWidget->setLayout(layout);
    }
    
    // 创建采色标签页（界面由插件提供）
    QWidget* colorWidget = new QWidget(myTabWidget);
    myTabWidget->addTab(colorWidget, "Color");       // 采色标签始终显示
    
    // 尝试加载采色插件
    if (!pluginManager->isPluginLoaded(PluginType::COLOR)) {
        qDebug() << "Loading Color plugin...";
        pluginManager->loadPlugin("ColorPlugin.dll", PluginType::COLOR);
    }
    
    // 如果插件加载成功，替换为插件界面
    QWidget* colorPluginWidget = pluginManager->createPluginWidget(PluginType::COLOR, colorWidget);
    if (colorPluginWidget) {
        // 移除空白widget，使用插件widget
        delete colorWidget;
        int index = myTabWidget->indexOf(colorWidget);
        myTabWidget->removeTab(index);
        myTabWidget->insertTab(index, colorPluginWidget, "Color");
    } else {
        // 插件未加载，显示提示信息
        QVBoxLayout* layout = new QVBoxLayout(colorWidget);
        QLabel* label = new QLabel("采色插件未加载，请确保 ColorPlugin.dll 存在", colorWidget);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: red; font-size: 14px;");
        layout->addWidget(label);
        colorWidget->setLayout(layout);
    }
    
    // 设置DisplayDlg指针给插件
    pluginManager->setDisplayDlgForPlugins(displayDlg);
    
    MatchDlg_5vs5* matchDlg = new MatchDlg_5vs5(myTabWidget);
    matchDlg->setDisplayDlg(displayDlg);  // 设置DisplayDlg指针
    myTabWidget->addTab(matchDlg, "competition"); // 比赛
    
    // 连接标签页切换信号
    connect(myTabWidget, &QTabWidget::currentChanged, [=](int index) {
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
        // 强制立即更新显示区域，减少切换延迟
        displayDlg->update();
        // 处理所有待处理事件，确保界面响应
        QCoreApplication::processEvents();
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
}

QtWidgetDesign::~QtWidgetDesign()
{
}
