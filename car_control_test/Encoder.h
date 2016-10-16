
/* 本代码是刹车电机函数声明及实现，通过串口读写
 * 2016.10.16
 */
#ifndef ENCODER_H_
#define ENCODER_H_

#include "CnComm.h"

#define uchar unsigned char
#define ushort unsigned short

#define STANDARD_ENCODER_VALUE 0
#define STANDARD_OFFSET_VALUE 20

using namespace std;
int count_flag = 0;

class Motor_Comm : public CnComm
{
public:
	unsigned short aim;
	unsigned short current;
	unsigned short send;
	unsigned short margin;
	bool isNeedtoStop;
	bool isTurning;

public:
	void setAim(unsigned short _aim) {
		this->aim = _aim;
		//updateDir();
	}
	void setCurrent(unsigned short _current) {
		this->current = _current;
	}
	void setSend(unsigned short _send) {
		this->send = _send;

	}
	void OnReceive() {
		char buffer[256];
		ReadString(buffer, 256);
		cout << "Get string from motor: " << buffer << endl;
	}
public:
	Motor_Comm() {
		current = aim = 550;
		margin = 15;
		isNeedtoStop = false;
		isTurning = false;
	}
	~Motor_Comm() {
		/*setAim(600);
		TurningToAim();
		Sleep(1000);
		stop();*/
	}
	void TurningDirection(int left_right, int speed)
	{
		unsigned char buf[4];
		buf[0] = 0x80;
		buf[1] = (unsigned char)left_right;
		buf[2] = (unsigned char)speed;
		buf[3] = (unsigned char)((buf[0] + buf[1] + buf[2]) & 0x7f);
		Write(buf, 4);
	}

	void stop() {
		unsigned char buf[4];
		buf[0] = 0x80;
		buf[1] = 0;
		buf[2] = 0;
		buf[3] = (unsigned char)((buf[0] + buf[1] + buf[2]) & 0x7f);
		Write(buf, 4);
	}

	bool TurningToAim() {
		if (current > 700 && isTurning)
		{
			printf("overflow stop!!!!!\n");
			stop();
			isTurning = false;
			return true;
		}

		/* 超出区间，停止 */
		if (current >= aim - margin && current <= aim + margin) {
			if (isTurning) {
				stop();
				printf("Stop now......\n");

			}
			//printf("stopping......\n");
			isTurning = false;
			return true;
		}
		// 左转
		else if (current > aim + margin) {
			if (!isTurning) {
				printf("current > aim; aim = %d; current = %d\n", aim, current);
				isTurning = true;
				TurningDirection(0, 80);
			}
		}
		// 右转
		else if (current < aim - margin) {
			if (!isTurning) {
				printf("current < aim aim = %d; current = %d\n", aim, current);
				isTurning = true;
				TurningDirection(1, 80);
			}
		}
		return false;
	}

	bool resetToInit() {
		aim = 650;
		return TurningToAim();
	}

};

class Encoder_Comm : public CnComm
{
public:
	int iIsHead;
	bool bIsHead;
	bool bStop;
	int iBufferLen;
	int iBufferFlag;
	int iTurnOrent;
	ushort ushEncoderValue;
	ushort ushABSValue;
	ushort ushStandard;
	uchar ucOldData[5];
	uchar ucBuffer[5];
	int iStart;
	int iEnd;

	CRITICAL_SECTION encoderCS;

public:
	Encoder_Comm()
	{
		iIsHead = 0;
		bIsHead = false;
		bStop = false;
		iBufferLen = 0;
		iBufferFlag = 0;
		iTurnOrent = 0;
		ushEncoderValue = 0;
		ushABSValue = 0;
		ushStandard = 0;
		iStart = 300;
		iEnd = 80;
		memset(ucOldData, 0, 5);
		memset(ucBuffer, 0, 5);
	}
public:
	bool rec_flag;
	void OnReceive(bool print = false)

	{

		unsigned char buffer[512];
		DWORD dwRealRead;
		//dwRealRead = Read(buffer, 64);
		dwRealRead = Read(buffer, 10);

		for (int i = 0; i < dwRealRead; i++)
		{
			switch (iIsHead)
			{
			case 0:
				if (buffer[i] == 0xFF)
				{
					iIsHead = 1;
					ucBuffer[iBufferLen] = buffer[i];
					iBufferLen++;
				}
				break;
			case 1:
				if (buffer[i] == 0x81)
				{
					iIsHead = 2;
					ucBuffer[iBufferLen] = buffer[i];
					iBufferLen++;
				}
				break;
			case 2:
				if (iBufferLen < 5)
				{
					ucBuffer[iBufferLen] = buffer[i];
					iBufferLen++;
					if (iBufferLen > 4)
					{
						iIsHead = 0;
						iBufferLen = 0;
						iBufferFlag = 1;
						ushEncoderValue = 0;
					}
				}
				break;
			}


			//
			if (iBufferFlag == 1)
			{

				//if (memcmp(ucOldData, ucBuffer, 5) != 0)
				//{
				memcpy(ucOldData, ucBuffer, 5);
				uchar ucTemp = 0;
				ucTemp = ucBuffer[2] + ucBuffer[3];
				if (ucTemp == ucBuffer[4])
				{
					ushort ushTemp;
					ushEncoderValue = ushTemp = 0;
					ushEncoderValue = ucBuffer[2] & 0x0003;
					ushTemp = ucBuffer[3] & 0x00FF;
					ushEncoderValue = ((ushEncoderValue << 8) & 0x0300 | ushTemp) & 0xFFFF;
					if (print)
						printf("EncoderVal = %d\n", ushEncoderValue);
					//rec_flag = true;
					/*if (ushEncoderValue >= iStart || ushEncoderValue <= iEnd)
					{
						bStop = true;
					}*/
				}
				//else
				//{
				//count_flag++;
				//}
				//}
				iBufferFlag = 0;
				memset(ucBuffer, 0, 5);
			}
		}
	}
};
#endif