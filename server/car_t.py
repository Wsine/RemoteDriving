"""LCM type definitions
This file automatically generated by lcm.
DO NOT MODIFY BY HAND!!!!
"""

try:
    import cStringIO.StringIO as BytesIO
except ImportError:
    from io import BytesIO
import struct

class car_t(object):
    __slots__ = ["steering_angle", "pedal", "brake", "gear", "steering_angle_speed"]

    def __init__(self):
        self.steering_angle = ""
        self.pedal = ""
        self.brake = ""
        self.gear = ""
        self.steering_angle_speed = ""

    def encode(self):
        buf = BytesIO()
        buf.write(car_t._get_packed_fingerprint())
        self._encode_one(buf)
        return buf.getvalue()

    def _encode_one(self, buf):
        __steering_angle_encoded = self.steering_angle.encode('utf-8')
        buf.write(struct.pack('>I', len(__steering_angle_encoded)+1))
        buf.write(__steering_angle_encoded)
        buf.write(b"\0")
        __pedal_encoded = self.pedal.encode('utf-8')
        buf.write(struct.pack('>I', len(__pedal_encoded)+1))
        buf.write(__pedal_encoded)
        buf.write(b"\0")
        __brake_encoded = self.brake.encode('utf-8')
        buf.write(struct.pack('>I', len(__brake_encoded)+1))
        buf.write(__brake_encoded)
        buf.write(b"\0")
        __gear_encoded = self.gear.encode('utf-8')
        buf.write(struct.pack('>I', len(__gear_encoded)+1))
        buf.write(__gear_encoded)
        buf.write(b"\0")
        __steering_angle_speed_encoded = self.steering_angle_speed.encode('utf-8')
        buf.write(struct.pack('>I', len(__steering_angle_speed_encoded)+1))
        buf.write(__steering_angle_speed_encoded)
        buf.write(b"\0")

    def decode(data):
        if hasattr(data, 'read'):
            buf = data
        else:
            buf = BytesIO(data)
        if buf.read(8) != car_t._get_packed_fingerprint():
            raise ValueError("Decode error")
        return car_t._decode_one(buf)
    decode = staticmethod(decode)

    def _decode_one(buf):
        self = car_t()
        __steering_angle_len = struct.unpack('>I', buf.read(4))[0]
        self.steering_angle = buf.read(__steering_angle_len)[:-1].decode('utf-8', 'replace')
        __pedal_len = struct.unpack('>I', buf.read(4))[0]
        self.pedal = buf.read(__pedal_len)[:-1].decode('utf-8', 'replace')
        __brake_len = struct.unpack('>I', buf.read(4))[0]
        self.brake = buf.read(__brake_len)[:-1].decode('utf-8', 'replace')
        __gear_len = struct.unpack('>I', buf.read(4))[0]
        self.gear = buf.read(__gear_len)[:-1].decode('utf-8', 'replace')
        __steering_angle_speed_len = struct.unpack('>I', buf.read(4))[0]
        self.steering_angle_speed = buf.read(__steering_angle_speed_len)[:-1].decode('utf-8', 'replace')
        return self
    _decode_one = staticmethod(_decode_one)

    _hash = None
    def _get_hash_recursive(parents):
        if car_t in parents: return 0
        tmphash = (0xe8cb35a93c65c4e6) & 0xffffffffffffffff
        tmphash  = (((tmphash<<1)&0xffffffffffffffff)  + (tmphash>>63)) & 0xffffffffffffffff
        return tmphash
    _get_hash_recursive = staticmethod(_get_hash_recursive)
    _packed_fingerprint = None

    def _get_packed_fingerprint():
        if car_t._packed_fingerprint is None:
            car_t._packed_fingerprint = struct.pack(">Q", car_t._get_hash_recursive([]))
        return car_t._packed_fingerprint
    _get_packed_fingerprint = staticmethod(_get_packed_fingerprint)
