
/* 本代码用来调试测试油门，通过串口读写
 * 2016.10.16
 */
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <iostream> 
#include <fstream>


#include "lcm/lcm-cpp.hpp"
#include "exlcm/car_t.hpp"

#include "ControlCAN.h"
#include "Streeing_CAN.h"

#include "calculateCRC.h"
#include "accelerator.h"

using namespace std;

int input_mv = 700;
int input_dir = 1;//1 加 0 减
DWORD log_period = 20;//log记录周期 50ms

//obu_lcm::accelerate_feedback_info g_accelerate_feedback_info;

Accelerator_Comm Com_acc;


int main(int argc, char* argv[])
{
	Com_acc.Open(8, 9600);
	Com_acc.InputVoltage(700);
	Com_acc.ShiftRelay(OFF);

	cout<<"system begin"<<endl;

	Com_acc.ShiftRelay(ON);
	while(true)
	{
		Com_acc.InputVoltage(input_mv);
		Sleep(50);

		int a=0,b=0;
		Com_acc.CollectingVoltage(&a,&b);

		cout << "high_sensor_v: " << a <<endl;
		cout << "low_sensor_v: " << b <<endl;
	    //g_accelerate_feedback_info.high_sensor_v = a;
		//g_accelerate_feedback_info.low_sensor_v = b;
		Sleep(20);

		//Sleep(150);

		//if (input_dir == 1)
		//{
		//	input_mv = input_mv + 50;
		//}
		//else
		//{
		//	input_mv = input_mv - 50;
		//}

		//if (input_mv >= 1300) Sleep(2000);

		//if (input_mv >= 1300) input_dir = 0;
		//if (input_mv <= 700) input_dir = 1;
	}

	while (1)
	{
		Sleep(2000);
	}
	return 0;
}

