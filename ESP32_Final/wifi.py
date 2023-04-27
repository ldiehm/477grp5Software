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
class ESP32_WiFi_init():
    def __init__(self, SSID, password):
        
        self.ap = network.WLAN(network.AP_IF)
        self.ap.active(True)
        self.ap.config(essid = SSID, password = password)
        
        while self.ap.active() == False:
            pass
        print('Connection successful')
        print(self.ap.ifconfig())
        
        self.s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) #set up UDP socket
        self.s.bind(('192.168.4.1', 80))
        

