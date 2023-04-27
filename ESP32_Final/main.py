#Code to combine Bluetooth + WiFi for ESP32
#from machine import Timer, Pin
from time import sleep_ms
import bluetooth

from wifi import ESP32_WiFi_init
from ble import ESP32_BLE

buffer = 0

WiFi = ESP32_WiFi_init(SSID = "BOXFISH", password = "BOX")

ble = bluetooth.BLE()
p = ESP32_BLE(ble=ble, wifi=WiFi)
_BUFF_SIZE = 3*64 + 1
#ble = ESP32_BLE("LED Controller", WiFi) #, advertising_payload)
buf1 = bytearray(_BUFF_SIZE)
buf2 = bytearray(_BUFF_SIZE)



buf1[_BUFF_SIZE - 1] = bytes('K', 'utf-8')[0]
buf2[_BUFF_SIZE - 1] = bytes('K', 'utf-8')[0]

count1 = 0
count2 = 0
def on_rx(v, wifi):
    #TODO add count that resets the byte array each frame
    global buf1
    global buf2
    global count1
    global count2
    if(count1 < _BUFF_SIZE - 1):
        for i in range(16):
            #if not (count1 + i) % 3: 
            #    buf1[count1 + i] = int((count1 + i) / 2) #(count).to_bytes(1, "little")
            buf1[count1+i] = int(v[i] / 32)
        count1 += 16
    
    else:
        for i in range(16):
            buf2[count2+i] = int(v[i] / 32)
        
        count2 += 16
        
    if(count2 >= _BUFF_SIZE - 1):
        
        print(buf1)
        print(buf2)

        wifi.s.sendto(buf1, ("192.168.4.3", 80))
        wifi.s.sendto(buf2, ("192.168.4.4", 80))
        
        count1 = 0
        count2 = 0

        
p.on_write(on_rx)
#while True:
  #  sleep_ms(100)
    #print("log")

