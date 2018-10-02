#!/usr/bin/env python
#coding=utf-8
import lcm
import time
import car_t
import web


urls = (
    '/', 'index'
)
app = web.application(urls, globals())

class index:
    def GET(self):
    	print 'get request'
    def POST(self):
    	i = web.input()
    	wheel = i.get('wheel')
        pedal = i.get('pedal')
        brake = i.get('brake')
        gear = i.get('Gear')
    	print 'Received: ', 'wheel: ', wheel, 'pedal: ',pedal, 'brake: ', brake, 'gear:', gear
	lc = lcm.LCM("udpm://239.255.76.67:7667?ttl=5")
	#lc = lcm.LCM()
	msg = car_t.car_t()
	msg.wheel = wheel
	msg.pedal = pedal
	msg.brake = brake
	msg.gear = gear
	lc.publish("CAR", msg.encode())        
	
	return 'command successful'
if __name__ == "__main__":
    app.run()
