
/* ���ػ�ΪWindows�ĵײ���ƴ��룬�ʺ���Զ��ң����Ŀ���ϲ��贫��ת�����š�ɲ���Լ���λ���л�ģʽ������Ϣ
 * ͨ��CANʵ�ֶԷ����̵Ŀ���
 * ͨ������ʵ�ֶ����ź�ɲ���Ŀ���
 * ͨ��������LCM�����ϲ㷢���Ŀ���ָ��
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

/* LCM���鲥��ַ��˿ںţ���Ҫ���ϲ㷢�Ͷ�һ�� */
lcm::LCM xyz_lcm("udpm://239.255.76.68:7667?ttl=2&recv_buf_size=40860000");


/****************���Ʋ���***************************/
short control_mode = INIT_MODE;
short steerWheelAngle_remote = 0; //ת���
BYTE steerWheelSpd_remote = 0x10;	  //������ת���ٶȣ���ʼ��10
short pedal_remote = 0; //���յ�Ŀ������̤��״̬���ʼ��ʼ��Ϊ0mv������δ��
unsigned short current_pedal = 0; //��ǰʵ�����ŵ�״̬
short brake_remote = 0; //���յ�Ŀ��ɲ��̤��״̬���ʼ��ʼ��Ϊ0mv������δ��
short current_brake = 0;//��ǰʵ��ɲ�������״̬
short write_brake = 0;//ͨ��PID���㣬ʵ��Ӧ�ü�������ɲ�������ֵ
/***************************************************/


/*
bool isRemotedriving = false;
bool isSendingCAN = false;
bool isNeedtoStopSend = false;


extern obu_lcm::CAN_value canValuedata;
extern obu_lcm::CAN_status canStatusdata;
extern obu_lcm::steering_feedback_info steering_feedback;
*/

/* ת�����š�ɲ�� */
class Streeing  AX7_Streeing;
Accelerator_Comm throttleComm;
Motor_Comm motorComm;

/* ��string���͵�ת���ٶ�ת��λ16���Ƶ�byte���ͣ�����ֵΪbyte
 * @param	steering_angle_speed_str	angle speed to be converted.
 */
BYTE STRINGtoBYTE(string steering_angle_speed_str)
{
	short temp_speed = atoi(steering_angle_speed_str.c_str());

	temp_speed = temp_speed / 4;//��ת��
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
		reverse(s.begin(), s.end());//��ת

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

/* ͨ��LCM�յ����ݣ�����������ת�������߳̽�����ת���������
 * @param	steering_angle_speed_str	ת���ٶ�		�Ǹ�����Ϊ0ʱ����Ϊ10
 * @param	steering_angle_str 			ת���			-450~450
 * @param	pedal 						���Ŵ�С		0~2500
 * @param	brake 						ɲ����С		0~5000
 * @param	gear						ģʽ�л�		3ΪԶ��
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


		/* ��ͨ��LCM��õ��ϲ�������ݰ�ת���ɶ�Ӧ����,��ȫ�ֱ�������д����
		 * @param	steerWheelSpd_remote	BYTE
		 * @param	steerWheelAngle_remote	short
		 * @param	pedal_remote	short
		 * @param	brake_remote	short
		 * @param	gear	string	��λ��Ϣ�������л�״̬
		 */
		steerWheelAngle_remote = atoi(steering_angle_str.c_str());
		brake_remote = atoi(brake.c_str());
		pedal_remote = atoi(pedal.c_str());
		/* �����ƶ˷������ٶ�С��10ʱ�����︳ֵΪһ���̶����ٶȣ����ַ����̶̹����䡣*/
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


/* ����INS���� */
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


/* �����̽Ƕȶ�ȡ������Ҫͨ��LCM���ظ��ϲ㣬����Ҫ����һ��LCMͨ�� */
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


/* ����steer�������CAN�ڣ���Ҫ���ݴ�ʱ�л�״̬�ж��Ƿ��� */
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


/* �趨�������䣬�ж�ɲ�����ֹͣ���������� */
void Thread_Brake_Function(void* param) {
	short oldValue = -1;
	while (true) {
		short diff = brake_remote - oldValue;
		/* ��ɲ��������ߵĹ�����������Ϊ-100~100���������ڴ�������ֹͣ���� */
		if (!motorComm.isTurning && (diff > 100 || diff < -100)) {
			oldValue = brake_remote;
			/* ɲ�����ߵ�Ŀ��ֵ */
			unsigned short value = (unsigned short)(brake_remote * -1 * 274.0 / 5000 + 650);
			//printf("value = %d\n", value);
			motorComm.setAim(value);
			//motorComm.isNeedtoStop = false;
		}
		Sleep(100);
	}
}

/* ���������ش�ʱ���ת����� */
void Thread_Brake_feedback_Function(void* param) {
	Encoder_Comm encoderComm;
	/* �򿪱������豸���˿�COM9��������38400 */
	encoderComm.Open(9, 38400);
	//int counter = 0;
	while (true) {
		encoderComm.OnReceive();
		if (encoderComm.ushEncoderValue != 0)
			motorComm.setCurrent(encoderComm.ushEncoderValue);
		//Sleep(2);
	}
}

/* �ж�control_mode������ģʽ����ɲ����� */
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

/* ���ݵ�ǰģʽ���������� */
int Thread_Pedal_Function(void* param) {
	/* ���Ŵ��ڣ��˿�COM10��������9600 */
	throttleComm.Open(10, 9600);
	throttleComm.InputVoltage(400);
	throttleComm.ShiftRelay(OFF);
	while (true) {
		/* �����ǰģʽ��Ϊ�ֶ���ʻ�������� */
		if (control_mode == MANUAL_MODE && throttleComm.isShiftRelayON) {
			throttleComm.InputVoltage(400);
			throttleComm.ShiftRelay(OFF);
			throttleComm.isShiftRelayON = false;
		}
		/* Զ�̼�ʻģʽ */
		else if (control_mode == REMOTE_MODE) {
			/* ��ʼ������400mv */
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

/* ��ȡ��ʱ����״̬ */
int Thread_Pedal_feedback_Function(void* param) {
	//To be continued
	//current_pedal = throttleComm.CollectingVoltage();
	return 0;
}

int main(int argc, char* argv[]) {

	/* ����LCM�������ϲ�ָ�����ݰ� */
	if (!xyz_lcm.good()) {
		cout << "steering_lcm is bad...." << endl;
		return -1;
	}
	Handler handlerObject;
	xyz_lcm.subscribe("COMMAND", &Handler::handleMessage, &handlerObject);

	/* ����CAN�����Ʒ����� */
	if (AX7_Streeing.StartDevice() == -1) {
		cout << "StartDevic failed......" << endl;
	}

	/* �������ڣ�����ɲ�����������COM8��������9600 */
	motorComm.Open(8, 9600);
	cout << "system begin" << endl;

	/* ����INS */
	CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_ins_Function,
		NULL, 0, 0);
	/* ��ײ㷽����д���� */
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_CANCar_Function,
		(LPVOID)(&AX7_Streeing), 0, 0);
	/* ���������̷������� */
	CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_steering_feedback_Function,
		(LPVOID)(&AX7_Streeing), 0, 0);

	/* ����ɲ������������� */
	/*CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_Brake_Function,
		NULL, 0, 0);*/
	
	/* ɲ������������������������� */
	/*CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_Brake_feedback_Function,
		NULL, 0, 0);*/
	/* ���ݵ�ǰģʽ������ɲ��������� */
	/*CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_Brake_calculate_Function,
		NULL, 0, 0);*/
	/* ���Ŵ��ڹ��� */
	/*CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_Pedal_Function,
		NULL, 0, 0);*/
	/* ���Ŵ��ڷ��ص�ǰֵ */
	/*CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)Thread_Pedal_feedback_Function,
		NULL, 0, 0);*/

	while (1) {
		Sleep(10000);
	}
	return 0;
}