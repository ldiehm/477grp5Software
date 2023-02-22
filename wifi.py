# Complete project details at https://RandomNerdTutorials.com

try:
  import usocket as socket
except:
  import socket

import network
import time

import esp
esp.osdebug(None)

import gc
gc.collect()

ssid = 'BOXFISH'
password = 'BOX'

ap = network.WLAN(network.AP_IF)
ap.active(True)
ap.config(essid=ssid, password=password)

while ap.active() == False:
    pass

print('Connection successful')
print(ap.ifconfig())

def web_page():
    html = "(255, 200, 255)"
    return html


s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) #set up UDP socket

s.bind(('192.168.4.1', 80))
#s.listen(5)
print("Hello")
#conn, addr = s.accept()
while True:
    #print('Got a connection from %s' % str(addr))
    print("ran")
    time.sleep(1)
    s.sendto(bytes("testing some ascii", "utf-8"), ("192.168.4.2", 80))
    #request = conn.recv(1024)
    #print('Content = %s' % str(request))
    #response = web_page()
    #conn.send(response)
#conn.close()

