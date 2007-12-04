import serial
import time

ser = serial.Serial('/dev/ttyACM0', 19200, timeout=1)

ser.write("*")
ser.write("*dB1*")

while(1):
  ser.write("pB0*")
  time.sleep(1)
  ser.write("pB1*")
  time.sleep(1)
