
/* 本代码是转向类实现文件，需要通过CAN口向底层读写
 * 2016.10.14
 */
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "ControlCAN.h"
#include "Streeing_CAN.h"

#include "lcm/lcm-cpp.hpp"
#include "lcm/lcm.h"

#include "obu_lcm/steering_control_info.hpp"
#include "obu_lcm/steering_feedback_info.hpp"
#include "obu_lcm/CAN_status.hpp"
#include "obu_lcm/CAN_value.hpp"
#include "obu_lcm/accelerate_feedback_info.hpp"

using namespace std;

obu_lcm::CAN_value canValuedata;
obu_lcm::CAN_status canStatusdata;
obu_lcm::steering_feedback_info steering_feedback;


/* @func	StartDevice 	开启设备 */
int Streeing::StartDevice(void)
{
	int dwRelOpenDevice;
	int dwRelVCI_InitCAN;
	int dwRelVCI_InitCAN_1;
	int dwRelVCI_InitCAN_2;

	nDeviceType = 4; /* USBCAN-2A USBCAN-2C CANalyst-II */
	nDeviceInd = 0;

	nDeviceTypeCar = 4; /* USBCAN-2A USBCAN-2C CANalyst-II */
	nDeviceIndCar = 0;

	nCANInd_1 = 0;
	nCANInd_2 = 0;
	nCANInd_3 = 1;

	dwRelOpenDevice = VCI_OpenDevice(nDeviceType, nDeviceInd, 0);

	if (dwRelOpenDevice != 1)
	{
		cout << "VCI_OpenDevice fail! " << endl;
		return -1;
	}

	/*初始 CAN*/
	vic_1.AccCode = 0x80000008;
	vic_1.AccMask = 0xFFFFFFFF;//屏蔽码，现在代表全部接收
	vic_1.Filter = 1;  //接受所有帧
	vic_1.Timing0 = 0x00;
	vic_1.Timing1 = 0x1C;//波特率500Kbps
	vic_1.Mode = 0;		//正常模式，为1的时候只接收
	
	/* @func	VCI_InitCAN 			初始化指定的CAN通道
	 * @param	nDeviceType 			设备类型号
	 * @param	nDeviceInd 				设备索引号，从0开始增加
	 * @param	nCANInd_ 				第几路CAN，即对应卡的CAN通道号
	 * @param	&vic_ 					初始化参数结构体，描述此通道CAN接收帧的情况
	 */
	dwRelVCI_InitCAN = VCI_InitCAN(nDeviceType, nDeviceInd, nCANInd_1, &vic_1);
	if (dwRelVCI_InitCAN != 1)
	{
		VCI_CloseDevice(nDeviceType, nDeviceInd);
		cout << "dwRelVCI_InitCAN fail!" << endl;
		return -1;
	}

	
	vic_2.AccCode = 0x80000008;
	vic_2.AccMask = 0xFFFFFFFF;
	vic_2.Filter = 1;
	vic_2.Timing0 = 0x00;
	vic_2.Timing1 = 0x1C;
	vic_2.Mode = 0;
	dwRelVCI_InitCAN_1 = VCI_InitCAN(nDeviceType, nDeviceInd, nCANInd_2, &vic_2);
	if (dwRelVCI_InitCAN_1 != 1)
	{
		VCI_CloseDevice(nDeviceType, nDeviceInd);
		cout << "dwRelVCI_InitCAN_1 fail!" << endl;
		return -1;
	}


	vic_3.AccCode = 0x80000008;
	vic_3.AccMask = 0xFFFFFFFF;
	vic_3.Filter = 1;
	vic_3.Timing0 = 0x00;
	vic_3.Timing1 = 0x1C;
	vic_3.Mode = 1;
	dwRelVCI_InitCAN_2 = VCI_InitCAN(nDeviceTypeCar, nDeviceIndCar, nCANInd_3, &vic_3);
	if (dwRelVCI_InitCAN_2 != 1)
	{
		VCI_CloseDevice(nDeviceTypeCar, nDeviceIndCar);
		cout << "dwRelVCI_InitCAN_2 fail!" << endl;
		return -1;
	}


	/* @func	VCI_StartCAN 			启动CAN卡的某一个CAN通道
	 * @param	nDeviceType 			设备类型号
	 * @param	nDeviceInd 				设备索引号，从0开始增加
	 * @param	nCANInd_ 				第几路CAN，即对应卡的CAN通道号
	 */
	if (VCI_StartCAN(nDeviceType, nDeviceInd, nCANInd_1) != 1)
	{
		VCI_CloseDevice(nDeviceType, nDeviceInd);
		cout << "VCI_StartCAN 1 fail!!" << endl;
		return -1;
	}

	if (VCI_StartCAN(nDeviceType, nDeviceInd, nCANInd_2) != 1)
	{
		VCI_CloseDevice(nDeviceType, nDeviceInd);
		cout << "VCI_StartCAN 2 fail!" << endl;
		return -1;
	}

	if (VCI_StartCAN(nDeviceTypeCar, nDeviceIndCar, nCANInd_3) != 1)
	{
		VCI_CloseDevice(nDeviceTypeCar, nDeviceIndCar);
		cout << "¹VCI_StartCAN 3 fail!" << endl;
		return -1;
	}

	VCI_ClearBuffer(nDeviceType, nDeviceInd, nCANInd_1);
	VCI_ClearBuffer(nDeviceTypeCar, nDeviceIndCar, nCANInd_3);
	return 1;
}


/* @func	SendStreeingCommand 	向底层发送转动命令
 * @param	steerWheelAngle 		需要转动的角度
 * @param 	steerWheelSpd 			转速
 * @param	vehicleSpd 				车速，默认0x00
 * @param	engineSpd 				发动机转速，默认0x00
 * @param	steerWheelStatus 		0x20，此状态代表控制权交给
 */
int Streeing::SendStreeingCommand(short  steerWheelAngle, BYTE steerWheelSpd,
	BYTE vehicleSpd, BYTE engineSpd, BYTE steerWheelStatus)
{
	int dwRel;
	vco_send[0].ID = (UINT)(0x0000B500);//固定格式的帧的首部
	vco_send[0].RemoteFlag = 0;//远程帧
	vco_send[0].ExternFlag = 1; //扩展帧
	vco_send[0].DataLen = 7;//长度为7个byte

	//steerWheelAngle = 1024 + steerWheelAngle; //?

	steerWheelAngle = steerWheelAngle * 10;

	BYTE highangel = steerWheelAngle >> 8;

	BYTE lowangel = (steerWheelAngle << 8) >> 8;

	//short temp_angle;
	//temp_angle = ((int)highangel << 8) + (int)lowangel;
	//temp_angle = temp_angle / 10;

	/* 发送给底层帧的格式
	 * byte[0]			转向角右移8位？
	 * byte[1]			转向角
	 * byte[2]			转角速度
	 * byte[3]			车速，默认0x00
	 * byte[4]			发动机转速，默认0x00
	 * byte[5]			工作状态，0x20代表无人驾驶模式
	 * byte[6]			？默认0x00
	 */
	BYTE b0 = vco_send[0].Data[0] = highangel;
	BYTE b1 = vco_send[0].Data[1] = lowangel;
	BYTE b2 = vco_send[0].Data[2] = steerWheelSpd;
	BYTE b3 = vco_send[0].Data[3] = vehicleSpd;
	BYTE b4 = vco_send[0].Data[4] = engineSpd;
	BYTE b5 = vco_send[0].Data[5] = steerWheelStatus;
	//BYTE temp_result = b0^b1^b2^b3^b4^b5;
	BYTE b6 = vco_send[0].Data[6] = 0x00;

	/* @func	VCI_Transmit 			发送帧的函数，返回值为实际发送成功帧的数量
	 * @param	nDeviceType 			设备类型号
	 * @param	nDeviceInd 				设备索引号，从0开始增加
	 * @param	nCANInd_ 				第几路CAN，即对应卡的CAN通道号
	 * @param	vco_send				要发送的内容
	 * @param	len 					每一次发送帧的数量，这里每一次只发一个帧，所以就是1
	 */
	dwRel = VCI_Transmit(nDeviceType, nDeviceInd, nCANInd_1, vco_send, 1);
	return 1;
}


/*
 * @func	StartHuman_Driving 		转回手动驾驶
 * To be implemented
 */
int Streeing::StartHuman_Driving(unsigned short int steeringangle)
{
	//SendStreeingCommand(0x10, steeringangle,);
	return 1;
}


/* @func	StartSelf_Driving 		自动驾驶或者遥控驾驶模式，油门由此程序接口
 * @param	steeringangle 			即将要转的角度
 * @param	steerWheelSpd 			转速
 */
int Streeing::StartSelf_Driving(short steeringangle, BYTE steerWheelSpd)
{
	SendStreeingCommand(steeringangle, steerWheelSpd, 0x00, 0x00, 0x20);
	return 1;
}


/*
 * @func	StopDSP 		？
 * To be implemented
 */
int Streeing::StopDSP(unsigned short int steeringangle)
{
	//SendStreeingCommand(0x00, steeringangle);
	return 1;
}


/*
 * @func	CloseDevice 		关闭CAN
 */
int Streeing::CloseDevice()
{
	int dwRel;
	dwRel = VCI_CloseDevice(nDeviceType, nDeviceInd);
	if (dwRel != 1)
	{
		cout << "Can Close Errors! " << endl;
		return -1;
	}
	return 1;
}


/* @func	ReceiveStreeingAngle 	读取当前转速情况 */
int Streeing::ReceiveStreeingAngle(void)
{
	int dwRel;
	double current_steeringangle = 0;
	short temp_angle = 0;

	/* @func	VCI_Receive 			接收帧的函数，从指定设备的CAN通道缓冲接收区读取数据，返回值为实际读取到的帧数
	 * @param	nDeviceType 			设备类型号
	 * @param	nDeviceInd 				设备索引号，从0开始增加
	 * @param	nCANInd_ 				第几路CAN，即对应卡的CAN通道号
	 * @param	vco_receive				接收的内容
	 * @param	Len 					本次接收最多帧的数目
	 * @param	WaitTime				缓冲区无数据，函数阻塞等待时间，毫秒级
	 */
	dwRel = VCI_Receive(nDeviceType, nDeviceInd, nCANInd_1, vco_receive, 60, 0);

	if (dwRel > 0 && vco_receive[0].ID == (UINT)(0x0000A500))
	{
		BYTE high = (BYTE)(vco_receive[0].Data[0]);
		BYTE low = (BYTE)(vco_receive[0].Data[1]);

		temp_angle = ((int)high << 8) + (int)low;
		current_steeringangle = temp_angle / 10.0;
		//current_steeringangle = current_steeringangle - 1024 + 23;

		//std::cout << "temp_angle: " << temp_angle << endl;
		steering_feedback.steering_angle = current_steeringangle;
		//steering_feedback.steering_angle_speed = (double)vco_receive[0].Data[2];
		memcpy(&(steering_feedback.steering_angle_speed), &(vco_receive[0].Data[2]), sizeof(double));
		steering_feedback.steering_angle_speed = steering_feedback.steering_angle_speed * 4;

		//std::cout<<"current_steeringangle = "<<current_steeringangle<<endl;
		//std::cout << "current_steering_angle_speed = " << steering_feedback.steering_angle_speed << endl;
	}
	else if (dwRel == -1)
	{
		//cout << "no streeing CAN data. " << endl;
		VCI_CloseDevice(nDeviceType, nDeviceInd);
		VCI_OpenDevice(nDeviceType, nDeviceInd, 0);
	}
	else
	{
		//cout << "othet CAN data" << endl;

		//std::cout << "ID =" << vco_receive[0].ID << endl;

	}
	return 1;
}

int Streeing::ReceiveVehicleSpeed(void)
{
	int dwRelCar;
	dwRelCar = VCI_Receive(nDeviceTypeCar, nDeviceIndCar, nCANInd_3, vco_receiveCar, 60, 0);
	if (dwRelCar > 0)
	{
		//Sleep(20);
		//输出发动机的转速信息  EngineRPM * 0.125  同时发送油门电压信息
		if (vco_receiveCar[0].ID == 0x180)
		{
			canValuedata.engine_RPM = (vco_receiveCar[0].Data[0] * 256 + vco_receiveCar[0].Data[1])*0.125;
			//cout<<"RPM: "<<canValuedata.engine_RPM << endl;
		}
		if (vco_receiveCar[0].ID == 0x174)
		{
			canStatusdata.auto_gear = vco_receiveCar[0].Data[3]*256+vco_receiveCar[0].Data[4];
			//cout<<"RPM: "<<canValuedata.engine_RPM << endl;
		}
		//BCM 转向灯的信息 BCM3 60D
		if (vco_receiveCar[0].ID == 0x60D)
		{
			// 直行  28 06 00 00    00 00 00 00
			// 左转  28 26 00 00    00 00 00 00
			// 右转  28 46 00 00    00 00 00 00
			// 双闪  28 66 00 00    00 00 00 00
			//第二位在直行与当前转向灯状态之间跳变

			/*	cout<<"  BCM: "; 13
			switch(vco_receiveCar[0].Data[1])
			{
			case 0x06: cout<<"S";break; //0000 0110
			case 0x26: cout<<"L";break; //0010 0110
			case 0x46: cout<<"R";break;// 0100 0110
			case 0x66: cout<<"D";break;// 0110 0110
			default: break;
			}*/
			canStatusdata.flashing_status = vco_receiveCar[0].Data[1];
			//cout<<"flashing_status: "<<canStatusdata.flashing_status<<endl;
		}
		if (vco_receiveCar[0].ID == 0x35D)
		{
			// 36
			//刹车
			//Data[4] = 16 有刹车 0001 0000
			//	canStatusdata.flashing_status = vco_receiveCar[0].Data[5];

			BYTE brake_status_temp = vco_receiveCar[0].Data[4] & 0x10;
			if (brake_status_temp == 0)
				canStatusdata.brake_status = 0;
			else
			{
				canStatusdata.brake_status = 1;
			}
		}
		//CVT 变速箱的信息 421
		if (vco_receiveCar[0].ID == 0x421)
		{
			//08 00 P档 0000 1000
			//10 00 R档 0001 0000
			//20 00 D档 0010 0000
			//18 00 N档 0001 1000
			//cout<<"CVT: "<<bin<<vco_receiveCar[0].Data[0]<<endl;
			//cout<<"  CVT: ";
			//switch(vco_receiveCar[0].Data[0])
			//{
			//case 0x08: cout<<"P";break;
			//case 0x10: cout<<"R";break;
			//case 0x20: cout<<"D";break;
			//case 0x18: cout<<"N";break;
			//default: break;
			//}
			canStatusdata.at_status = (int32_t)(vco_receiveCar[0].Data[0] & 0xf8);//去除低3位，同时与之前的程序兼容
			//cout<<"at_status: "<<canStatusdata.at_status<<endl;
		}
		//ABS1 前轮轮速信号 284
		if (vco_receiveCar[0].ID == 0x284)
		{
			//wheel_speed_FR  第1、2位为车速*0.042
			//wheel_speed_FL  第3、4位为车速*0.042 
			//VehicleSpeed  第5、6位为车速*0.01   
			canValuedata.wheel_speed_FR = (vco_receiveCar[0].Data[0] * 256 + vco_receiveCar[0].Data[1])*0.042 / 3.6;//m/s
			canValuedata.wheel_speed_FL = (vco_receiveCar[0].Data[2] * 256 + vco_receiveCar[0].Data[3])*0.042 / 3.6;//m/s
			canValuedata.car_speed = (vco_receiveCar[0].Data[4] * 256 + vco_receiveCar[0].Data[5])*0.01 / 3.6;//m/s

			//cout<<"  ASD: "<<canValuedata.car_speed<<endl;
			//	g_lcm_vehicle.publish("CAN_value",&canValuedata);

		}
		//ABS2 后轮轮速信号 285
		if (vco_receiveCar[0].ID == 0x285)
		{
			//WheelspeedRR  第1、2位为车速*0.042
			//WheelspeedRL  第3、4位为车速*0.042 
			canValuedata.wheel_speed_BR = (vco_receiveCar[0].Data[0] * 256 + vco_receiveCar[0].Data[1])*0.042 / 3.6;//m/s
			canValuedata.wheel_speed_BL = (vco_receiveCar[0].Data[2] * 256 + vco_receiveCar[0].Data[3])*0.042 / 3.6;//m/s
			//cout<<"  ARR: "<<(vco_receiveCar[0].Data[0]*256+vco_receiveCar[0].Data[1])*0.042;//<<endl;
			//cout<<"  ARL: "<<(vco_receiveCar[0].Data[2]*256+vco_receiveCar[0].Data[3])*0.042;//<<endl;
			//	g_lcm_vehicle.publish("CAN_value",&canValuedata);
		}
		//数据发布add by alex 2016.05.22 10:22:13
	}
	else if (dwRelCar == -1)
	{
		cout << "no CAN data received" << endl;
		VCI_CloseDevice(nDeviceType, nDeviceInd);
		VCI_OpenDevice(nDeviceType, nDeviceInd, 0);
	}
	return 1;
}

