#ifndef __PID_H__
#define __PID_H__

#define PID_Uint struct PID
	
struct PID         				//定义PID结构体
{
	int SetValue;   			//设定值
	float Kp; 			//比例系数
	float Ki; 			//积分系数
	float Kd; 			//微分系数
	int LastError;
	int PrevError;
};

void PIDInit(PID_Uint *sptr);
int PID_Calc(int MeasureValue,PID_Uint *sptr);

#endif