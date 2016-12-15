sudo route add default gw 192.168.1.252 metric 600 wlan1
sudo route add default gw 10.186.138.57 metric 700 wwan0
sudo route add -host 10.186.138.51 gw 10.186.138.57 wwan0
sudo route add -host 10.186.138.54 gw 10.186.138.57 wwan0 
sudo route add -net 10.186.138.56 netmask 255.255.255.248 metric 700 wwan0
sudo route add -net 169.254.0.0/16 metric 1000 wwan0
sudo route add -net 192.168.1.0/24 metric 600 wlan1
sudo route add -host 239.255.76.68 wlan1
