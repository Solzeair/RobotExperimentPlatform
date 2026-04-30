
#ifndef _august_globe_data_structure
#define _august_globe_data_structure
#include "math.h"

#define Inf 100
#define pi 3.14159265358979323846
#define Pi 3.14159265358979323846
#define INFLARGE 10000000000.0
#define ABSSMALL 0.0000000001
#define sqrt2 1.414213562373
#define CLOCKWISE 1
#define ANTICLOCK -1
#define NOCLOCK  0
#define FOREWARD 1
#define BACKWARD -1
typedef struct tagRECTDIMENSION{
	int length;
	int width;
}RECTDIMENSION;

typedef struct tagdbRECTDIMENSION{
	double length;
	double width;
}dbRECTDIMENSION;

typedef struct tagdbPOINT{
	double x;
	double y;
}dbPOINT,BallInformation,ballInformation;

typedef struct tagBALLMOVEREC {
	double velocity;
	double theta;
}BALLMOVEREC, WALLVD, VELOCITYREC;

typedef struct tagROBOTPOSTURE	//场地物体位姿信息
{
	int x;	//物体中心的坐标x
	int y;  //物体中心的坐标y,该坐标是以显示坐标为基准的
	int theta;//方向角
}ROBOTPOSTURE, RobotInfor;

typedef struct tagdbROBOTPOSTURE	//场地物体浮点位姿信息
{
	double x;	//物体中心的坐标x
	double y;  //物体中心的坐标y,该坐标是以显示坐标为基准的
	double theta;//弧度方向角
	tagdbROBOTPOSTURE() {
		x = y = theta = 0.0;
	}
	tagdbROBOTPOSTURE(tagdbROBOTPOSTURE & info) {
		x = info.x;
		y = info.y;
		theta = info.theta;
	}
}dbROBOTPOSTURE, RobotInford;

typedef struct tagROBOTCOMMAND{//小车命令
	int LeftValue;
	int RightValue;
}INTROBOTCOMMAND;

typedef struct tagdbLRWheelVelocity{//小车轮速值浮点型
	double LeftValue;
	double RightValue;
}dbLRWheelVelocity, ROBOTCOMMAND;

typedef struct tagFIELDPARAM{
	dbRECTDIMENSION rectReal;//实际赛场尺寸
	dbRECTDIMENSION rectMonitor;//场地显示大小
	dbRECTDIMENSION rectGoal;//球门大小
	dbRECTDIMENSION RealGoal;//实际球门大小
	COLORREF GroundColor;
	COLORREF LineColor;
	COLORREF BallColor;
	float MonitorBaffle;//挡板大小
	float RealBaffle;//实际挡板大小
	float MonitorBallDiameter;
	float RealBallDiameter;//实际球的直径
	float MonitorVehicleSize;
	float RealVehicleSize;//实际车的直径
	float WheelDiameter;//轮径
}FIELDPARAM;

typedef struct tagPANELPARAM{//面板控制参数
	int SimulationTimeSlice;//仿真周期
	int PlayerNumber;//人数（三对三、五对五）
	BOOL DiscernOn;
	BOOL RecordOn;
	BYTE OperationMode;//比赛模式
	BYTE HomeSide;//我方半场
	BYTE StartSide;//开球方
	BYTE StartMode;//开球模式
	CComboBox *StrategySel;//策略选择
	CComboBox *RaceSortSel;////比赛类型
}PANELPARAM;

typedef struct tagOBSTACLE{//障碍线段
	POINT A;
	POINT B;
}OBSTACLE;

typedef struct tagdbOBSTACLE{//障碍线段
	dbPOINT A;
	dbPOINT B;
}dbOBSTACLE, SEGMENT;

#endif

