#pragma once

/*
* Debug.h - 调试信息输出类
*
* 采用单例模式，将调试信息统一输出到指定的 QTextEdit 控件，
* 便于在界面上集中查看运行日志。同时支持宽字符与 QString 两种输入，
* 兼容底层 C++ 接口与 Qt 接口的调用场景。
*/

#include <QDebug>
#include <QTextEdit>
#include <QString>

class Debug
{
private:
    Debug();
public:
    static Debug* get();
    void init(QTextEdit *edit);
    void print(const wchar_t* str);
    void print(const QString& str);
    void clean();
private:
    static Debug *_pDebug;
    QTextEdit *m_edit;
};