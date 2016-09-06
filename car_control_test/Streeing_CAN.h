#pragma once

#include <Windows.h>
#include <iostream>
#include "ControlCAN.h"

#define ussint unsigned short int

class Streeing
{
	VCI_CAN_OBJ vco_receive[64];//64
	VCI_CAN_OBJ vco_receiveCar[64];
	VCI_CAN_OBJ vco_send[8];
	int nDeviceType; /* USBCAN-2A»òUSBCAN-2C»òCANalyst-II */
	int nDeviceInd; /* µÚ1¸öÉè±¸*/

	int nDeviceTypeCar; /* USBCAN-2A»òUSBCAN-2C»òCANalyst-II */
	int nDeviceIndCar; /* µÚ1¸öÉè±¸*/

	//	int dwRel;
	int SendStreeingCommand(BYTE command, unsigned short int steering_angle);

	// Add by Wsine
	int SendStreeingCommandForAX7SRS_R(ussint steerWheelAngle, BYTE steerWheelSpd,
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
		nDeviceType = 4; /* USBCAN-2A»òUSBCAN-2C»òCANalyst-II */
		nDeviceInd = 0; /* µÚ1¸öÉè±¸*/

		nDeviceTypeCar = 4; /* USBCAN-2A»òUSBCAN-2C»òCANalyst-II */
		nDeviceIndCar = 0; /* µÚ1¸öÉè±¸*/

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
	int StartSelf_Driving(unsigned short int steering_angle);
	int StopDSP(unsigned short int steering_angle);

	int ReceiveStreeingAngle();
	int ReceiveVehicleSpeed();
};	


