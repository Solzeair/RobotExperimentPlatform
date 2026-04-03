#include "CFrameLessWidgetBase.h"
#include <qt_windows.h>
#include <windows.h>

CFrameLessWidgetBase::CFrameLessWidgetBase(QWidget* parent)
    : QWidget(parent)
{
    // 去除系统边框，保留系统最小化、最大化原生动画机制
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_Hover);     // 开启窗口部件的悬停属性支持，使得控件可以响应鼠标悬停样式的改变

    // 加回系统的标题栏和调整大小边框，便于拉伸 
    HWND hwnd = (HWND)this->winId();                                                       // 获取当前窗口的原生Windows句柄
    DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);                                        // 获取窗口当前的原生样式
    ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION); // 强行附加最大化、拉伸、标题栏的属性

    initBaseUI();                   // 调用私有方法初始化自定义标题栏和整体布局
}

CFrameLessWidgetBase::~CFrameLessWidgetBase()
{
}

void CFrameLessWidgetBase::initBaseUI()
{
    // 1. 标题栏
    m_pTitleBarWidget = new QWidget(this);
    m_pTitleBarWidget->setAttribute(Qt::WA_StyledBackground); // 开启样式背景属性，允许通过QSS设置标题栏容器的背景色
	m_pTitleBarWidget->setFixedHeight(32);                    // 标题栏高度 32
    m_pTitleBarWidget->setStyleSheet("background-color:rgb(130, 177, 255)");

    // 左上角logo
    m_pLogo = new QLabel(m_pTitleBarWidget);
    m_pLogo->setFixedSize(32, 32);
    m_pLogo->setStyleSheet("background: transparent; border: none;");
    // 正确缩放SVG图标
    QIcon logoIcon(":/QtWidgetDesign/resources/xsyu_logo.svg");
    m_pLogo->setPixmap(logoIcon.pixmap(QSize(32, 32)));
    m_pLogo->setScaledContents(true); // 居中且平滑缩放

	// 标题文本
    m_pTitleTextLabel = new QLabel(m_pTitleBarWidget);
	m_pTitleTextLabel->setText("default title"); // 默认标题，子类通过方法 setWindowTitleText 修改
    m_pTitleTextLabel->setFixedWidth(280);
    m_pTitleTextLabel->setStyleSheet("QLabel{font-family: Microsoft YaHei; font-size:18px; color: #E54242;}");

	// 右侧按钮：设置
    m_pSetBtn = new QPushButton(m_pTitleBarWidget);
    m_pSetBtn->setFixedSize(32, 32);
    m_pSetBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/set.svg);border:none}"
        "QPushButton:hover{background-color:rgb(182, 209, 255);border:none;}");

    // 右侧按钮：缩小
    m_pMinBtn = new QPushButton(m_pTitleBarWidget);
    m_pMinBtn->setFixedSize(32, 32);
    m_pMinBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/min.svg);border:none}"
        "QPushButton:hover{background-color:rgb(182, 209, 255);border:none;}");

	// 右侧按钮：放大 当前显示为最大化图标，点击后会切换为还原图标
    m_pMaxBtn = new QPushButton(m_pTitleBarWidget);
    m_pMaxBtn->setFixedSize(32, 32);
    m_pMaxBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/normal.svg);border:none}"
        "QPushButton:hover{background-color:rgb(182, 209, 255);border:none;}");

	// 右侧按钮：关闭
    m_pCloseBtn = new QPushButton(m_pTitleBarWidget);
    m_pCloseBtn->setFixedSize(32, 32);
    m_pCloseBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/close.svg);border:none}"
        "QPushButton:hover{background-color:rgb(182, 209, 255);border:none;}");

    // 2. 布局标题栏
    QHBoxLayout* pTitleLayout = new QHBoxLayout(m_pTitleBarWidget);
    pTitleLayout->addWidget(m_pLogo);
    pTitleLayout->addWidget(m_pTitleTextLabel);
    pTitleLayout->addStretch();                   // 添加弹簧挤压控件
    pTitleLayout->addWidget(m_pSetBtn);
    pTitleLayout->addSpacing(10);                 // 增加留白间距
    pTitleLayout->addWidget(m_pMinBtn);
    pTitleLayout->addSpacing(10);
    pTitleLayout->addWidget(m_pMaxBtn);
    pTitleLayout->addSpacing(10);
    pTitleLayout->addWidget(m_pCloseBtn);
    pTitleLayout->setContentsMargins(0, 0, 0, 0); // 标题栏容器无内边距

    // 5. 信号槽连接
    connect(m_pMinBtn, &QPushButton::clicked, this, &CFrameLessWidgetBase::onTitleBarButtonClicked);    // 最小化
    connect(m_pMaxBtn, &QPushButton::clicked, this, &CFrameLessWidgetBase::onTitleBarButtonClicked);    // 最大化
    connect(m_pCloseBtn, &QPushButton::clicked, this, &CFrameLessWidgetBase::onTitleBarButtonClicked);  // 关闭
}

// 提供给子类调用的接口，修改标题栏文本
void CFrameLessWidgetBase::setWindowTitleText(const QString& text)
{
    if (m_pTitleTextLabel) {
        m_pTitleTextLabel->setText(text);
    }
}

// 提供给子类调用的接口，自定义 Widget 类
void CFrameLessWidgetBase::setCentralWidget(QWidget* widget)
{
    m_pCentralWidget = widget;
    if (m_pCentralWidget) {
        // 整体垂直布局
        QVBoxLayout* pMainLayout = new QVBoxLayout(this);
        pMainLayout->setContentsMargins(0, 0, 0, 0);  // 去除整体窗口布局的外边距
        pMainLayout->setSpacing(0);                   // 取消标题栏部件和中心业务部件之间的上下间隙
        pMainLayout->addWidget(m_pTitleBarWidget);
        pMainLayout->addWidget(m_pCentralWidget);
        this->setLayout(pMainLayout);                 // 垂直布局应用到当前窗口对象上
    }
}

void CFrameLessWidgetBase::onTitleBarButtonClicked()
{
    QPushButton* pButton = qobject_cast<QPushButton*>(sender()); // 获取被点击对象

	if (pButton == m_pMinBtn) {         // 最小化按钮被点击
        this->showMinimized();
    }
	else if (pButton == m_pMaxBtn) {    // 最大化/还原按钮被点击，并触发窗口状态改变事件 changeEvent
		if (this->isMaximized()) {      
            this->showNormal();         // 当前是最大化则还原
        }
        else {
            this->showMaximized();      // 当前是还原则最大化，
        }
    }
    else {
        this->close();                  // 调用 closeEvent 
    }
}

void CFrameLessWidgetBase::updateMaximizeButtonState(bool isMaximized)
{
	if (isMaximized) { // 当前窗口处于最大化状态，按钮显示为还原图标
        m_pMaxBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/max.svg);border:none}"
            "QPushButton:hover{background-color:rgb(182, 209, 255);;border:none;}");
    }
    else {
        m_pMaxBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/normal.svg);border:none}"
            "QPushButton:hover{background-color:rgb(182, 209, 255);border:none;}");
    }
}

// 最大化/还原按钮状态变化，自动调用
void CFrameLessWidgetBase::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {                                        // 窗口状态变化
        QWindowStateChangeEvent* pStateEvent = static_cast<QWindowStateChangeEvent*>(event); // 从事件中拿状态
        updateMaximizeButtonState(isMaximized());                                            // 更新图标状态
    }
    QWidget::changeEvent(event);                                                             // 调用父类默认处理
}

// 点击窗口关闭，自动调用
void CFrameLessWidgetBase::closeEvent(QCloseEvent* event)
{
    QMessageBox::StandardButton _exit = QMessageBox::warning( 
        this, 
        "tip",                                // 提示
        "make sure to exit?",                 // 确认要关闭吗？
        QMessageBox::Yes | QMessageBox::No, 
        QMessageBox::No
    );   
    if (_exit == QMessageBox::Yes) {
        event->accept(); // 同意关闭
    }
    else {
        event->ignore(); // 忽略关闭
	}
}

// 鼠标在窗口上移动，自动调用                            平台标识符，系统消息参数，结果返回值
bool CFrameLessWidgetBase::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    MSG* param = static_cast<MSG*>(message);                               // 将系统消息参数转换为Windows API的MSG结构体指针

    // 拦截 WM_NCCALCSIZE 消息，消除因为加上 WS_THICKFRAME 而出现的原生白边
    if (param->message == WM_NCCALCSIZE) {                                 // 判断系统是否在重新计算窗口非客户区大小
        if (param->wParam == TRUE) {                                       // 如果是 TRUE 意味着系统要求计算客户区
            *result = 0;                                                   // 直接返回 0，告诉 Windows：我的整个窗口全都是有效客户区，别给我画系统边框
            return true;                                                   // 拦截消息，不再往下传
        } 
    } 

    if (param->message == WM_NCHITTEST)                                      // 处理拖拽和边缘缩放
    {
        QPoint globalPos = QCursor::pos();                                   // 获取鼠标绝对全局坐标
        QPoint localPos = this->mapFromGlobal(globalPos);                    // 转换为相对于当前窗口内部的逻辑坐标
        int nX = localPos.x();                                               // 获取X轴逻辑坐标
        int nY = localPos.y();                                               // 获取Y轴逻辑坐标

        const int nBorder = 8;                                               // 边缘感应区长度

        // 1. 拉伸与缩放功能，不能比窗口设定值还小，边缘检测防止越界
        bool bLeft = (nX >= 0 && nX <= nBorder);                             // 判断左侧感应区
        bool bRight = (nX >= this->width() - nBorder && nX <= this->width());// 判断右侧感应区
        bool bTop = (nY >= 0 && nY <= nBorder);                              // 判断顶部感应区
        bool bBottom = (nY >= this->height() - nBorder && nY <= this->height()); // 判断底部感应区

        // 必须优先判断四个角落，再判断四条边
        if (bLeft && bTop) { *result = HTTOPLEFT; return true; }             // 命中左上角
        if (bRight && bTop) { *result = HTTOPRIGHT; return true; }           // 命中右上角
        if (bLeft && bBottom) { *result = HTBOTTOMLEFT; return true; }       // 命中左下角
        if (bRight && bBottom) { *result = HTBOTTOMRIGHT; return true; }     // 命中右下角
        if (bLeft) { *result = HTLEFT; return true; }                        // 命中左边缘
        if (bRight) { *result = HTRIGHT; return true; }                      // 命中右边缘
        if (bBottom) { *result = HTBOTTOM; return true; }                    // 命中下边缘
        if (bTop) { *result = HTTOP; return true; }                          // 命中上边缘

        // 2. 拖拽功能
        if (m_pTitleBarWidget && m_pTitleBarWidget->geometry().contains(localPos)) { // 判断鼠标是否落在标题栏范围内
            QPoint titleBarPos = m_pTitleBarWidget->mapFrom(this, localPos);         // 获取鼠标在标题栏容器内部的相对坐标
            QWidget* child = m_pTitleBarWidget->childAt(titleBarPos);                // 探测鼠标指针正下方是否有子控件
            if (!child || !qobject_cast<QPushButton*>(child)) {                      // 如果下方没有控件，或者该控件不是按钮
                *result = HTCAPTION;                                                 // 赋值表示拖拽功能
                return true;                                                         // 返回 true 表示成功拦截消息
            } 
        } 
    } 

	// 3. 调用基类处理，实际的拖拽和缩放逻辑代码复用
    return QWidget::nativeEvent(eventType, message, result);                         
}