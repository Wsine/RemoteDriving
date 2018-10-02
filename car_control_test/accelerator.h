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

// accelerator.cpp : �������̨Ӧ�ó������ڵ㡣
//



using namespace std;

class Accelerator_Comm : public CnComm
{
	public:
	~Accelerator_Comm()
	{
		InputVoltage(800);
		ShiftRelay(OFF);//�ֶ���ʻ
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
	//��ѹΪ����	0·Ϊ�ߵ�ѹ��1·Ϊ�͵�ѹ
	void InputVoltage(int iVoltage)
	{
		uchar uchBuf[13] = {0x03, 0x10, 0x00, 0x60, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		uchar uchVoltage = 0;
		if (iVoltage > 1500)
		{
			printf("Voltage setting is too large��the value must < 1.5v");
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
		printf("Get voltage from dev,len is :%d\n", len);
		for (DWORD i = 0; i < len; i++)
		{
			printf("%02x ", ucBuffer[i]);
		}
		printf("\n");

		if (ucBuffer[0] == 0x03 && ucBuffer[1] == 0x10 && ucBuffer[3] == 0x60)
		{
			printf("Voltage success!\n");
		}
	}
	
	//�̵������� ������ON/OFF
	bool ShiftRelay(int iOnOrOff)
	{
		uchar uchResult[2];
		unsigned short ushResult;
		uchar uchBuf[10] = { 0x02, 0x0f, 0x02, 0x00, 0x00, 0x03, 0x01, 0x07, 0x8F, 0x62 };

		if (!iOnOrOff)
		{
			uchBuf[7] = 0;
		}
		ushResult = CRC16(uchBuf, 8, uchResult);
		uchBuf[8] = uchResult[0];
		uchBuf[9] = uchResult[1];

		Write(uchBuf, 10);
		uchar ucBuffer[256];
		DWORD len;
		len = Read(ucBuffer, 256);
		printf("Get shift info from dev,len is :%d\n", len);
		for (int i = 0; i < len; i++)
		{
			printf("\n%02x ", ucBuffer[i]);
		}
		printf("\n");

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

	//�ɼ���ѹ 
	unsigned short CollectingVoltage()
	{
		uchar uchBuf[8] = { 0x01, 0x04, 0x00, 0x40, 0x00, 0x01, 0x30, 0x1E };	//0·
		//uchar uchBuf[8] = {0x01, 0x04, 0x00, 0x41, 0x00, 0x01, 0x61, 0xDE};	//1·
		uchar uchVoltage = 0;
		unsigned short shValue = 0;

		Write(uchBuf, 8);
		uchar ucBuffer[256];
		DWORD len;
		len = Read(ucBuffer, 256);
		printf("Get collecting info from dev,len is :%d\n", len);
		for (int i = 0; i < len; i++)
		{
			printf("%02x ", ucBuffer[i]);
		}
		printf("\n");

		if (ucBuffer[0] == 0x01 && ucBuffer[1] == 0x04 && ucBuffer[2] == 0x02)
		{
			printf("Collect Voltage success!\n");
			shValue = ucBuffer[3];
			shValue = shValue << 8 & 0xFF00;
			shValue = shValue | ucBuffer[4];
			printf("Collect Voltage :%04x\n", shValue);
			int iResult = 0;
			if (shValue < 0x7FFF)
			{
				iResult = (-1)*(0x8000 - shValue) * 10000 / 0x7FFF;
			}
			else
			{
				iResult = (shValue - 0x8000) * 10000 / 0x7FFF;
			}

			return iResult;

		}
		return -1;
	}
};

