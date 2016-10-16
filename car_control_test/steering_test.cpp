
/* 工控机为Windows的底层控制代码，适合于远程遥控项目，上层需传输转向、油门、刹车以及档位（切换模式）的信息
 * 通过CAN实现对方向盘的控制
 * 通过串口实现对油门和刹车的控制
 * 通过局域网LCM接收上层发来的控制指令
 * 2016.10.13
 */

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <iostream> 
#include <fstream>
#include <string>
#include <cstdlib>

#include "lcm\lcm-cpp.hpp"
#include "lcm\lcm.h"
#include "obu_lcm\ins_info.hpp"
#include "obu_lcm/steering_control_info.hpp"
#include "obu_lcm/steering_feedback_info.hpp"
#include "obu_lcm/CAN_status.hpp"
#include "obu_lcm/CAN_value.hpp"
#include "obu_lcm/accelerate_feedback_info.hpp"

#include "lcm/lcm-cpp.hpp"
#include "exlcm/car_t.hpp"
#include "exlcm/steering_control_info.hpp"
#include "exlcm/steering_feedback_info.hpp"

#include "ControlCAN.h"
#include "Streeing_CAN.h"
#include "Encoder.h"

#include "calculateCRC.h"
#include "accelerator.h"

using namespace std;

#define INIT_MODE 0
#define MANUAL_MODE 2
#define REMOTE_MODE 3
#define AUTO_MODE 4

/* LCM的组播地址与端口号，需要与上层发送端一致 */
lcm::LCM xyz_lcm("udpm://239.255.76.68:7667?ttl=2&recv_buf_size=40860000");


/****************控制参数***************************/
short control_mode = INIT_MODE;
short steerWheelAngle_remote = 0; //转向角
BYTE steerWheelSpd_remote = 0x10;	  //方向盘转向速度，初始化10
short pedal_remote = 0; //接收到目的油门踏板状态，最开始初始化为0mv，代表未踩
unsigned short current_pedal = 0; //当前实际油门的状态
short brake_remote = 0; //接收到目的刹车踏板状态，最开始初始化为0mv，代表未踩
short current_brake = 0;//当前实际刹车电机的状态
short write_brake = 0;//通过PID计算，实际应该继续拉动刹车电机的值
/***************************************************/


/*
bool isRemotedriving = false;
bool isSendingCAN = false;
bool isNeedtoStopSend = false;


extern obu_lcm::CAN_value canValuedata;
extern obu_lcm::CAN_status canStatusdata;
extern obu_lcm::steering_feedback_info steering_feedback;
*/

/* 转向、油门、刹车 */
class Streeing  AX7_Streeing;
Accelerator_Comm throttleComm;
Motor_Comm motorComm;

/* 把string类型的转角速度转换位16进制的byte类型，返回值为byte
 * @param	steering_angle_speed_str	angle speed to be converted.
 */
BYTE STRINGtoBYTE(string steering_angle_speed_str)
{
	short temp_speed = atoi(steering_angle_speed_str.c_str());

	temp_speed = temp_speed / 4;//度转换
	temp_speed += 10;

	if (temp_speed <= 255 && temp_speed >= 0)
	{
		if (temp_speed < 10)
		{
			return 0x22;
		}

		string s = "";
		if (temp_speed == 0)
			s = "0";

		while (temp_speed != 0)
		{
			if (temp_speed % 16 >9)
				s += temp_speed % 16 - 10 + 'A';
			else
				s += temp_speed % 16 + '0';
			temp_speed = temp_speed / 16;
		}
		reverse(s.begin(), s.end());//反转

		BYTE tem, tem1, tem2;

		if (s.length() >= 2)
		{
			switch (s[0])
			{
			case '0': {tem1 = 0x00; break; }
			case '1': {tem1 = 0x10; break; }
			case '2': {tem1 = 0x20; break; }
			case '3': {tem1 = 0x30; break; }
			case '4': {tem1 = 0x40; break; }
			case '5': {tem1 = 0x50; break; }
			case '6': {tem1 = 0x60; break; }
			case '7': {tem1 = 0x70; break; }
			case '8': {tem1 = 0x80; break; }
			case '9': {tem1 = 0x90; break; }
			case 'A': {tem1 = 0xA0; break; }
			case 'B': {tem1 = 0xB0; break; }
			case 'C': {tem1 = 0xC0; break; }
			case 'D': {tem1 = 0xD0; break; }
			case 'E': {tem1 = 0xE0; break; }
			case 'F': {tem1 = 0xF0; break; }
			}
			switch (s[1])
			{
			case '0': {tem2 = 0x00; break; }
			case '1': {tem2 = 0x01; break; }
			case '2': {tem2 = 0x02; break; }
			case '3': {tem2 = 0x03; break; }
			case '4': {tem2 = 0x04; break; }
			case '5': {tem2 = 0x05; break; }
			case '6': {tem2 = 0x06; break; }
			case '7': {tem2 = 0x07; break; }
			case '8': {tem2 = 0x08; break; }
			case '9': {tem2 = 0x09; break; }
			case 'A': {tem2 = 0x0A; break; }
			case 'B': {tem2 = 0x0B; break; }
			case 'C': {tem2 = 0x0C; break; }
			case 'D': {tem2 = 0x0D; break; }
			case 'E': {tem2 = 0x0E; break; }
			case 'F': {tem2 = 0x0F; break; }
			}
		}
		else if (s.length() == 1)
		{
			tem1 = 0x00;
			switch (s[0])
			{
			case '0': {tem2 = 0x00; break; }
			case '1': {tem2 = 0x01; break; }
			case '2': {tem2 = 0x02; break; }
			case '3': {tem2 = 0x03; break; }
			case '4': {tem2 = 0x04; break; }
			case '5': {tem2 = 0x05; break; }
			case '6': {tem2 = 0x06; break; }
			case '7': {tem2 = 0x07; break; }
			case '8': {tem2 = 0x08; break; }
			case '9': {tem2 = 0x09; break; }
			case 'A': {tem2 = 0x0A; break; }
			case 'B': {tem2 = 0x0B; break; }
			case 'C': {tem2 = 0x0C; break; }
			case 'D': {tem2 = 0x0D; break; }
			case 'E': {tem2 = 0x0E; break; }
			case 'F': {tem2 = 0x0F; break; }
			}
		}
		else
		{
			cout << "string to byte error!!!" << endl;
		}
		return  tem1^tem2;
	}
	else
		return  0x11;
}

/* 通过LCM收到数据，并进行类型转换，各线程将调用转换后的数据
 * @param	steering_angle_speed_str	转向速度		非负数，为0时设置为10
 * @param	steering_angle_str 			转向角			-450~450
 * @param	pedal 						油门大小		0~2500
 * @param	brake 						刹车大小		0~5000
 * @param	gear						模式切换		3为远程
 */
class Handler {
public:
	~Handler() {}

	void handleMessage(const lcm::ReceiveBuffer* rbuf,
		const std::string& chan,
		const exlcm::car_t* msg)
	{
		std::string steering_angle_str = msg->steering_angle;
		std::string steering_angle_speed_str = msg->steering_angle_speed;
		std::string pedal = msg->pedal;
		std::string brake = msg->brake;
		std::string gear = msg->gear;


		/* 将通过LCM获得的上层控制数据包转换成对应类型,对全局变量进行写操作
		 * @param	steerWheelSpd_remote	BYTE
		 * @param	steerWheelAngle_remote	short
		 * @param	pedal_remote	short
		 * @param	brake_remote	short
		 * @param	gear	string	档位信息，用来切换状态
		 */
		steerWheelAngle_remote = atoi(steering_angle_str.c_str());
		brake_remote = atoi(brake.c_str());
		pedal_remote = atoi(pedal.c_str());
		/* 当控制端发来角速度小于10时，这里赋值为一个固定角速度，保持方向盘固定不变。*/
		steerWheelSpd_remote = STRINGtoBYTE(steering_angle_speed_str);

		if (gear[0] == '4') {
			control_mode = MANUAL_MODE;
			printf("current mode = MANUAL\n");
		}
		else if (gear[0] == '3') {
			control_mode = REMOTE_MODE;
			printf("current mode = REMOTE\n");
		}
		else if (gear[0] == '2') {
			control_mode = AUTO_MODE;
			printf("current mode = AUTO\n");
		}
		//printf("Received message on channel \"%s\":\n", chan.c_str());
		//std::cout << "wheel: " << steerWheelAngle_remote << ", pedal: " << pedal << ", brake: " << brake << ", gear" << gear << std::endl;
	}
};


/* 监听INS数据 */
int Thread_ins_Function(void* param) {
	int i = 0;
	while (0 == xyz_lcm.handle())
	{
		/*cout << "NO." << i << endl;
		cout << "trying angle: " << steerWheelAngle_remote;
		cout << "  current angle: " << steering_feedback.steering_angle;
		printf("  speed: %x ", steerWheelSpd_remote);
		cout << "  trying brake: " << brake_remote;
		cout << "  current brake: " << current_brake << endl;
		i++;*/
	}
	return 0;
}


/* 方向盘角度读取，若需要通过LCM返回给上层，则需要另开启一个LCM通道 */
int Thread_steering_feedback_Function(void* param) {
	int exitcode = 1;
	Streeing* pStreeing = (Streeing*)param;
	while (1 == exitcode)
	{
		exitcode = pStreeing->ReceiveStreeingAngle();
		//steering_lcm.publish("steering_feedback_info", &steering_feedback);
		Sleep(10);
	}
	return 0;
}


/* 发送steer控制命令到CAN口，需要根据此时切换状态判断是否发送 */
int Thread_CANCar_Function(void* param) {
	Streeing* pStreeing = (Streeing*)param;
	while (true) {
		if (control_mode != INIT_MODE && control_mode != MANUAL_MODE) {
			pStreeing->StartSelf_Driving(steerWheelAngle_remote, steerWheelSpd_remote);
		}
		Sleep(10);
	}
	return 0;
}


/* 设定工作区间，判断刹车电机停止工作的条件 */
void Thread_Brake_Function(void* param) {
	short oldValue = -1;
	while (true) {
		short diff = brake_remote - oldValue;
		/* 将刹车电机拉线的工作区间设置为-100~100，最终落在此区间内停止拉线 */
		if (!motorComm.isTurning && (diff > 100 || diff < -100)) {
			oldValue = brake_remote;
			/* 刹车拉线的目标值 */
			unsigned short value = (unsigned short)(brake_remote * -1 * 274.0 / 5000 + 650);
			//printf("value = %d\n", value);
			motorComm.setAim(value);
			//motorComm.isNeedtoStop = false;
		}
		Sleep(100);
	}
}

/* 编码器读回此时电机转动情况 */
void Thread_Brake_feedback_Function(void* param) {
	Encoder_Comm encoderComm;
	/* 打开编码器设备，端口COM9，波特率38400 */
	encoderComm.Open(9, 38400);
	//int counter = 0;
	while (true) {
		encoderComm.OnReceive();
		if (encoderComm.ushEncoderValue != 0)
			motorComm.setCurrent(encoderComm.ushEncoderValue);
		//Sleep(2);
	}
}

/* 判断control_mode并根据模式控制刹车电机 */
int Thread_Brake_calculate_Function(void* param) {
	short last_control_mode = control_mode;
	while (true) {
		if (control_mode == MANUAL_MODE && last_control_mode != control_mode) {
			while (!motorComm.resetToInit());
			last_control_mode = control_mode;
		}
		else if (control_mode == REMOTE_MODE) {
			motorComm.TurningToAim();
			last_control_mode = REMOTE_MODE;
		}
		Sleep(100);
	}
	return 0;
}

/* 根据当前模式，控制油门 */
int Thread_Pedal_Function(void* param) {
	/* 油门串口，端口COM10，波特率9600 */
	throttleComm.Open(10, 9600);
	throttleComm.InputVoltage(400);
	throttleComm.ShiftRelay(OFF);
	while (true) {
		/* 如果当前模式切为手动驾驶，关油门 */
		if (control_mode == MANUAL_MODE && throttleComm.isShiftRelayON) {
			throttleComm.InputVoltage(400);
			throttleComm.ShiftRelay(OFF);
			throttleComm.isShiftRelayON = false;
		}
		/* 远程驾驶模式 */
		else if (control_mode == REMOTE_MODE) {
			/* 初始化，给400mv */
			if (!throttleComm.isShiftRelayON) {
				throttleComm.InputVoltage(400);
				throttleComm.ShiftRelay(ON);
				throttleComm.isShiftRelayON = true;
			}
			// 400 ~ 500
			short value = (short)(pedal_remote / 12.5 + 400);
			throttleComm.InputVoltage(value);
		}
		// AUTO_MODE implement here
		Sleep(10);
	}
	throttleComm.ShiftRelay(OFF);
	return 0;
}

/* 读取此时油门状态 */
int Thread_Pedal_feedback_Function(void* param) {
	//To be continued
	//current_pedal = throttleComm.CollectingVoltage();
	return 0;
}

int main(int argc, char* argv[]) {

	/* 开启LCM，监听上层指令数据包 */
	if (!xyz_lcm.good()) {
		cout << "steering_lcm is bad...." << endl;
		return -1;
	}
	Handler handlerObject;
	xyz_lcm.subscribe("COMMAND", &Handler::handleMessage, &handlerObject);

	/* 开启CAN，控制方向盘 */
	if (AX7_Streeing.StartDevice() == -1) {
		cout << "StartDevic failed......" << endl;
	}

	/* 开启串口，控制刹车电机，串口COM8，波特率9600 */
	motorComm.Open(8, 9600);
	cout << "system begin" << endl;

	/* 监听INS */
	CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_ins_Function,
		NULL, 0, 0);
	/* 向底层方向盘写数据 */
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_CANCar_Function,
		(LPVOID)(&AX7_Streeing), 0, 0);
	/* 监听方向盘返回数据 */
	CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_steering_feedback_Function,
		(LPVOID)(&AX7_Streeing), 0, 0);

	/* 设置刹车电机工作区间 */
	/*CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_Brake_Function,
		NULL, 0, 0);*/
	
	/* 刹车电机编码器工作，读回数据 */
	/*CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_Brake_feedback_Function,
		NULL, 0, 0);*/
	/* 根据当前模式，控制刹车电机工作 */
	/*CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_Brake_calculate_Function,
		NULL, 0, 0);*/
	/* 油门串口工作 */
	/*CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_Pedal_Function,
		NULL, 0, 0);*/
	/* 油门串口返回当前值 */
	/*CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_Pedal_feedback_Function,
		NULL, 0, 0);*/

	while (1) {
		Sleep(10000);
	}
	return 0;
}