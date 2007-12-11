import serial
import time

ser = serial.Serial('/dev/ttyACM0', 19200, timeout=1)

ser.write("d10*")


while(1):
  ser.write("g*")
  print ser.read(8)

  time.sleep(1)
