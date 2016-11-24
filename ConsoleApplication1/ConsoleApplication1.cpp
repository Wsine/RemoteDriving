// ConsoleApplication1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <strstream>
#include <windows.h>
#include <math.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "LogitechSteeringWheelLib.lib")
#include "LogitechSteeringWheelLib.h"
using namespace std;

/* tool fucntion : parse button value to string*/
int maxPedal = 2500;
string thisAdd = "192.168.191.1";
string toGear(unsigned char * button);
string toLRSlice(unsigned char * button);
int wheelToStandardDataType(int wheel);
float pedalToStandardDataType(int pedal);
float brakeToStandardDataType(int brake);
int threadForHearbeatListener(void * param);

int main()
{

	string serverAdd;
	int serverPort;
	cout << "Input command server IP address(xxx.xxx.xxx.xxx):";
	cin >> serverAdd;
	cout << "Input command server port:";
	cin >> serverPort;
	cout << "Input this client IP address(xxx.xxx.xxx.xxx):";
	cin >> thisAdd;

	DIJOYSTATE2ENGINES* pState;
	
	if (LogiSteeringInitialize(true)) {
		printf("init finish\n");
	}
	else {
		printf("init error:could not find device\n");
		return 0;
	}
	

	int ID = 1;
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		return 0;
	}
	SOCKET sclient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(serverPort);
	sin.sin_addr.S_un.S_addr = inet_addr(serverAdd.c_str());

	int len = sizeof(sin);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadForHearbeatListener, NULL, 0, 0);

	while (1) {
		if (LogiUpdate()) {
			pState = LogiGetStateENGINES(0);
			strstream ss;
			/*	Notice what the value represents
			(steering) wheel
			left ---- middle ---- right
			-32768      0         32767

			(accelerator) pedal, brake, clutch
			press --------------- release
			-32768				  32767
			*/

			SYSTEMTIME sysTime;
			GetLocalTime(&sysTime);
			unsigned long int mTime;
			mTime = sysTime.wHour * 3600 * 1000 + sysTime.wMinute * 60 * 1000 + sysTime.wMinute * 1000 + sysTime.wMilliseconds;
			ss << wheelToStandardDataType(pState->lX) << "&" << pedalToStandardDataType(pState->lY)
				<< "&" << brakeToStandardDataType(pState->lRz);
			ss << "&" << toGear(pState->rgbButtons);
			ss << "&" << ID << "&" << mTime;
			ID++;
			string strResult;
			string showResult;
			ss >> strResult;		
			sendto(sclient, strResult.c_str(), strlen(strResult.c_str()), 0, (sockaddr *)&sin, len);
			strstream out;
			out << "方向盘值：" << wheelToStandardDataType(pState->lX) << "油门值：" << pedalToStandardDataType(pState->lY)
				<< "刹车值：" << brakeToStandardDataType(pState->lRz);
			out << "档位值：" << toGear(pState->rgbButtons);
			out >> showResult;
			cout << showResult << endl;
			Sleep(100);
		}
	}
	closesocket(sclient);
	WSACleanup();
	system("pause");
	return 0;
}







/*	According to SDK
the button 8 ~ 14 represents Gear
if the button is pressed, the value is 128
otherwise, the value is 0
*/

string toGear(unsigned char * button) {
	if (button[8] != 0) {
		return "1";
	}
	else if (button[9] != 0) {
		return "2";
	}
	else if (button[10] != 0) {
		return "3";
	}
	else if (button[11] != 0) {
		return "4";
	}
	else if (button[12] != 0) {
		return "5";
	}
	else if (button[13] != 0) {
		return "6";
	}
	else if (button[14] != 0) {
		return "R";
	}

	return  "N";
}

/*  Using binary representation
left       right
button[5] button[4] LRSlice
0          0         0
0          128       1
128        0         2
128        128       3
*/
string toLRSlice(unsigned char * button) {
	//TODO: need debounce
	if (button[4] == button[5]) {
		return button[4] == 0 ? "LR=0" : "LR=3";
	}
	else {
		return button[4] > button[5] ? "LR=1" : "LR=2";
	}
}


/* According to the rule, change the wheel, pedal and brake to standard type. */
int wheelToStandardDataType(int wheel) {
	int result;
	if (wheel >= 0) {
		result = (double)floor(wheel * 450 / 32767);
		result = -result;
	}
	else {
		wheel = -wheel;
		result = (double)ceil (wheel * 450 / 32768);
	}
	return result;
}

float pedalToStandardDataType(int pedal) {
	float result;
	if (pedal >= 0) {
		pedal = pedal - 32767;
		pedal = -pedal;
	}
	else {
		pedal = -pedal;
		pedal += 32767;
	}


	result = pedal * 5000 / 65535;

	if (result > maxPedal) {
		result = maxPedal;
	}
	return result;
}

float brakeToStandardDataType(int brake) {
	float result;
	if (brake >= 0) {
		brake = brake - 32767;
		brake = -brake;
	}
	else {
		brake = -brake;
		brake += 32767;
	}
	result = brake * 5000 / 65535;
	return result;
}

int threadForHearbeatListener(void * param) {
	/* 计时器 */
	DWORD start, end;
	bool isConnected = true;
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	
	if (WSAStartup(sockVersion, &wsaData) != 0) {
		return 1;
	}
	SOCKET serSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serSocket == INVALID_SOCKET) {
		printf("socket error !");
		return 1;
	}
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8000);
	serAddr.sin_addr.S_un.S_addr = inet_addr(thisAdd.c_str());;
	if (bind(serSocket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR) {
		printf("bind error !");
		closesocket(serSocket);
		return 1;
	}
	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
	start = GetTickCount();
	while (true) {		
		end = GetTickCount();
		char recvData[255];
		int iMode = 1;
		ioctlsocket(serSocket, FIONBIO, (u_long FAR*) &iMode);
		int ret = recvfrom(serSocket, recvData, 255, 0, (sockaddr *)&remoteAddr, &nAddrLen);
		
		if (ret > 0) {
			recvData[ret] = 0x00;
			printf("收到指令指令服务器返回的心跳包：\r\n");
			printf(recvData);
			isConnected = true;
			start = GetTickCount();
			end = GetTickCount();
		}
		if (end - start <= 1500) {
			isConnected = true;
		}
		else {
			if (isConnected == true) {
				printf("\n*****未连接到指令服务器！*****\r\n");
				isConnected = false;
			}
		}
	}
	closesocket(serSocket);
	WSACleanup();
	return 0;
}