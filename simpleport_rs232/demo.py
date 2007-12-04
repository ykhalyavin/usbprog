import serial
import time

ser = serial.Serial('/dev/ttyACM0', 19200, timeout=1)

ser.write("*")
ser.write("d10*")


while(1):
  #ser.write("i1*")
  #print ser.read(2)
  #print ser.read(2)
  ser.write("g*")
  print ser.read(8)

  time.sleep(1)
