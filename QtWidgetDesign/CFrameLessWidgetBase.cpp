#include "CFrameLessWidgetBase.h"
#include <qt_windows.h>
#include <windows.h>

CFrameLessWidgetBase::CFrameLessWidgetBase(QWidget* parent)
    : QWidget(parent)
{
    // 去除系统边框，保留系统最小化、最大化的原生动画机制
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_Hover);     // 启用悬停事件支持，配合QSS实现控件悬停态样式

    // 关键技巧：先去Qt边框再通过Win32 API加回原生WS_THICKFRAME/WS_CAPTION，
    // 这样既能自绘标题栏，又能复用系统的边缘缩放与最大化动画，无需自己重写缩放逻辑
    HWND hwnd = (HWND)this->winId();                                                       // 获取窗口原生Win32句柄
    DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);                                        // 读取当前窗口原生样式
    ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION); // 附加最大化、拉伸边框、标题栏能力

    initBaseUI();                   // 初始化自定义标题栏与布局
}

CFrameLessWidgetBase::~CFrameLessWidgetBase()
{
}

/*
* 自定义无边框窗口UI初始化
* 功能：构建自定义标题栏、窗口控制按钮，并预留拖拽与边缘缩放能力
*/
void CFrameLessWidgetBase::initBaseUI()
{
    // 1. 标题栏
    m_pTitleBarWidget = new QWidget(this);
    m_pTitleBarWidget->setAttribute(Qt::WA_StyledBackground); // 启用样式背景，否则QWidget的QSS背景色不生效
	m_pTitleBarWidget->setFixedHeight(32);                    // 标题栏固定高度
    m_pTitleBarWidget->setStyleSheet("background-color:rgb(130, 177, 255)");

    // 左上角logo
    m_pLogo = new QLabel(m_pTitleBarWidget);
    m_pLogo->setFixedSize(32, 32);
    m_pLogo->setStyleSheet("background: transparent; border: none;");
    // SVG无法直接setPixmap，需借助QIcon渲染到pixmap才能正确显示矢量图标
    QIcon logoIcon(":/QtWidgetDesign/resources/xsyu_logo.svg");
    m_pLogo->setPixmap(logoIcon.pixmap(QSize(32, 32)));
    m_pLogo->setScaledContents(true); // SVG图标按容器等比缩放

	// 标题文本
    m_pTitleTextLabel = new QLabel(m_pTitleBarWidget);
	m_pTitleTextLabel->setText("default title"); // 占位默认标题，子类通过setWindowTitleText覆盖
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

	// 右侧按钮：最大化/还原，图标随窗口状态动态切换（见updateMaximizeButtonState）
    m_pMaxBtn = new QPushButton(m_pTitleBarWidget);
    m_pMaxBtn->setFixedSize(32, 32);
    m_pMaxBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/normal.svg);border:none}"
        "QPushButton:hover{background-color:rgb(182, 209, 255);border:none;}");

	// 右侧按钮：关闭
    m_pCloseBtn = new QPushButton(m_pTitleBarWidget);
    m_pCloseBtn->setFixedSize(32, 32);
    m_pCloseBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/close.svg);border:none}"
        "QPushButton:hover{background-color:rgb(182, 209, 255);border:none;}");

    // 2. 标题栏横向布局：logo与标题左对齐，控制按钮右对齐，中间用弹簧撑开
    QHBoxLayout* pTitleLayout = new QHBoxLayout(m_pTitleBarWidget);
    pTitleLayout->addWidget(m_pLogo);
    pTitleLayout->addWidget(m_pTitleTextLabel);
    pTitleLayout->addStretch();                   // 弹簧把右侧按钮挤到最右
    pTitleLayout->addWidget(m_pSetBtn);
    pTitleLayout->addSpacing(10);                 // 按钮间留白
    pTitleLayout->addWidget(m_pMinBtn);
    pTitleLayout->addSpacing(10);
    pTitleLayout->addWidget(m_pMaxBtn);
    pTitleLayout->addSpacing(10);
    pTitleLayout->addWidget(m_pCloseBtn);
    pTitleLayout->setContentsMargins(0, 0, 0, 0); // 标题栏贴合窗口边缘无内边距

    // 5. 标题栏按钮统一路由到onTitleBarButtonClicked，再按sender区分行为
    connect(m_pMinBtn, &QPushButton::clicked, this, &CFrameLessWidgetBase::onTitleBarButtonClicked);
    connect(m_pMaxBtn, &QPushButton::clicked, this, &CFrameLessWidgetBase::onTitleBarButtonClicked);
    connect(m_pCloseBtn, &QPushButton::clicked, this, &CFrameLessWidgetBase::onTitleBarButtonClicked);
}

// 对外接口：子类设置标题栏文本
void CFrameLessWidgetBase::setWindowTitleText(const QString& text)
{
    if (m_pTitleTextLabel) {
        m_pTitleTextLabel->setText(text);
    }
}

// 对外接口：子类注入业务中心Widget，本基类负责将其与标题栏组合成整体垂直布局
void CFrameLessWidgetBase::setCentralWidget(QWidget* widget)
{
    m_pCentralWidget = widget;
    if (m_pCentralWidget) {
        // 标题栏在顶、业务区在下的垂直布局
        QVBoxLayout* pMainLayout = new QVBoxLayout(this);
        pMainLayout->setContentsMargins(0, 0, 0, 0);  // 贴边布局，无外边距
        pMainLayout->setSpacing(0);                   // 标题栏与业务区无缝衔接
        pMainLayout->addWidget(m_pTitleBarWidget);
        pMainLayout->addWidget(m_pCentralWidget);
        this->setLayout(pMainLayout);
    }
}

void CFrameLessWidgetBase::onTitleBarButtonClicked()
{
    QPushButton* pButton = qobject_cast<QPushButton*>(sender()); // 通过sender识别被点击的按钮

	if (pButton == m_pMinBtn) {         // 最小化
        this->showMinimized();
    }
	else if (pButton == m_pMaxBtn) {    // 最大化/还原切换，状态变化会触发changeEvent刷新图标
		if (this->isMaximized()) {      
            this->showNormal();
        }
        else {
            this->showMaximized();
        }
    }
    else {
        this->close();                  // 关闭按钮，走closeEvent二次确认
    }
}

void CFrameLessWidgetBase::updateMaximizeButtonState(bool isMaximized)
{
	if (isMaximized) { // 最大化时按钮显示"还原"图标
        m_pMaxBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/max.svg);border:none}"
            "QPushButton:hover{background-color:rgb(182, 209, 255);;border:none;}");
    }
    else {
        m_pMaxBtn->setStyleSheet("QPushButton{background-image:url(:/QtWidgetDesign/resources/normal.svg);border:none}"
            "QPushButton:hover{background-color:rgb(182, 209, 255);border:none;}");
    }
}

// 窗口状态变化时自动回调，用于同步最大化/还原按钮图标
void CFrameLessWidgetBase::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {                                        // 仅处理窗口状态变化事件
        QWindowStateChangeEvent* pStateEvent = static_cast<QWindowStateChangeEvent*>(event); // 转换为状态变化事件
        updateMaximizeButtonState(isMaximized());                                            // 根据当前状态刷新按钮图标
    }
    QWidget::changeEvent(event);                                                             // 其余事件交由父类默认处理
}

// 窗口关闭前自动回调，弹窗二次确认避免误关
void CFrameLessWidgetBase::closeEvent(QCloseEvent* event)
{
    QMessageBox::StandardButton _exit = QMessageBox::warning( 
        this, 
        "tip",                                // 标题
        "make sure to exit?",                 // 提示语
        QMessageBox::Yes | QMessageBox::No, 
        QMessageBox::No
    );   
    if (_exit == QMessageBox::Yes) {
        event->accept(); // 确认关闭
    }
    else {
        event->ignore(); // 取消关闭
	}
}

// 原生事件回调：通过拦截Win32消息实现无边框窗口的自绘标题栏拖拽与边缘缩放
bool CFrameLessWidgetBase::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    MSG* param = static_cast<MSG*>(message);                               // 将系统消息参数转换为MSG结构体指针

    // 拦截 WM_NCCALCSIZE，消除 WS_THICKFRAME 带来的原生白边
    if (param->message == WM_NCCALCSIZE) {                                 // 系统重新计算非客户区时拦截
        if (param->wParam == TRUE) {                                       // TRUE表示系统请求计算客户区
            *result = 0;                                                   // 返回0：整个窗口均视作客户区，不绘制系统边框
            return true;                                                   // 拦截该消息，不再下发
        } 
    } 

    if (param->message == WM_NCHITTEST)                                      // 命中测试：判定鼠标应触发拖拽还是边缘缩放
    {
        QPoint globalPos = QCursor::pos();                                   // 鼠标全局坐标
        QPoint localPos = this->mapFromGlobal(globalPos);                    // 转换为窗口内逻辑坐标
        int nX = localPos.x();                                               // X轴逻辑坐标
        int nY = localPos.y();                                               // Y轴逻辑坐标

        const int nBorder = 8;                                               // 边缘缩放感应区宽度（像素）

        // 1. 边缘缩放：按鼠标所在边缘/角落返回对应HT*命中码
        bool bLeft = (nX >= 0 && nX <= nBorder);                             // 命中左侧感应区
        bool bRight = (nX >= this->width() - nBorder && nX <= this->width());// 命中右侧感应区
        bool bTop = (nY >= 0 && nY <= nBorder);                              // 命中顶部感应区
        bool bBottom = (nY >= this->height() - nBorder && nY <= this->height()); // 命中底部感应区

        // 四角优先于四边，否则角落会被边的判断抢先命中
        if (bLeft && bTop) { *result = HTTOPLEFT; return true; }             // 左上角
        if (bRight && bTop) { *result = HTTOPRIGHT; return true; }           // 右上角
        if (bLeft && bBottom) { *result = HTBOTTOMLEFT; return true; }       // 左下角
        if (bRight && bBottom) { *result = HTBOTTOMRIGHT; return true; }     // 右下角
        if (bLeft) { *result = HTLEFT; return true; }                        // 左边缘
        if (bRight) { *result = HTRIGHT; return true; }                      // 右边缘
        if (bBottom) { *result = HTBOTTOM; return true; }                    // 下边缘
        if (bTop) { *result = HTTOP; return true; }                          // 上边缘

        // 2. 拖拽：鼠标落在标题栏非按钮区域时返回HTCAPTION交由系统拖动窗口
        if (m_pTitleBarWidget && m_pTitleBarWidget->geometry().contains(localPos)) { // 仅标题栏区域内可拖拽
            QPoint titleBarPos = m_pTitleBarWidget->mapFrom(this, localPos);         // 转为标题栏容器内坐标
            QWidget* child = m_pTitleBarWidget->childAt(titleBarPos);                // 探测鼠标下是否有子控件
            if (!child || !qobject_cast<QPushButton*>(child)) {                      // 落空或非按钮才允许拖拽，避免误吞按钮点击
                *result = HTCAPTION;                                                 // 返回拖拽命中码
                return true;                                                         // 成功拦截该消息
            } 
        } 
    } 

	// 3. 其余消息交由基类处理，复用Qt默认行为
    return QWidget::nativeEvent(eventType, message, result);                         
}