#pragma once

//Debug 类	打印调试信息，将信息打印到调试窗口
// Usage:	Debug::get()->print(L"");
// Usage:	Debug::get()->clean();

#include <QDebug>
#include <QTextEdit>

class Debug
{
private:
    Debug();
public:
    static Debug* get();
    void init(QTextEdit *edit);
    void print(const wchar_t* str);
    void clean();
private:
    static Debug *_pDebug;
    QTextEdit *m_edit;
};