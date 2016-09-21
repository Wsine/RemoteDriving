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

string toGear(unsigned char * button);
string toLRSlice(unsigned char * button);
int wheelToStandardDataType(int wheel);
float pedalToStandardDataType(int pedal);
float brakeToStandardDataType(int brake);


int main()
{

	string serverAdd;
	int serverPort;
	cout << "Input command server IP address(xxx.xxx.xxx.xxx):";
	cin >> serverAdd;
	cout << "Input command server port:";
	cin >> serverPort;
	//int timeDelay;
	//cout << "Input sleep time:";
	//cin >> timeDelay;
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
	//sin.sin_port = htons(8000);
	sin.sin_port = htons(serverPort);
	//sin.sin_addr.S_un.S_addr = inet_addr("192.168.0.227");
	sin.sin_addr.S_un.S_addr = inet_addr(serverAdd.c_str());

	int len = sizeof(sin);


	while (1) {
		if (LogiUpdate()) {
			pState = LogiGetStateENGINES(0);
			//to get the return results in string
			strstream ss;
			string wheel = "wheel=";
			string pedal = "pedal=";
			string brake = "brake=";
			string id = "id=";
			string time = "time=";
			/*	Notice what the value represents
			(steering) wheel
			left ---- middle ---- right
			-32768      0         32767

			(accelerator) pedal, brake, clutch
			press --------------- release
			-32768				  32767
			*/

			/*ss << wheel << pState->lX << "&" << pedal <<  pState->lY << "&" << brake << pState->lRz;
			ss << "&" << clutch << pState->rglSlider[1] << "&" << toGear(pState->rgbButtons);
			ss << "&" << toLRSlice(pState->rgbButtons);*/
			SYSTEMTIME sysTime;
			GetLocalTime(&sysTime);
			unsigned long int mTime;
			mTime = sysTime.wHour * 3600 * 1000 + sysTime.wMinute * 60 * 1000 + sysTime.wMinute * 1000 + sysTime.wMilliseconds;
			ss << wheelToStandardDataType(pState->lX) << "&" << pedalToStandardDataType(pState->lY)
				<< "&" << brakeToStandardDataType(pState->lRz);
			ss << "&" << toGear(pState->rgbButtons);
			ss << "&" << ID << "&" << mTime;
			ID++;
			cout << "ID: " << ID << endl;
			string strResult;
			ss >> strResult;
						
			sendto(sclient, strResult.c_str(), strlen(strResult.c_str()), 0, (sockaddr *)&sin, len);
			cout << strResult << endl;
			//Sleep(timeDelay);	
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