#include "CFrameLessWidgetBase.h"
#include <QEvent>
#include <qt_windows.h>
#include <windows.h>
#include <windowsx.h>

#pragma comment(lib, "user32.lib")

CFrameLessWidgetBase::CFrameLessWidgetBase(QWidget* parent)
    : QWidget(parent)
{
    // 去除系统边框，保留系统最小化、最大化原生动画机制
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_Hover);

    initBaseUI();
}

CFrameLessWidgetBase::~CFrameLessWidgetBase()
{
}

void CFrameLessWidgetBase::initBaseUI()
{
    // 1. 创建标题栏容器
    m_pTitleBarWidget = new QWidget(this);
    m_pTitleBarWidget->setAttribute(Qt::WA_StyledBackground);
    m_pTitleBarWidget->setFixedHeight(32 + 5 * 2);
    m_pTitleBarWidget->setStyleSheet("background-color:rgb(2, 230, 150)");

    // 2. 实例化标题栏控件
    m_pLogo = new QLabel(m_pTitleBarWidget);
    m_pLogo->setFixedSize(32, 32);
    m_pLogo->setStyleSheet("border-image: url(:/QtWidgetDesign/resources/xsyu_logo.svg); border: none;");

    m_pTitleTextLabel = new QLabel(m_pTitleBarWidget);
    m_pTitleTextLabel->setText("title");
    m_pTitleTextLabel->setFixedWidth(120);
    m_pTitleTextLabel->setStyleSheet("QLabel{font-family: Microsoft YaHei; font-size:18px; color:#BDC8E2;background-color:rgb(54,54,54);}");

    m_pSetBtn = new QPushButton(m_pTitleBarWidget);
    m_pSetBtn->setFixedSize(32, 32);
    m_pSetBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/set.svg);border:none}"
        "QPushButton:hover{background-color:rgb(99, 99, 99);background-image:url(:/QtWidgetDesign/resources/set_hover.svg);border:none;}");

    m_pMinBtn = new QPushButton(m_pTitleBarWidget);
    m_pMinBtn->setFixedSize(32, 32);
    m_pMinBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/min.svg);border:none}"
        "QPushButton:hover{background-color:rgb(99, 99, 99);background-image:url(:/QtWidgetDesign/resources/min_hover.svg);border:none;}");

    m_pMaxBtn = new QPushButton(m_pTitleBarWidget);
    m_pMaxBtn->setFixedSize(32, 32);
    m_pMaxBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/normal.svg);border:none}"
        "QPushButton:hover{background-color:rgb(99, 99, 99);background-image:url(:/QtWidgetDesign/resources/normal_hover.svg);border:none;}");

    m_pCloseBtn = new QPushButton(m_pTitleBarWidget);
    m_pCloseBtn->setFixedSize(32, 32);
    m_pCloseBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/close.svg);border:none}"
        "QPushButton:hover{background-color:rgb(99, 99, 99);background-image:url(:/QtWidgetDesign/resources/close_hover.svg);border:none;}");

    // 3. 布局标题栏
    QHBoxLayout* pTitleLayout = new QHBoxLayout(m_pTitleBarWidget);
    pTitleLayout->addWidget(m_pLogo);
    pTitleLayout->addWidget(m_pTitleTextLabel);
    pTitleLayout->addStretch();
    pTitleLayout->addWidget(m_pSetBtn);
    pTitleLayout->addSpacing(20);
    pTitleLayout->addWidget(m_pMinBtn);
    pTitleLayout->addSpacing(18);
    pTitleLayout->addWidget(m_pMaxBtn);
    pTitleLayout->addSpacing(18);
    pTitleLayout->addWidget(m_pCloseBtn);
    pTitleLayout->setContentsMargins(5, 5, 5, 5);

    // 4. 创建中心部件（给子类用的）
    m_pCentralWidget = new QWidget(this);

    // 5. 整体垂直布局
    QVBoxLayout* pMainLayout = new QVBoxLayout(this);
    pMainLayout->setContentsMargins(0, 0, 0, 0); // 必须是0
    pMainLayout->setSpacing(0);
    pMainLayout->addWidget(m_pTitleBarWidget);
    pMainLayout->addWidget(m_pCentralWidget);
    this->setLayout(pMainLayout);

    // 6. 信号槽连接
    connect(m_pMinBtn, &QPushButton::clicked, this, &CFrameLessWidgetBase::onTitleBarButtonClicked);
    connect(m_pMaxBtn, &QPushButton::clicked, this, &CFrameLessWidgetBase::onTitleBarButtonClicked);
    connect(m_pCloseBtn, &QPushButton::clicked, this, &CFrameLessWidgetBase::onTitleBarButtonClicked);
}

void CFrameLessWidgetBase::setWindowTitleText(const QString& text)
{
    if (m_pTitleTextLabel) {
        m_pTitleTextLabel->setText(text);
    }
}

void CFrameLessWidgetBase::onTitleBarButtonClicked()
{
    QPushButton* pButton = qobject_cast<QPushButton*>(sender());

    if (pButton == m_pMinBtn) {
        this->showMinimized();
    }
    else if (pButton == m_pMaxBtn) {
        if (this->isMaximized()) {
            this->showNormal();
        }
        else {
            this->showMaximized();
        }
    }
    else if (pButton == m_pCloseBtn) {
        emit sig_close();
        this->close(); // 默认调用关闭，子类也可通过重写 closeEvent 拦截
    }
}

void CFrameLessWidgetBase::updateMaximizeButtonState(bool isMaximized)
{
    if (isMaximized) {
        m_pMaxBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/normal.svg);border:none}"
            "QPushButton:hover{background-color:rgb(99, 99, 99);background-image:url(:/QtWidgetDesign/resources/normal_hover.svg);border:none;}");
    }
    else {
        m_pMaxBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/max.svg);border:none}"
            "QPushButton:hover{background-color:rgb(99, 99, 99);background-image:url(:/QtWidgetDesign/resources/max_hover.svg);border:none;}");
    }
}

bool CFrameLessWidgetBase::event(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        updateMaximizeButtonState(this->isMaximized());
    }
    return QWidget::event(event);
}

bool CFrameLessWidgetBase::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    MSG* param = static_cast<MSG*>(message);

    if (param->message == WM_NCHITTEST)
    {
        QPoint globalPos = QCursor::pos();
        QPoint localPos = this->mapFromGlobal(globalPos);
        int nX = localPos.x();
        int nY = localPos.y();

        // 1. 边缘检测（缩放）
        bool bLeft = nX < m_nBorderWidth;
        bool bRight = nX > this->width() - m_nBorderWidth;
        bool bTop = nY < m_nBorderWidth;
        bool bBottom = nY > this->height() - m_nBorderWidth;

        if (bLeft && bTop) { *result = HTTOPLEFT; return true; }
        if (bRight && bTop) { *result = HTTOPRIGHT; return true; }
        if (bLeft && bBottom) { *result = HTBOTTOMLEFT; return true; }
        if (bRight && bBottom) { *result = HTBOTTOMRIGHT; return true; }
        if (bLeft) { *result = HTLEFT; return true; }
        if (bRight) { *result = HTRIGHT; return true; }
        if (bBottom) { *result = HTBOTTOM; return true; }
        if (bTop) { *result = HTTOP; return true; }

        // 2. 标题栏区域检测（拖拽完美解决方案）
        if (m_pTitleBarWidget && m_pTitleBarWidget->geometry().contains(localPos)) {
            // 获取鼠标在标题栏内的相对坐标
            QPoint titleBarPos = m_pTitleBarWidget->mapFrom(this, localPos);
            QWidget* child = m_pTitleBarWidget->childAt(titleBarPos);
            // 如果鼠标下没有子控件（比如不在最大化、关闭按钮上），则允许拖拽
            if (!child || !qobject_cast<QPushButton*>(child)) {
                *result = HTCAPTION;
                return true;
            }
        }
    }
    return QWidget::nativeEvent(eventType, message, result);
}