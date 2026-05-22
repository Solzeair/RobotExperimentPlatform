#pragma once

/*
* Debug.h - 调试信息输出类头文件
* 
* 功能：
* 1. 定义调试信息输出的接口
* 2. 提供单例模式访问
* 3. 支持宽字符和QString格式的调试信息
* 4. 提供清理调试信息的方法
* 
* 使用方法：
* Debug::get()->print(L"调试信息");
* Debug::get()->clean();
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