#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <iostream> 
#include <fstream>

#include "lcm\lcm-cpp.hpp"
#include "lcm\lcm.h"
#include "obu_lcm\ins_info.hpp"
#include "obu_lcm/steering_control_info.hpp"
#include "obu_lcm/steering_feedback_info.hpp"
#include "obu_lcm/CAN_status.hpp"
#include "obu_lcm/CAN_value.hpp"
#include "obu_lcm/accelerate_feedback_info.hpp"

#include "ControlCAN.h"
#include "Streeing_CAN.h"
#include "Encoder.h"

#include "calculateCRC.h"
#include "accelerator.h"

using namespace std;

#define DEBUG_MODE true
#define DEBUG_INDEX 3
void runDebug();
void Brake_Test();
void Brake_Feedback_Test();
void Throttle_Test();

int logger(void);

int input_mv = 700;
int input_dir = 1;//1 加 0 减
DWORD log_period = 20;//log记录周期 50ms

obu_lcm::accelerate_feedback_info g_accelerate_feedback_info;
lcm::LCM g_lcm_ins("udpm://239.255.76.63:7667?ttl=3");//监听INS信息

obu_lcm::ins_info g_ins_info;
extern obu_lcm::CAN_value canValuedata;
extern obu_lcm::CAN_status canStatusdata;
extern obu_lcm::steering_feedback_info steering_feedback;
class Streeing  A60_Streeing;
Accelerator_Comm Com_acc;

#define GET_PERIOD(x) static DWORD last_timer=0;\
static int count = 0;\
DWORD cur_timer = GetTickCount();\
count++;\
if(cur_timer - last_timer >= 1000)\
{\
	cout << x << " period: " << count << endl;\
	count = 0;\
	last_timer = cur_timer;\
}

class Handler_Ins
{
public:
	~Handler_Ins() {}

	void Handle_ins_Message(const lcm::ReceiveBuffer* rbuf,
		const std::string& chan,
		const obu_lcm::ins_info* msg)
	{
		//g_ins_info.longitudinal_speed = msg->longitudinal_speed;
		//g_ins_info.heading_speed = msg->heading_speed;
		//g_ins_info.gps_time = msg->gps_time;
		g_ins_info = *msg;

//		GET_PERIOD("ins_info");
	}
};

//监听INS数据
int Thread_ins_Function(void* param)
{
	while( 0 == g_lcm_ins.handle());
	return 0;
}

//方向盘角度读取
int Thread_steering_feedback_Function(void* param)
{
	int exitcode = 1;
	Streeing* pStreeing = (Streeing*)param;
	while (1 == exitcode)
	{
		exitcode = pStreeing->ReceiveStreeingAngle();
		Sleep(10);
	}
	return 0;
}

//车体数据读取 发动机转速 车速 转向灯 档位 等
int Thread_CANCar_Function(void* param)
{
	int exitcode = 1;
	Streeing* pStreeing = (Streeing*)param;
	while (1 == exitcode)
	{
		exitcode = pStreeing->ReceiveVehicleSpeed();
	}
	return 0;
}

int logger(void)
{
	static uint16_t disp_count = 0;
	ofstream logfile("log_gear_1.txt", std::ofstream::app);
	logfile.precision(13);
	SYSTEMTIME cur_time;
	GetLocalTime(&cur_time);
	long system_time = cur_time.wHour * 60 * 60 * 1000 + cur_time.wMinute * 60 * 1000 + cur_time.wSecond * 1000 + cur_time.wMilliseconds;
	logfile << dec 
		<< system_time << " "
		//<< input_mv << " "
		<< g_ins_info.roll << " "
		<< g_ins_info.heading << " "
		<< g_ins_info.pitch << " "
		<< g_ins_info.lat << " "
		<< g_ins_info.lon << " "
		<< g_ins_info.down_accelerate << " "
		<< g_ins_info.lateral_accelerate << " "
		<< g_ins_info.longitudinal_speed << " "
		<< g_ins_info.longitudinal_accelerate << " "
		<< canValuedata.car_speed << " "
		<< canValuedata.steering_angle << " "
		//<< g_ins_info.pitch_speed << " "
		<< canValuedata.engine_RPM << " "
		//<<steering_feedback.steering_angle << " "
		<< canStatusdata.at_status << " "
		<< canStatusdata.brake_status << " "
		<< g_accelerate_feedback_info.high_sensor_v << " "
		 << g_accelerate_feedback_info.low_sensor_v <<" "
		//<< hex <<  canStatusdata.auto_gear << " "
		<< canValuedata.wheel_speed_FL << " "
		<< canValuedata.wheel_speed_FR << " "
		<< canValuedata.wheel_speed_BL << " "
		<< canValuedata.wheel_speed_BR << " "
		<< endl;
	logfile.close();

	disp_count++;
	if (disp_count % 10 == 0)
	{
		cout << dec
			
			//<< "time:" << system_time
			//<< " roll:" << g_ins_info.roll
			<< " pitch:" << g_ins_info.pitch
			//<< " headig:" << g_ins_info.heading
			//<< " lat:" << g_ins_info.lat
			//<< " lon:" << g_ins_info.lon
			//<< " In:" << input_mv
			//<< " LA:" << g_ins_info.longitudinal_accelerate
			<< " LS:" << g_ins_info.longitudinal_speed
			<< " Inbh:" << g_accelerate_feedback_info.high_sensor_v
			<< " Inbl:" << g_accelerate_feedback_info.low_sensor_v
			<<" SD:"<<canValuedata.car_speed * 3.6
			<<" RPM:" << canValuedata.engine_RPM
			//<< " sa:" << canValuedata.steering_angle
			//<<" SG:"<<steering_feedback.steering_angle
			//<< " CVT:" << canStatusdata.at_status
			//<< " BS:" << canStatusdata.brake_status
			//<< hex << " gear:" <<canStatusdata.auto_gear 
			//<< "pitch speed:" << g_ins_info.pitch_speed
			//<< " WPFL:" << canValuedata.wheel_speed_FL
			//<< " WPFR:" << canValuedata.wheel_speed_FR
			//<< " WPBL:" << canValuedata.wheel_speed_BL
			//<< " WPBR:" << canValuedata.wheel_speed_BR
			<< endl;
	}
	return 1;
}

VOID CALLBACK TimerLogSendRSDS(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	logger();
}

int main(int argc, char* argv[])
{
	if (DEBUG_MODE) {
		runDebug();
		system("pause");
		return 0;
	}

	Handler_Ins handIns;
	Com_acc.Open(8, 9600);
	Com_acc.InputVoltage(700);
	Com_acc.ShiftRelay(OFF);

	A60_Streeing.StartDevice();

	if(!g_lcm_ins.good())
		return 1;

	cout<<"system begin"<<endl;

	g_lcm_ins.subscribe("ins_info", &Handler_Ins::Handle_ins_Message, &handIns);

	CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)Thread_ins_Function, 
				NULL, 0, 0);

	CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_steering_feedback_Function,
		(LPVOID)(&A60_Streeing), 0, 0);

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_CANCar_Function,
		(LPVOID)(&A60_Streeing), 0, 0);

	HANDLE hTimerQueue = NULL;
	HANDLE hTimerlog = NULL;
	hTimerQueue = CreateTimerQueue();
	if (!CreateTimerQueueTimer(&hTimerlog, hTimerQueue,
		(WAITORTIMERCALLBACK)TimerLogSendRSDS, NULL, 100, log_period, WT_EXECUTEINTIMERTHREAD))
	{
		cout << "creat timer log fail" << endl;
		return -1;
	}

	//Com_acc.ShiftRelay(ON);
	while(true)
	{
		//Com_acc.InputVoltage(input_mv);
		//Sleep(50);
		//g_accelerate_feedback_info.high_sensor_v = Com_acc.CollectingVoltage();
		//Sleep(20);
    	//g_accelerate_feedback_info.low_sensor_v = Com_acc.CollectingVoltage2();
		//Sleep(20);
		int a=0,b=0;
		Com_acc.CollectingVoltage(&a,&b);
		g_accelerate_feedback_info.high_sensor_v = a;
		g_accelerate_feedback_info.low_sensor_v = b;
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

void runDebug() {
	if (DEBUG_INDEX == 1) {
		Brake_Test();
	} else if (DEBUG_INDEX == 2) {
		Brake_Feedback_Test();
	} else if (DEBUG_INDEX == 3) {
		Throttle_Test();
	}
}

void Brake_Test() {
	Motor_Comm motorComm;
	// 打开设备，参数一是设备管理器的端口COM8
	motorComm.Open(8, 9600);
	unsigned char address = ;
	// 键盘输入测试
	int code, speed = 60;
	while (true) {
		printf("Please enter your code: ");
		scanf("%d", &code);
		if (code == 1) {
			motorComm.DriveForward(0x80, speed);
			Sleep(500);
			motorComm.stop();
		} else if (code == 2) {
			motorComm.DriveBackward(0x80, speed);
			Sleep(500);
			motorComm.stop();
		} else if (code == 3) {
			motorComm.stop();
		} else if (code == -1) {
			break;
		} else {
			continue;
		}
	}
}

void Brake_Feedback_Test() {
	Encoder_Comm encoderComm;
	// 打开设备，参数一是设备管理器的端口COM8
	encoderComm.Open(8, 9600);
	// 持续打开访问设备
	int maxFire = 50;
	while (maxFire--) {
		encoderComm.OnReceive();
		Sleep(1000);
	}
}

void Throttle_Test() {
	Accelerator_Comm throttleComm;
	// 打开设备，参数一是设备管理器的端口COMn
	throttleComm.Open(8, 9600);
	throttleComm.InputVoltage(700);
	throttleComm.ShiftRelay(OFF);
	// 键盘输入测试
	int code, voltage = 0;
	while (true) {
		printf("Please enter your code: ");
		scanf("%d", &code);
		if (code == 1) {
			scanf("%d", &voltage);
			throttleComm.InputVoltage(voltage);
			printf("Current voltage = %d\n", voltage);
		} else if (code == 2) {
			int left, right;
			throttleComm.CollectingVoltage(&left, &right);
			printf("left = %d, right = %d\n", left, right);
		} else if (code == -1) {
			break;
		} else {
			continue;
		}
	}
}