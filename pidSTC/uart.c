
#include "uart.h"
#include <STC12C5A60S2.H>

#include "pid.h"
extern Speed_Measure;
extern PID_Uint sPID;
extern PID_Uint *sptr;
extern SPK_Info();
extern unsigned char mode;
extern unsigned char PWM_DES;
void ByteToFloat(unsigned char *b,float *f)
{
	char i=0;
	unsigned char *t=(unsigned char *)f;
	for(i=3;i>=0;i--)
	{
		t[i]=*(b+3-i);
	}
}
void ByteToInt(unsigned char *b,int *f)
{
	char i=0;
	unsigned char *t=(unsigned char *)f;
	for(i=1;i>=0;i--)
	{
		t[i]=*(b+1-i);
	}
}

void UartInit(void)		//115200bps@11.0592MHz 864B/s
{
	PCON &= 0x7F;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x04;		//独立波特率发生器时钟为Fosc,即1T
	BRT = 0xFD;		//设定独立波特率发生器重装值
	AUXR |= 0x01;		//串口1选择独立波特率发生器为波特率发生器
	AUXR |= 0x10;		//启动独立波特率发生器
	ES=1;
}
unsigned char recvBuf[20]={0};
unsigned char recvLen=0;
void UartRecv() interrupt 4
{
	unsigned char recv=0;
	static unsigned char index=0;
	static bit isReceiving=0;
	if(RI)
	{
		RI=0;
		recv=SBUF;
		if(isReceiving)
		{
			if(recvLen==0)
			{
				recvLen=recv;
				
				Speed_Measure=recvLen;
				
			}else{
				recvBuf[index++]=recv;
				
				if(index==recvLen)
				{
					isReceiving=0;
					UartProc();
				}
			}
		}
		else if(recv==Flag_First && isReceiving==0)
		{
			isReceiving=1;
			recvLen=0;
			index=0;
			
			Speed_Measure=1;
			
		}
	}
	if(TI)
		TI=0;
}

void UartProc()
{
	switch(recvBuf[0])
	{
		case Flag_Start:
			ET0=1;EX0=1;
		
			break;
		case Flag_Stop:
			ET0=0;EX0=0;
		
			break;
		case Flag_SetPWM:
			mode=1;
			PWM_DES=recvBuf[1];
			break;
		case Flag_SetTar:
			ByteToInt(recvBuf+1,&(sptr->SetValue));
			mode=0;
			break;
    case Flag_SetPID:
			ByteToFloat(recvBuf+1,&(sptr->Kp));
			ByteToFloat(recvBuf+5,&(sptr->Ki));
			ByteToFloat(recvBuf+9,&(sptr->Kd));
			break;
		case Flag_GetPID:
			UartSendPID(sptr);
			break;
		default:
			break;
	}
}
void UartSend(unsigned char *dat)
{
	char j=100;
	while(*dat!='\0')
	{
		SBUF=*dat;
		j=100;
    while(j--);
		dat++;
	}
}
void UartSendByte(unsigned char dat)
{
	char j=100;
	SBUF=dat;
	while(j--);
}
void UartSendFloat(float dat)
{
	unsigned char *p=(unsigned char *)&dat;
	char i=0;
	char j=100;
	float t=dat;
	for(i=3;i>=0;i--)
	{
		SBUF=*(p+i);
		j=100;
    while(j--);
	}
}
void UartSendInt(int dat)
{
	unsigned char *p=(unsigned char *)&dat;
	char i=0;
	char j=100;
	float t=dat;
	for(i=1;i>=0;i--)
	{
		SBUF=*(p+i);
		j=100;
    while(j--);
	}
}
void UartSendSpeed(int speed)
{
	UartSendByte(Flag_First);
	UartSendByte(3);
	UartSendByte(Flag_GetSpeed);
	UartSendInt(speed);
}
void UartSendPWM(unsigned char pwm)
{
	UartSendByte(Flag_First);
	UartSendByte(2);
	UartSendByte(Flag_GetPWM);
	UartSendByte(pwm);
}
void UartSendPID(PID_Uint *sptr)
{
	UartSendByte(Flag_First);
	UartSendByte(13);
	UartSendByte(Flag_GetPID);
	UartSendFloat(sptr->Kp);
	UartSendFloat(sptr->Ki);
	UartSendFloat(sptr->Kd);
}

