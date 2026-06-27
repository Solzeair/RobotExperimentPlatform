#include "Debug.h"

/*
* Debug.cpp - 调试信息输出类实现
* 单例模式，将日志统一输出到绑定的 QTextEdit，避免散落各处的 printf
*/

// 调试信息输出类：通过单例统一管理日志，避免到处直接访问UI控件
Debug* Debug::_pDebug = nullptr;

/**
* @brief 构造函数：输出目标初始化为空，必须先调用 init 绑定文本框后才能输出
*/
Debug::Debug()
    : m_edit(nullptr)
{
}

/**
* @brief 懒汉单例：首次访问时创建实例，全局唯一
*/
Debug* Debug::get()
{
    if (_pDebug == nullptr)
    {
        _pDebug = new Debug();
    }
    return _pDebug;
}

/**
* @brief 绑定日志输出的文本框，需在调用 print/clean 之前完成
*/
void Debug::init(QTextEdit *edit)
{
    m_edit = edit;
}

/**
* @brief 打印宽字符调试信息，逐条追加换行保持可读性
*/
void Debug::print(const wchar_t* str)
{
    if (m_edit != nullptr)
    {
        m_edit->append(QString::fromWCharArray(str));
        m_edit->append("\n");
    }
}

/**
* @brief 打印QString调试信息，与宽字符版本行为一致
*/
void Debug::print(const QString& str)
{
    if (m_edit != nullptr)
    {
        m_edit->append(str);
        m_edit->append("\n");
    }
}

/**
* @brief 清空日志显示，便于重新开始观察输出
*/
void Debug::clean()
{
    if (m_edit != nullptr)
    {
        m_edit->clear();
    }
}