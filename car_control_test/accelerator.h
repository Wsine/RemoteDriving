//#include "stdafx.h"
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <stdlib.h>
#include <iostream>

#include "CnComm.h"
#include "calculateCRC.h"

#define uchar unsigned char
#define  ON 1
#define  OFF 0

// accelerator.cpp : 定义控制台应用程序的入口点。
//



using namespace std;

class Accelerator_Comm : public CnComm
{
	public:
	~Accelerator_Comm()
	{
		InputVoltage(800);
		ShiftRelay(OFF);//手动驾驶
		Close();
	}
public:
	void OnReceive()
	{
// 		uchar buffer[256];
// 		DWORD len;
// 		len = Read(buffer,256);
// 		//printf("Get info from dev,len is :%d\n", len);
// 		for (int i = 0; i < len; i++)
// 		{
// 			//printf("\n%02x ", buffer[i]);
// 		}
		
	}

public:
	//电压为毫伏	0路为高电压；1路为低电压
	void InputVoltage(int iVoltage)
	{
		uchar uchBuf[13] = {0x03, 0x10, 0x00, 0x60, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		uchar uchVoltage = 0;
		if (iVoltage > 1500)
		{
			//printf("Voltage setting is too large，the value must < 1.5v");
			return;
		}
		iVoltage = iVoltage*4095/10000;
		uchVoltage = iVoltage >> 8 & 0xFF;
		uchBuf[7] = uchVoltage;
		////printf("%02x ", uchVoltage);
		uchVoltage = iVoltage & 0xFF;
		uchBuf[8] = uchVoltage;
		////printf("%02x ", uchVoltage);

		iVoltage /= 2;
		uchVoltage = iVoltage >> 8 & 0xFF;
		uchBuf[9] = uchVoltage;
		////printf("%02x ", uchVoltage);
		uchVoltage = iVoltage & 0xFF;
		uchBuf[10] = uchVoltage;
		////printf("%02x ", uchVoltage);

		uchar uchResult[2];
		unsigned short ushResult;
		ushResult = CRC16(uchBuf, 11, uchResult);
		uchBuf[11] = uchResult[0];
		uchBuf[12] = uchResult[1];

		Write(uchBuf, 13);
		uchar ucBuffer[256];
		DWORD len;
		len = Read(ucBuffer,256);
		//printf("Get info from dev,len is :%d\n", len);
		for (DWORD i = 0; i < len; i++)
		{
			//printf("\n%02x ", ucBuffer[i]);
		}
		//printf("\n");

		if (ucBuffer[0] == 0x03 && ucBuffer[1] == 0x10 && ucBuffer[3] == 0x60)
		{
			//printf("Voltage success!\n");
		}
	}
	
	//继电器开关 参数：ON/OFF
	bool ShiftRelay(int iOnOrOff)
	{
		uchar uchResult[2];
		unsigned short ushResult;
		uchar uchBuf[13] = {0x02, 0x0f, 0x02, 0x00, 0x00, 0x20, 0x04, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00};
		ushResult = CRC16(uchBuf, 11, uchResult);
		uchBuf[11] = uchResult[0];
		uchBuf[12] = uchResult[1];
		if (!iOnOrOff)
		{
			uchBuf[7] = 0;
			uchBuf[9] = 0;
			uchBuf[11] = 0xD2;
			uchBuf[12] = 0xAC;
		}
		//
		
		Write(uchBuf, 13);
		uchar ucBuffer[256];
		DWORD len;
		len = Read(ucBuffer,256);
		//printf("Get info from dev,len is :%d\n", len);
		for (DWORD i = 0; i < len; i++)
		{
			//printf("\n%02x ", ucBuffer[i]);
		}
		//printf("\n");

		if (ucBuffer[0] == 0x02 && ucBuffer[1] == 0x0F && ucBuffer[2] == 02)
		{
			printf("Relay success!\n");
			return true;
		}
		else
		{
			return false;
		}
	}

	//采集电压 //返回值由unsigned short 改为int modify by alex 2016.07.15 11::44::05
	void CollectingVoltage(int *accpede0,int *accpede1)
	{
		uchar uchBuf[12] = {0x01, 0x47, 0x5A, 0x59, 0x4D, 0x42, 0x04, 0x00,0xB0,0x00,0x40,0x80};	//2016.080.20周奎修改两路一起读
		//uchar uchBuf[8] = {0x01, 0x04, 0x00, 0x41, 0x00, 0x01, 0x61, 0xDE};	//1路
		uchar uchVoltage = 0;	
		unsigned short shValue = 0;

		Write(uchBuf, 12);
		uchar ucBuffer[128];
		DWORD len;
		len = Read(ucBuffer,128);
		//printf("Get info from dev,len is :%d\n", len);
		//for (DWORD i = 0; i < len; i++)
		//{
			//printf("\n%02x ", ucBuffer[i]);
		//}
		//printf("\n");

		if (ucBuffer[0] == 0x01 && ucBuffer[1] == 0x04 && ucBuffer[2] == 0x40)
		{
			//printf("Collect Voltage success!\n");
			shValue = ucBuffer[3];
			shValue = shValue << 8 & 0xFF00;
			shValue = shValue | ucBuffer[4];
			//printf("Collect Voltage :%04x\n", shValue);
			int iResult = 0;
			if (shValue < 0x7FFF)
			{
				iResult = (-1)*(0x8000 - shValue)*10000/0x7FFF;
			}
			else
			{
				iResult = (shValue - 0x8000)*10000/0x7FFF;
			}		
			*accpede0=iResult;
			
			iResult=0;
			shValue=0;
			shValue = ucBuffer[11];
			shValue = shValue << 8 & 0xFF00;
			shValue = shValue | ucBuffer[12];
			//printf("Collect Voltage :%04x\n", shValue);
			if (shValue < 0x7FFF)
			{
				iResult = (-1)*(0x8000 - shValue)*10000/0x7FFF;
			}
			else
			{
				iResult = (shValue - 0x8000)*10000/0x7FFF;
			}
			*accpede1=iResult;

		}
//		return -1;
	}

	/*20160802，两路油门一起读，注释掉
	int CollectingVoltage2()
	{
		//uchar uchBuf[8] = {0x01, 0x04, 0x00, 0x40, 0x00, 0x01, 0x30, 0x1E};	//0路
		uchar uchBuf[8] = {0x01, 0x04, 0x00, 0x41, 0x00, 0x01, 0x61, 0xDE};	//1路
		uchar uchVoltage = 0;	
		unsigned short shValue = 0;

		Write(uchBuf, 8);
		uchar ucBuffer[256];
		DWORD len;
		len = Read(ucBuffer,256);
		//printf("Get info from dev,len is :%d\n", len);
		for (DWORD i = 0; i < len; i++)
		{
			//printf("\n%02x ", ucBuffer[i]);
		}
		//printf("\n");

		if (ucBuffer[0] == 0x01 && ucBuffer[1] == 0x04 && ucBuffer[2] == 0x02)
		{
			//printf("Collect Voltage success!\n");
			shValue = ucBuffer[3];
			shValue = shValue << 8 & 0xFF00;
			shValue = shValue | ucBuffer[4];
			//printf("Collect Voltage :%04x\n", shValue);
			int iResult = 0;
			if (shValue < 0x7FFF)
			{
				iResult = (-1)*(0x8000 - shValue)*10000/0x7FFF;
			}
			else
			{
				iResult = (shValue - 0x8000)*10000/0x7FFF;
			}		

			return iResult;

		}
		return -1;
	}
	*/
};

