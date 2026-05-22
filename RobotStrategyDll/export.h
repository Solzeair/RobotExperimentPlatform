// export.h
// DLL 导出/导入宏统一管理

#ifndef EXPORT_H
#define EXPORT_H

// 如果没有定义 ROBOTSTRATEGYDLL_EXPORTS，且不是导入模式，则默认为导出
#if !defined(ROBOTSTRATEGYDLL_EXPORTS) && !defined(ROBOTSTRATEGYDLL_IMPORTS)
#define ROBOTSTRATEGYDLL_EXPORTS  // 默认为导出模式
#endif

#ifdef ROBOTSTRATEGYDLL_EXPORTS
#define ROBOTSTRATEGYDLL_API __declspec(dllexport)
#else
#define ROBOTSTRATEGYDLL_API __declspec(dllimport)
#endif

#endif // EXPORT_H