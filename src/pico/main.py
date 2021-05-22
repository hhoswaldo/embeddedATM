from machine import Pin
from machine import I2C
import utime

led_pin = Pin(25, Pin.OUT)
sda_pin = Pin(8)
scl_pin = Pin(9)

node_mcu_address = 0x08

i2c = I2C(
    0,
    sda=sda_pin,
    scl=scl_pin,
    freq=100000
)

led_pin.value(1)

while True:
    devices = i2c.scan()
    for device in devices:
        print('Devices found: ', len(devices))
        print("Decimal address: ", device, " | Hex address: ", hex(device))
    utime.sleep(2)
    