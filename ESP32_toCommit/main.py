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
_BUFF_SIZE = 3*64
#ble = ESP32_BLE("LED Controller", WiFi) #, advertising_payload)
buf1 = bytearray(_BUFF_SIZE)
buf2 = bytearray(_BUFF_SIZE)
count = 0
def on_rx(v, wifi):
    #TODO add count that resets the byte array each frame
    global buf1
    global buf2
    global count
    if(count < _BUFF_SIZE):
        for i in range(16):
#                 print(type((count).to_bytes(1, "little")))
#                 print(type(buf1[count + i]))
            buf1[count + i] = count #(count).to_bytes(1, "little")
#             else:
#                 fakeVariable = 0
#                 buf1[count + i] = 0 #(fakeVariable).to_bytes(1, "little")             
#             buf1[count+i] = v[i]
    else:
        for i in range(16):
            buf2[count-192+i] = v[i]
        
    count += 16
    if(count >= 2 * _BUFF_SIZE):
        #print("SENDING: ", len(buf1))
#         print(len(buf2))
#         print(buf1)
#         bq =
        #bq = [0] * 192
        #for i in range(1,192,3):
        #    bq[i] = i
        #bq = bytearray(bq)
        print(buf1)
        wifi.s.sendto(buf1, ("192.168.4.2", 80))
        #wifi.s.sendto(bytes((str(buf2)), "utf-8"), ("192.168.4.3", 80))
        
        count = 0
        
p.on_write(on_rx)
#while True:
  #  sleep_ms(100)
    #print("log")

