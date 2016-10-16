
/* 本代码是转向类头文件，需要通过CAN口向底层读写
 * 2016.10.13
 */
#pragma once

#include <Windows.h>
#include <iostream>
#include "ControlCAN.h"



class Streeing
{
	VCI_CAN_OBJ vco_receive[64];//64
	VCI_CAN_OBJ vco_receiveCar[64];
	VCI_CAN_OBJ vco_send[8];
	int nDeviceType; /* USBCAN-2A USBCAN-2C CANalyst-II */
	int nDeviceInd;

	int nDeviceTypeCar; /* USBCAN-2A USBCAN-2C CANalyst-II */
	int nDeviceIndCar;

	//	int dwRel;
	int SendStreeingCommand( short steerWheelAngle, BYTE steerWheelSpd,
		BYTE vehicleSpd, BYTE engineSpd, BYTE steerWheelStatus);

	int nCANInd_1;
	VCI_INIT_CONFIG vic_1;

	int nCANInd_2;
	VCI_INIT_CONFIG vic_2;

	int nCANInd_3;
	VCI_INIT_CONFIG vic_3;

public:
	short int current_steeringangle;

	Streeing()
	{
		nDeviceType = 4; /* USBCAN-2A USBCAN-2C CANalyst-II */
		nDeviceInd = 0;

		nDeviceTypeCar = 4; /* USBCAN-2A USBCAN-2C CANalyst-II */
		nDeviceIndCar = 0; 

		nCANInd_1 = 0;
		nCANInd_2 = 0;
		nCANInd_3 = 1;
	}

	~Streeing()
	{
		CloseDevice();
	}

	int StartDevice(void );
	int CloseDevice();

	int StartHuman_Driving(unsigned short int steering_angle);
	int StartSelf_Driving(short steeringangle, BYTE steerWheelSpd);
	int StopDSP(unsigned short int steering_angle);

	/* 用于反馈转向的信息 */
	int ReceiveStreeingAngle();
	int ReceiveVehicleSpeed();
};