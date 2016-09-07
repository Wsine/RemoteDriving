#!/usr/bin/env python
#coding=utf-8
from flask import Flask, Response, json
import random
# import lcm, car_t
import threading

app = Flask(__name__)

wheelObj = {'speed': 40, 'rpm': 2000}
wheelObj_lock = threading.Lock()

@app.route('/getWheel', methods=['GET'])
def getWheel():
	wheelObj['speed'] = round(random.uniform(0, 220), 2)
	wheelObj['rpm'] = round(random.uniform(0, 7), 2)
	# wheelObj_lock.acquire()
	resp = Response(json.dumps(wheelObj), mimetype='application/json')
	# wheelObj_lock.release()
	resp.headers['Access-Control-Allow-Origin'] = '*'
	return resp

# def lcm_handler(channel, data):
# 	print("Received message on channel %s" % channel)
# 	car_tObj = car_t.car_t.decode(data)
# 	wheelObj_lock.acquire()
# 	wheelObj['speed'] = car_tObj.wheel # TODO: fix car_tObj parameter map
# 	wheelObj['rmp'] = car_tObj.brake
# 	wheelObj_lock.release()

# def listenLCM():
# 	lc = lcm.LCM("udpm://239.255.76.67:7667?ttl=2")
# 	subscription = lc.subscribe("CAR", lcm_handler)
# 	try:
# 		while True:
# 			lc.handle()
# 	except KeyboardInterrupt:
# 		print("keyboard Interruption...")
# 	finally:
# 		lc.unsubscribe(subscription)

def main():
	# t1 = threading.Thread(target=listenLCM)
	# t1.start()
	app.run()

if __name__ == '__main__':
	main()