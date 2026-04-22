#ifndef USB340DLL_DEF
#define USB340DLL_DEF _declspec(dllimport)
#endif
#include <windows.h>
//初始化设备,默认VID为0xffff，PID为2
bool USB340DLL_DEF InitUSB340();

//使用前需要使用此函数来指明您的设备正确的PID和VID，然后再使用之
bool USB340DLL_DEF ResetUSB340_PID_VID(int VID, int PID);

//检测设备是否存在，存在返回true，否则false
bool USB340DLL_DEF CheckIfExist();

//获取当前设备的频率,返回频率402 - 527,0为失败
int USB340DLL_DEF GetFre();

//设置通信频率,fre:402 - 527,op:0 or 1,1表示写入ROM永久保存，频率更改只需要使用op=0
bool USB340DLL_DEF SetFre(int fre,bool op);
		
//修改车频率,CarNum为需要修改频率的车号,0-11，0表示修改所有11辆车都要修改
			//NewCarFre为设置的目标频率,400-527之间
			//ChangeCarFreOp为1表示修改车频率后保存，否则不保存
bool USB340DLL_DEF ChangeCarFre(BYTE CarNum,int NewCarFre,bool ChangeCarFreOp);

//修改车号,OldNum为旧号,0-11，0表示修改所有1-11全部小车，NewNum为新号,1-11之间
bool USB340DLL_DEF ChangeCarNum(BYTE OldNum,BYTE NewNum);


//组装车的速度，CarNum为要操作的车号,0-11，0表示所有车，Left和Right分别为左右轮速度
bool USB340DLL_DEF BuildCarSpeed(BYTE CarNum,int Left,int Right);

//发送所有车的速度，参数num为车的总数，范围1 - 11，若设置为0，表示从1号车到num号车
bool USB340DLL_DEF SendAll(int num);

//发送单辆车的速度，参数num为车号,范围1 - 11，若设置为0，表示发送num号车
bool USB340DLL_DEF SendOneCar(int num);