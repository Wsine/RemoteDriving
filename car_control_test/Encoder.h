#ifndef ENCODER_H_
#define ENCODER_H_

#include "CnComm.h"

#define uchar unsigned char
#define ushort unsigned short

#define STANDARD_ENCODER_VALUE 0
#define STANDARD_OFFSET_VALUE 20

#define STAMP_BRAKE 560
#define LOOSE_BRAKE 720
#define BARKE_MARGIN 15

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
		current = aim = LOOSE_BRAKE;
		margin = BARKE_MARGIN;
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
		if (current > LOOSE_BRAKE && isTurning)
		{
			printf("overflow stop!!!!!\n");
			stop();
			isTurning = false;
			return true;
		}

		if (current < STAMP_BRAKE && isTurning) {
			printf("downflow stop!!!!!\n");
			stop();
			isTurning = false;
			return true;
		}

		if (current >= aim - margin && current <= aim + margin) {
			if (isTurning) {
				stop();
				//printf("Stop now......\n");

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
		aim = LOOSE_BRAKE;
		return TurningToAim();
	}

};

class Encoder_Comm : public CnComm
{
public:
	int iIsHead;				//0:脙禄脫脨脮脪碌陆脥路拢禄1拢潞卤铆脢戮脮脪碌陆0xff;2拢潞卤铆脢戮脮脪碌陆0x81
	bool bIsHead;
	bool bStop;
	int iBufferLen;
	int iBufferFlag;   	//0:卤铆脢戮buffer 5脳脰陆脷脦麓脤卯脗煤拢禄1拢潞卤铆脢戮buffer 5脳脰陆脷脤卯脗煤拢禄
	int iTurnOrent;  //0卤铆脢戮脧貌脥拢脳陋拢卢1卤铆脢戮脧貌脫脪脳陋, 2卤铆脢戮脫脪脳陋拢禄16-6-10 WFH脭枚录脫
	ushort ushEncoderValue;    //1024  0-1023	
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


			// 鎴愬姛璇诲彇鍒版暟鎹?
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
				//printf("鏍￠獙浣嶅け璐n");
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