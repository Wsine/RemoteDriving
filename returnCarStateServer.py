#!/usr/bin/env python
#coding=utf-8
import lcm
import time
import car_t
import web
from multiprocessing import Pool


urls = (
    '/', 'index'
)
app = web.application(urls, globals())
global result

class index:
    def GET(self):
    	print 'get request'
    def POST(self):
        return result

def my_handler(channel, data):
    msg = car_t.car_t.decode(data)
    result = "wheel=" + msg.wheel
    print("Received message on channel \"%s\"" % channel)
    print("   wheel   = %s" % str(msg.wheel))
    #print("   pedal    = %s" % str(msg.pedal))
    #print("   brake = %s" % str(msg.brake))
    #print("   gear     = %s" % str(msg.gear))
    print("")

def listenLCM():
    lc = lcm.LCM("udpm://239.255.76.67:7667?ttl=2")
    subscription = lc.subscribe("CAR", my_handler)
    try:
        while True:
            lc.handle()
    except KeyboardInterrupt:
        pass
    lc.unsubscribe(subscription)
    	
if __name__ == "__main__":
    p = Pool(1)
    p.apply_async(listenLCM)
    p.close()
    app.run()
    p.join()
