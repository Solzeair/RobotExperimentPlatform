#pragma once
//utili.h	包含一些常用的宏定义

//客户区的大小
#define CLIENT_H	700
#define CLIENT_W	1200

//显示区的大小
#define	DISPLAY_H	480
#define DISPLAY_W	640

//显示对话框的大小
#define DISPLAY_DLG_H 512
#define DISPLAY_DLG_W DISPLAY_W


//DEBUG文本区的大小
#define DEBUG_H	CLIENT_H - DISPLAY_DLG_H - 20
#define DEBUG_W	DISPLAY_DLG_W

//操作区的大小
#define CONTROL_W	CLIENT_W - DISPLAY_DLG_W
#define CONTROL_H	CLIENT_H

#define MAX_ROBOT_NUM 5

// 一些公用的结构体
typedef struct tagGroundInfo {//一个点的坐标映射和标志
	float  x;  //x坐标映射
	float  y;  //y坐标映射
	bool flag;  //该点是否为场地内的点的标志:1在场内，0不在场内
}GroundInfo;
typedef struct tagGround {//场地参数结构体，用于记录场地的各种参数
	GroundInfo groundInfo[DISPLAY_W][DISPLAY_H];  //图象内每个点的坐标映射和标志
}Ground;

//const CString confPath = L"C:\\RobotConf\\";

const CString confPath = L"C:\\Users\\19794\\Desktop\\";