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
private:
	/* 1 for increase
	 * 0 for stop
	 * -1 for decrease
	 */
	unsigned short dir;
	unsigned short aim;
	unsigned short current;
	unsigned short send;

public:
	void setAim(unsigned short _aim) {
		this->aim = _aim;
		this->dir = this->aim - this->current;
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
		dir = aim = current = send = 0;
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
		buf[1] = 0x10;
		buf[2] = 0;
		buf[3] = (unsigned char)((buf[0] + buf[1] + buf[2]) & 0x7f);
		Write(buf, 4);
	}

	void TurningToAim() {
		if (dir > 0) {
			if (current < aim) {
				TurningDirection(1, 40);
			} else {
				stop();
				dir = 0;
			}
		} else if (dir < 0) {
			if (current > aim) {
				TurningDirection(0, 40);
			} else {
				stop();
				dir = 0;
			}
		}
	}

};

class Encoder_Comm : public CnComm
{
public:
	int iIsHead;				//0:Ã»ÓÐÕÒµ½Í·£»1£º±íÊ¾ÕÒµ½0xff;2£º±íÊ¾ÕÒµ½0x81
	bool bIsHead;
	bool bStop;
	int iBufferLen;
	int iBufferFlag;   	//0:±íÊ¾buffer 5×Ö½ÚÎ´ÌîÂú£»1£º±íÊ¾buffer 5×Ö½ÚÌîÂú£»
	int iTurnOrent;  //0±íÊ¾ÏòÍ£×ª£¬1±íÊ¾ÏòÓÒ×ª, 2±íÊ¾ÓÒ×ª£»16-6-10 WFHÔö¼Ó
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
	void OnReceive()
	
	{
		
		unsigned char buffer[512];
		DWORD dwRealRead;
		dwRealRead = Read(buffer, 64);

		for (int i = 0; i < dwRealRead; i++)
		{
			switch (iIsHead)
			{
			case 0:
				if (buffer[i] == 0xFF)
				{
					iIsHead = 1;
					ucBuffer[iBufferLen] =buffer[i];
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

			
			// 成功读取到数据
			if (iBufferFlag == 1)
			{
				
				if (memcmp(ucOldData, ucBuffer, 5) != 0)
				{
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
						printf("EncoderVal = %d\n", ushEncoderValue);
						rec_flag = true;
						if (ushEncoderValue >= iStart || ushEncoderValue <= iEnd)
						{
							bStop = true;
						}
					}
					else
					{
						printf("校验位失败\n");
						count_flag ++;
					}
				}
				iBufferFlag = 0;
				memset(ucBuffer, 0, 5);
			}
		}
	}
};
#endif