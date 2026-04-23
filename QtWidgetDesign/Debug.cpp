#include "Debug.h"

/*
* Debug.cpp - 调试信息输出类实现
* 
* 功能：
* 1. 实现调试信息的打印功能
* 2. 提供清理调试信息的方法
* 3. 支持宽字符和QString格式的调试信息
*/

//调试信息输出
Debug* Debug::_pDebug = nullptr;

/**
* @brief Debug类构造函数
*/
Debug::Debug()
    : m_edit(nullptr)
{
}

/**
* @brief 获取Debug类的单例实例
* @return Debug* - Debug实例指针
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
* @brief 初始化Debug类，设置输出文本编辑框
* @param edit - 用于显示调试信息的QTextEdit指针
*/
void Debug::init(QTextEdit *edit)
{
    m_edit = edit;
}

/**
* @brief 打印宽字符格式的调试信息
* @param str - 宽字符格式的调试信息
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
* @brief 打印QString格式的调试信息
* @param str - QString格式的调试信息
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
* @brief 清理调试信息
*/
void Debug::clean()
{
    if (m_edit != nullptr)
    {
        m_edit->clear();
    }
}