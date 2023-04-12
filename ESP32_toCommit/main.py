#Code to combine Bluetooth + WiFi for ESP32
#from machine import Timer, Pin
from time import sleep_ms
import bluetooth

from wifi import ESP32_WiFi_init
from ble import ESP32_BLE

buffer = 0

WiFi = ESP32_WiFi_init(SSID = "BOXFISH", password = "BOX")

ble = bluetooth.BLE()
p = ESP32_BLE(ble)

#ble = ESP32_BLE("LED Controller", WiFi) #, advertising_payload)
buffer = bytearray()
def on_rx(v):
    global buffer
    buffer += v
    print(buffer, len(buffer))
    
p.on_write(on_rx)
while True:
    sleep_ms(100)
    #print("log")

