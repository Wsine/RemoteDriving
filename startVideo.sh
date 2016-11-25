raspivid -t 0 -h 720 -w 1080 -fps 25 -qp 28 -pf high -awb auto -vf -n -hf -b 0 -o -| socat - udp-datagram:192.168.191.1:5000
