#ifndef PARAMETERDIALOG_H
#define PARAMETERDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTimer>
#include <string>
#include <vector>

class ParameterDialog : public QDialog {
    Q_OBJECT

public:
    explicit ParameterDialog(QWidget* parent = nullptr);
    ~ParameterDialog();

    // 设置DLL函数指针
    void setSetParameterFunc(void* strategy, void (*func)(void*, const char*, double));
    void setGetParameterFunc(void* strategy, double (*func)(void*, const char*));
    void setSaveConfigFunc(void* strategy, bool (*func)(void*, const char*));
    void setLoadConfigFunc(void* strategy, bool (*func)(void*, const char*));
    void setStrategy(void* strategy) { m_strategy = strategy; }

private slots:
    void onSaveClicked();
    void onLoadClicked();
    void onResetClicked();
    void onApplyClicked();
    void onRefreshClicked();
    void onSpinBoxChanged(double value);
    void updateDisplay();

private:
    struct Parameter {
        std::string name;
        double value;
        double minVal;
        double maxVal;
        double step;
        std::string description;

        Parameter(const std::string& n, double v, double minV, double maxV, double s, const std::string& desc)
            : name(n), value(v), minVal(minV), maxVal(maxV), step(s), description(desc) {}
    };

    void setupUI();
    void loadParameters();
    void updateTable();

    // UI组件
    QTableWidget* m_table;
    QLineEdit* m_filterEdit;
    QPushButton* m_saveBtn;
    QPushButton* m_loadBtn;
    QPushButton* m_resetBtn;
    QPushButton* m_applyBtn;
    QPushButton* m_refreshBtn;
    QPushButton* m_closeBtn;
    QTimer* m_updateTimer;

    // 数据
    std::vector<Parameter> m_parameters;

    // DLL相关
    void* m_strategy;
    void (*m_setParameter)(void*, const char*, double);
    double (*m_getParameter)(void*, const char*);
    bool (*m_saveConfig)(void*, const char*);
    bool (*m_loadConfig)(void*, const char*);
};

#endif
