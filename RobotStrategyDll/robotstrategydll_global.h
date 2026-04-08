// robotstrategydll_global.h
// DLL全局定义

#ifndef ROBOTSTRATEGYDLL_GLOBAL_H
#define ROBOTSTRATEGYDLL_GLOBAL_H

#include <QtCore/qglobal.h>

// DLL导出/导入宏定义
#if defined(ROBOTSTRATEGYDLL_LIBRARY)
#define ROBOTSTRATEGYDLL_EXPORT Q_DECL_EXPORT  // 编译DLL时导出符号
#else
#define ROBOTSTRATEGYDLL_EXPORT Q_DECL_IMPORT  // 使用DLL时导入符号
#endif

#endif
