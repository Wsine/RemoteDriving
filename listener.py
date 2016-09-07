import lcm

import car_t

def my_handler(channel, data):
    msg = car_t.car_t.decode(data)
    print("Received message on channel \"%s\"" % channel)
    print("   wheel   = %s" % str(msg.wheel))
    print("   pedal    = %s" % str(msg.pedal))
    print("   brake = %s" % str(msg.brake))
    print("   gear     = %s" % str(msg.gear))
    print("")

lc = lcm.LCM("udpm://239.255.76.67:7667?ttl=10")
subscription = lc.subscribe("CAR", my_handler)

try:
    while True:
        lc.handle()

except KeyboardInterrupt:
    pass

lc.unsubscribe(subscription)
