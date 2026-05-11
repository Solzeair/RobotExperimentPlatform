#pragma once
//utili.h	包含一些常用的宏定义

//客户区的大小
#define CLIENT_H        700
#define CLIENT_W        1200
//显示区的大小
#define DISPLAY_H       480
#define DISPLAY_W       640
//显示对话框的大小
#define DISPLAY_DLG_H   512
#define DISPLAY_DLG_W   DISPLAY_W
//DEBUG文本区的大小
#define DEBUG_H         (CLIENT_H - DISPLAY_DLG_H - 20)
#define DEBUG_W         DISPLAY_DLG_W
//操作区的大小
#define CONTROL_W       (CLIENT_W - DISPLAY_DLG_W)
#define CONTROL_H       CLIENT_H
//机器人数量
#define MAX_ROBOT_NUM   5
//机器人编号
// 一些公用的结构体

// 地面标定数据结构
//
// 重要提示：这是唯一的权威定义。
//   DemarcateDlg.h 之前使用不同的 'flag' 类型（char vs bool）重新定义了这些结构。
//   该重复定义已被移除；所有翻译单元必须包含 utili.h 以获取此定义。
// ---------------------------------------------------------------

// 标定步骤生成的每个像素的地面信息。
// x, y  : 真实世界场地坐标，单位为厘米。
// flag  : 1 = 像素位于场地边界内，0 = 边界外。
typedef struct tagGroundInfo {
    float x;      // 坐标映射X (cm)
    float y;      // 坐标映射Y (cm)
    char  flag;   // 1 = inside field, 0 = outside
} GroundInfo;

typedef struct tagGround {//场地参数结构体，用于记录场地的各种参数
	GroundInfo groundInfo[DISPLAY_W][DISPLAY_H];  //图象内每个点的坐标映射和标志
}Ground;

// ---------------------------------------------------------------

// 标定模板文件名

// ---------------------------------------------------------------

// Ground 结构体的二进制文件 – 由
// DemarcateDlg::onButtonSave / onButtonLoad 生成和消费。

inline constexpr const char* kGroundDataFile = "ground.dat";

// 操作员在首次标定时选择的 25 个像素空间控制点。
// 后续运行时自动加载，无需操作员重新点击。

inline constexpr const char* kPointsTemplateFile = "points_template.dat";

// ---------------------------------------------------------------

// 透视校正 – 4 个角点控制点

// 当操作员点击"保存"时由 DemarcateDlg 写入，
// 由 DisplayDlg::applyPerspectiveCorrection() 读取。

// ---------------------------------------------------------------

//（无需额外的结构体；4 个 QPoint 嵌入在
//  points_template.dat 中，作为前 4 个条目
//  通过 DemarcateDlg::saveTemplate() 保存 – 参见 DemarcateDlg.cpp）