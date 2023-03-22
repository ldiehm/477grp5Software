#Code to combine Bluetooth + WiFi for ESP32
#from machine import Timer, Pin
from time import sleep_ms
#import ubluetooth


from wifi import ESP32_WiFi_init
from ble import ESP32_BLE

buffer = 0

WiFi = ESP32_WiFi_init(SSID = "BOXFISH", password = "BOX")

ble = ESP32_BLE("LED Controller", WiFi)

