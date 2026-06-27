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
    this->setMinimumSize(1400, 700);

    // 基于屏幕可用区（扣除任务栏）居中，避免标题栏被任务栏遮挡
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenRect = screen->availableGeometry();
        int x = (screenRect.width() - this->width()) / 2;
        int y = (screenRect.height() - this->height()) / 2;
        this->move(x, y);
    }

    this->setWindowTitleText("XSYU Football Robot Experimental Platform");

    // 主布局：左侧为相机显示区，右侧为功能标签页，二者按 stretch 分配剩余空间
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->setSpacing(10);

    // 左侧显示区固定尺寸，保证相机画面比例与场地标定坐标不随窗口缩放而失真
    DisplayDlg* displayDlg = new DisplayDlg(this);
    displayDlg->setFixedSize(640, 512);

    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(10);
    leftLayout->addWidget(displayDlg);

    // 调试信息区：可伸缩填满左侧剩余空间，供各模块输出运行日志
    QTextEdit* debugText = new QTextEdit(this);
    debugText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    debugText->setMinimumHeight(80);
    debugText->setReadOnly(true);
    debugText->setStyleSheet("font-family: Consolas; font-size: 10pt;");
    leftLayout->addWidget(debugText);

    QHBoxLayout* debugButtonLayout = new QHBoxLayout();
    debugButtonLayout->addStretch();
    QPushButton* cleanDebugButton = new QPushButton("Clean", this);
    cleanDebugButton->setFixedSize(80, 30);
    debugButtonLayout->addWidget(cleanDebugButton);
    leftLayout->addLayout(debugButtonLayout);

    connect(cleanDebugButton, &QPushButton::clicked, [=]() {
        Debug::get()->clean();
    });

    // 左侧整体不拉伸，保持显示区紧凑
    mainLayout->addLayout(leftLayout, 0);

    // 右侧标签页：承载相机、采色、标定等各功能模块
    QTabWidget* myTabWidget = new QTabWidget(this);
    myTabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    myTabWidget->setStyleSheet(
        "QTabBar::tab {"
        "   height: 2em;"
        "   width: 6em;"
        "   background: transparent;"
        "   border: none;"
        "}"
        "\n"
        "QTabBar::tab:selected {"
        "   font-weight: bold;"
        "}"
    );

    CameraDlg* cameraDlg = new CameraDlg(myTabWidget);
    myTabWidget->addTab(cameraDlg, "Camera");

    RobotDlg* robotDlg = new RobotDlg(myTabWidget);
    myTabWidget->addTab(robotDlg, "Frequency");

    // 标定页与显示页相互持有引用：标定需在校正后的画面上取点，显示页需转发鼠标点击
    DemarcateDlg* demarcateDlg = new DemarcateDlg(myTabWidget);
    demarcateDlg->setDisplayDlg(displayDlg);
    displayDlg->setDemarcateDlg(demarcateDlg);
    myTabWidget->addTab(demarcateDlg, "Demarcate");
    m_pDemarcateDlg = demarcateDlg;

    ColorDlg* colorDlg = new ColorDlg(myTabWidget);
    myTabWidget->addTab(colorDlg, "Color");
    m_pColorDlg = colorDlg;
    displayDlg->setColorDlg(colorDlg);

    MatchDlg_5vs5* matchDlg = new MatchDlg_5vs5(myTabWidget);
    matchDlg->setDisplayDlg(displayDlg);
    myTabWidget->addTab(matchDlg, "competition");

    // 标签页切换时切换显示页的工作模式：先停定时器再按页设定，用共享守卫避免切换中重入
    auto tabSwitching = std::make_shared<bool>(false);
    connect(myTabWidget, &QTabWidget::currentChanged, [=](int index) {
        if (*tabSwitching) return;
        *tabSwitching = true;

        displayDlg->Stop();

        switch (index) {
        case 0:
            displayDlg->ShowDynamic();
            break;
        case 1:
            displayDlg->ShowCarNum();
            break;
        case 2:
            displayDlg->ShowSingle();
            break;
        case 3:
            displayDlg->ShowSingle();
            displayDlg->SelectSetStatus(DisplayDlg::SET_STATUS::COLOR_SET);
            break;
        case 4:
            displayDlg->ShowInitGame();
            break;
        }
        displayDlg->update();

        *tabSwitching = false;
        });

    mainLayout->addWidget(myTabWidget, 1);

    Debug::get()->init(debugText);

    QWidget* centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);
    centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setCentralWidget(centralWidget);

    // 启动后立即进入 Camera 页，触发相机开始抓流
    emit myTabWidget->currentChanged(0);
}

QtWidgetDesign::~QtWidgetDesign()
{
    Debug::get()->init(nullptr);
}

void QtWidgetDesign::closeEvent(QCloseEvent* event)
{
    // 关闭前逐页检查未保存数据：Save 则落盘，Cancel 中止关闭，Discard 跳过继续下一页
    if (m_pDemarcateDlg && m_pDemarcateDlg->hasUnsavedData()) {
        QMessageBox::StandardButton ret = QMessageBox::question(
            this,
            "提示",
            "标定数据尚未保存，是否保存？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);

        if (ret == QMessageBox::Save) {
            m_pDemarcateDlg->saveCalibration();
        }
        else if (ret == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    if (m_pColorDlg && m_pColorDlg->hasUnsavedData()) {
        QMessageBox::StandardButton ret = QMessageBox::question(
            this,
            "提示",
            "颜色信息未保存，是否保存？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);

        if (ret == QMessageBox::Save) {
            m_pColorDlg->saveData();
        }
        else if (ret == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    CFrameLessWidgetBase::closeEvent(event);
}
