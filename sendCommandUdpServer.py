
#!/usr/bin/env python
#coding=utf-8
import lcm
import time
import car_t
import socket, traceback

host = "192.168.191.4"
port = 8000

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((host, port))
angle_previous = 0
time_previous = 0
angle_speed = 0
heart_counter = 0
heart_packet=""

while 1:
	try:
		message, address = s.recvfrom(1024)
		steering_angle_str = message.split('&')[0]
		pedal_str = message.split('&')[1]
		brake_str = message.split('&')[2]
		gear_str = message.split('&')[3]
		id_str = message.split('&')[4]
		time_str = message.split('&')[5]

		steering_angle = int(steering_angle_str)
		pedal = float(pedal_str)
		brake = float(brake_str)
		get_id = int(id_str)
		get_time = int(time_str)
		heart_packet = heart_packet + '&' + id_str
		heart_counter = heart_counter + 1
		if (heart_counter == 10):
			heart_packet = heart_packet + time.strftime("%Y-%m-%d-%T")
			heart_packet = heart_packet + '\n'
			remote_host = '192.168.191.1'
			remote_port = 8000
			remote_bufsize = 256
			remote_addr = (remote_host, remote_port)
			heartPacket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
			heartPacket.sendto(heart_packet, remote_addr)
			print "success sent"
			heartPacket.close()
			heart_counter = 0
			heart_packet = ""

		if (time_previous != 0):
			if (get_time > time_previous):
				angle_sub = steering_angle-angle_previous
				if (angle_sub < 0):
					angle_sub = -angle_sub
				time_sub = (get_time-time_previous)*0.001
				angle_speed = int(angle_sub / time_sub)
		lc = lcm.LCM("udpm://239.255.76.68:7667?ttl=2&recv_buf_size=1638400")
		msg = car_t.car_t()
		msg.steering_angle = steering_angle_str
		msg.pedal = pedal_str
		msg.brake = brake_str
		msg.gear = gear_str		
		msg.steering_angle_speed = str(angle_speed)
		lc.publish("COMMAND", msg.encode()) 
		print 'No. ', get_id
		print 'angle: ', steering_angle, 'speed: ', angle_speed, 'pedal: ', pedal_str, 'brake: ', brake_str, 'gear: ', gear_str
		time_previous = get_time
		angle_previous = steering_angle
		#time.sleep(0.001)		
	except (KeyboardInterrupt, SystemExit):
		raise
	except:
		traceback.print_exc()