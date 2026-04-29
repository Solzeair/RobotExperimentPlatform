// parameterdialog.cpp
// 参数调优对话框实现

#include "ParameterDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDoubleSpinBox>
#include <QLabel>
#include <iostream>

/**
 * 构造函数
 * 初始化UI并加载参数列表
 */
ParameterDialog::ParameterDialog(QWidget* parent)
    : QDialog(parent)
    , m_table(nullptr)
    , m_filterEdit(nullptr)
    , m_saveBtn(nullptr)
    , m_loadBtn(nullptr)
    , m_resetBtn(nullptr)
    , m_applyBtn(nullptr)
    , m_refreshBtn(nullptr)
    , m_closeBtn(nullptr)
    , m_updateTimer(nullptr)      // 暂时禁用定时器，避免频繁更新导致崩溃
    , m_strategy(nullptr)
    , m_setParameter(nullptr)
    , m_getParameter(nullptr)
    , m_saveConfig(nullptr)
    , m_loadConfig(nullptr) {

    std::cout << "ParameterDialog constructor" << std::endl;

    // 创建UI界面
    setupUI();

    // 加载参数定义
    loadParameters();

    std::cout << "ParameterDialog initialized, parameters count: " << m_parameters.size() << std::endl;

    // 注意：暂时禁用自动更新定时器，避免崩溃
    // 用户需要手动点击"刷新"按钮来更新参数
}

/**
 * 析构函数
 * 清理定时器资源
 */
ParameterDialog::~ParameterDialog() {
    std::cout << "ParameterDialog destructor" << std::endl;
    if (m_updateTimer) {
        m_updateTimer->stop();      // 停止定时器
        delete m_updateTimer;       // 删除定时器对象
        m_updateTimer = nullptr;
    }
}

/**
 * 初始化UI界面
 * 创建参数表格、搜索框、按钮等控件
 */
void ParameterDialog::setupUI() {
    // 设置窗口标题和最小尺寸
    setWindowTitle("策略参数调优");
    setMinimumSize(700, 600);

    // 主布局：垂直布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ==================== 搜索过滤栏 ====================
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("搜索:"));
    m_filterEdit = new QLineEdit();
    m_filterEdit->setPlaceholderText("输入参数名过滤...");
    // 连接搜索框文本变化信号到更新表格槽函数
    connect(m_filterEdit, &QLineEdit::textChanged, this, &ParameterDialog::updateTable);
    filterLayout->addWidget(m_filterEdit);
    mainLayout->addLayout(filterLayout);

    // ==================== 参数表格 ====================
    m_table = new QTableWidget();
    m_table->setColumnCount(6);  // 6列：参数名、当前值、最小值、最大值、步长、描述
    m_table->setHorizontalHeaderLabels({"参数名", "当前值", "最小值", "最大值", "步长", "描述"});
    m_table->horizontalHeader()->setStretchLastSection(true);  // 最后一列自动拉伸
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);  // 整行选中
    mainLayout->addWidget(m_table);

    // ==================== 按钮栏 ====================
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_saveBtn = new QPushButton("保存到文件");
    m_loadBtn = new QPushButton("从文件加载");
    m_resetBtn = new QPushButton("恢复默认");
    m_applyBtn = new QPushButton("应用到策略");
    m_refreshBtn = new QPushButton("刷新");
    m_closeBtn = new QPushButton("关闭");

    // 设置按钮样式（彩色按钮便于区分功能）
    m_saveBtn->setStyleSheet("background-color: #4CAF50; color: white;");   // 绿色 - 保存
    m_loadBtn->setStyleSheet("background-color: #2196F3; color: white;");   // 蓝色 - 加载
    m_resetBtn->setStyleSheet("background-color: #FF9800; color: white;");  // 橙色 - 重置
    m_applyBtn->setStyleSheet("background-color: #9C27B0; color: white;");  // 紫色 - 应用

    // 连接按钮点击信号
    connect(m_saveBtn, &QPushButton::clicked, this, &ParameterDialog::onSaveClicked);
    connect(m_loadBtn, &QPushButton::clicked, this, &ParameterDialog::onLoadClicked);
    connect(m_resetBtn, &QPushButton::clicked, this, &ParameterDialog::onResetClicked);
    connect(m_applyBtn, &QPushButton::clicked, this, &ParameterDialog::onApplyClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &ParameterDialog::onRefreshClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    // 添加按钮到布局
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_loadBtn);
    btnLayout->addWidget(m_resetBtn);
    btnLayout->addWidget(m_applyBtn);
    btnLayout->addWidget(m_refreshBtn);
    btnLayout->addWidget(m_closeBtn);
    mainLayout->addLayout(btnLayout);

    // ==================== 状态栏提示 ====================
    QLabel* statusLabel = new QLabel("提示: 使用SpinBox编辑参数值，修改后点击'应用到策略'生效");
    statusLabel->setStyleSheet("color: gray; font-size: 10px;");
    mainLayout->addWidget(statusLabel);

    std::cout << "setupUI completed" << std::endl;
}

/**
 * 加载参数定义
 * 初始化所有可调参数的默认值、范围和描述
 */
void ParameterDialog::loadParameters() {
    m_parameters.clear();

    // 运动控制参数组
    m_parameters.push_back(Parameter("max_speed", 75.0, 50.0, 100.0, 5.0, "最大移动速度 (cm/s)"));
    m_parameters.push_back(Parameter("min_speed", 15.0, 5.0, 40.0, 2.0, "最小移动速度 (cm/s)"));
    m_parameters.push_back(Parameter("kp_pos", 12.0, 5.0, 25.0, 1.0, "位置比例系数"));
    m_parameters.push_back(Parameter("kp_angle", 22.0, 10.0, 40.0, 1.0, "角度比例系数"));
    m_parameters.push_back(Parameter("kd_pos", 0.5, 0.1, 2.0, 0.1, "位置微分系数"));
    m_parameters.push_back(Parameter("kd_angle", 7.0, 2.0, 15.0, 0.5, "角度微分系数"));

    // 战术参数组
    m_parameters.push_back(Parameter("attack_aggression", 0.6, 0.0, 1.0, 0.05, "进攻侵略性 (0-1)"));
    m_parameters.push_back(Parameter("defense_depth", 0.5, 0.0, 1.0, 0.05, "防守深度 (0-1)"));
    m_parameters.push_back(Parameter("pressing_intensity", 0.5, 0.0, 1.0, 0.05, "压迫强度 (0-1)"));

    // 射门参数组
    m_parameters.push_back(Parameter("shoot_power", 80.0, 60.0, 100.0, 5.0, "射门力量"));
    m_parameters.push_back(Parameter("shoot_angle_tolerance", 0.2, 0.05, 0.5, 0.02, "射门角度容差 (rad)"));

    // 传球参数组
    m_parameters.push_back(Parameter("pass_success_threshold", 0.6, 0.3, 0.9, 0.05, "传球成功率阈值"));

    // 门将参数组
    m_parameters.push_back(Parameter("goalie_aggression", 0.5, 0.0, 1.0, 0.05, "门将出击侵略性"));
    m_parameters.push_back(Parameter("goalie_speed", 60.0, 40.0, 80.0, 5.0, "门将移动速度"));

    // 避障参数组
    m_parameters.push_back(Parameter("avoid_distance", 25.0, 15.0, 40.0, 1.0, "避障距离 (cm)"));
    m_parameters.push_back(Parameter("avoid_weight", 1.5, 0.5, 3.0, 0.1, "避障权重"));

    // 刷新表格显示
    updateTable();
}

/**
 * 更新表格显示
 * 根据搜索过滤条件刷新参数表格
 */
void ParameterDialog::updateTable() {
    if (!m_table || !m_filterEdit) return;

    QString filter = m_filterEdit->text().toLower();  // 获取过滤文本（转小写）

    // 计算符合条件的参数数量
    int visibleCount = 0;
    for (const auto& param : m_parameters) {
        if (filter.isEmpty() || QString::fromStdString(param.name).toLower().contains(filter)) {
            visibleCount++;
        }
    }

    // 设置表格行数
    m_table->setRowCount(visibleCount);

    int row = 0;
    for (const auto& param : m_parameters) {
        // 跳过不符合过滤条件的参数
        if (!filter.isEmpty() && !QString::fromStdString(param.name).toLower().contains(filter)) {
            continue;
        }

        // 列0: 参数名（只读）
        QTableWidgetItem* nameItem = new QTableWidgetItem(QString::fromStdString(param.name));
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);  // 设置为不可编辑
        m_table->setItem(row, 0, nameItem);

        // 列1: 当前值（使用SpinBox编辑）
        QDoubleSpinBox* spinBox = new QDoubleSpinBox();
        spinBox->setRange(param.minVal, param.maxVal);      // 设置取值范围
        spinBox->setSingleStep(param.step);                  // 设置步长
        spinBox->setDecimals(3);                             // 设置小数位数
        spinBox->setValue(param.value);                      // 设置当前值
        spinBox->setToolTip(QString::fromStdString(param.description));  // 设置提示
        // 连接值变化信号
        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &ParameterDialog::onSpinBoxChanged);
        m_table->setCellWidget(row, 1, spinBox);

        // 列2: 最小值（只读）
        QTableWidgetItem* minItem = new QTableWidgetItem(QString::number(param.minVal));
        minItem->setFlags(minItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 2, minItem);

        // 列3: 最大值（只读）
        QTableWidgetItem* maxItem = new QTableWidgetItem(QString::number(param.maxVal));
        maxItem->setFlags(maxItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 3, maxItem);

        // 列4: 步长（只读）
        QTableWidgetItem* stepItem = new QTableWidgetItem(QString::number(param.step));
        stepItem->setFlags(stepItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 4, stepItem);

        // 列5: 描述（只读）
        QTableWidgetItem* descItem = new QTableWidgetItem(QString::fromStdString(param.description));
        descItem->setFlags(descItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 5, descItem);

        row++;
    }

    // 调整列宽
    m_table->resizeColumnsToContents();
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
}

/**
 * SpinBox值变化处理
 * 更新内部参数存储
 * @param value 新的参数值
 */
void ParameterDialog::onSpinBoxChanged(double value) {
    QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox*>(sender());
    if (!spinBox) return;

    // 查找对应的参数并更新值
    for (int row = 0; row < m_table->rowCount(); row++) {
        if (m_table->cellWidget(row, 1) == spinBox) {
            QString name = m_table->item(row, 0)->text();
            for (auto& param : m_parameters) {
                if (param.name == name.toStdString()) {
                    param.value = value;  // 更新参数值
                    break;
                }
            }
            break;
        }
    }
}

/**
 * 更新显示（预留）
 * 当前禁用，避免自动更新导致崩溃
 */
void ParameterDialog::updateDisplay() {
    // 暂时禁用自动更新，避免崩溃
    // 用户需要手动点击"刷新"按钮
    return;
}

/**
 * 保存按钮点击处理
 * 将当前参数保存到文件
 */
void ParameterDialog::onSaveClicked() {
    // 弹出文件保存对话框
    QString filename = QFileDialog::getSaveFileName(this, "保存参数配置", "",
                                                    "参数文件 (*.txt);;所有文件 (*)");
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);
            // 写入文件头注释
            stream << "# Robot Strategy Parameters\n";
            stream << "# Format: parameter_name = value\n\n";

            // 写入所有参数
            for (const auto& param : m_parameters) {
                stream << QString::fromStdString(param.name) << " = " << param.value
                       << "  # " << QString::fromStdString(param.description) << "\n";
            }

            file.close();
            QMessageBox::information(this, "成功", "参数已保存到 " + filename);
        } else {
            QMessageBox::warning(this, "错误", "无法保存文件");
        }
    }
}

/**
 * 加载按钮点击处理
 * 从文件加载参数
 */
void ParameterDialog::onLoadClicked() {
    // 弹出文件打开对话框
    QString filename = QFileDialog::getOpenFileName(this, "加载参数配置", "",
                                                    "参数文件 (*.txt);;所有文件 (*)");
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            QString line;
            // 逐行读取文件
            while (stream.readLineInto(&line)) {
                line = line.trimmed();
                // 跳过空行和注释行
                if (line.isEmpty() || line.startsWith("#")) continue;

                // 解析"参数名 = 值"格式
                int eqPos = line.indexOf('=');
                if (eqPos > 0) {
                    QString name = line.left(eqPos).trimmed();
                    QString valueStr = line.mid(eqPos + 1).trimmed();

                    // 去除行尾注释
                    int commentPos = valueStr.indexOf('#');
                    if (commentPos > 0) valueStr = valueStr.left(commentPos).trimmed();

                    // 转换为数值
                    bool ok;
                    double value = valueStr.toDouble(&ok);
                    if (ok) {
                        // 更新参数值
                        for (auto& param : m_parameters) {
                            if (param.name == name.toStdString()) {
                                param.value = value;
                                break;
                            }
                        }
                    }
                }
            }
            file.close();

            // 刷新表格显示
            updateTable();
            QMessageBox::information(this, "成功", "参数已从 " + filename + " 加载");
        } else {
            QMessageBox::warning(this, "错误", "无法加载文件");
        }
    }
}

/**
 * 重置按钮点击处理
 * 将所有参数恢复为默认值（取中值）
 */
void ParameterDialog::onResetClicked() {
    // 确认对话框
    if (QMessageBox::question(this, "确认", "确定要恢复所有参数到默认值吗？") == QMessageBox::Yes) {
        // 将每个参数重置为默认值（最小值与最大值的平均值）
        for (auto& param : m_parameters) {
            param.value = (param.minVal + param.maxVal) / 2;
        }
        // 刷新表格显示
        updateTable();
        QMessageBox::information(this, "成功", "参数已恢复默认值");
    }
}

/**
 * 应用按钮点击处理
 * 将当前参数应用到策略DLL
 */
void ParameterDialog::onApplyClicked() {
    // 检查DLL函数是否已设置
    if (!m_setParameter || !m_strategy) {
        QMessageBox::warning(this, "错误", "无法应用参数：DLL函数未设置");
        return;
    }

    // 遍历所有参数，调用DLL的setParameter函数
    for (const auto& param : m_parameters) {
        m_setParameter(m_strategy, param.name.c_str(), param.value);
    }
    QMessageBox::information(this, "成功", "参数已应用到策略");
}

/**
 * 刷新按钮点击处理
 * 重新加载参数列表
 */
void ParameterDialog::onRefreshClicked() {
    loadParameters();  // 重新加载参数定义
    QMessageBox::information(this, "成功", "参数列表已刷新");
}

/**
 * 设置参数设置函数指针
 */
void ParameterDialog::setSetParameterFunc(void* strategy, void (*func)(void*, const char*, double)) {
    m_strategy = strategy;
    m_setParameter = func;
}

/**
 * 设置参数获取函数指针
 */
void ParameterDialog::setGetParameterFunc(void* strategy, double (*func)(void*, const char*)) {
    m_strategy = strategy;
    m_getParameter = func;
}

/**
 * 设置配置保存函数指针
 */
void ParameterDialog::setSaveConfigFunc(void* strategy, bool (*func)(void*, const char*)) {
    m_strategy = strategy;
    m_saveConfig = func;
}

/**
 * 设置配置加载函数指针
 */
void ParameterDialog::setLoadConfigFunc(void* strategy, bool (*func)(void*, const char*)) {
    m_strategy = strategy;
    m_loadConfig = func;
}
