import lcm
import time

import car_t

lc = lcm.LCM("udpm://239.255.76.67:7667?ttl=10")

msg = car_t.car_t()
msg.wheel = "1.1"
msg.brake = "2.2"
msg.pedal = "3.3"
msg.gear = "N"

lc.publish("CAR", msg.encode())
