#include "Debug.h"

Debug* Debug::_pDebug = nullptr;

Debug::Debug()
    : m_edit(nullptr)
{
}

Debug* Debug::get()
{
    if (_pDebug == nullptr)
    {
        _pDebug = new Debug();
    }
    return _pDebug;
}

void Debug::init(QTextEdit *edit)
{
    m_edit = edit;
}

void Debug::print(const wchar_t* str)
{
    if (m_edit != nullptr)
    {
        m_edit->append(QString::fromWCharArray(str));
    }
}

void Debug::clean()
{
    if (m_edit != nullptr)
    {
        m_edit->clear();
    }
}