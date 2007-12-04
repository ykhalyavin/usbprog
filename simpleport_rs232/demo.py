import serial
import time

ser = serial.Serial('/dev/ttyACM0', 19200, timeout=1)

#ser.write("*dB1*")
ser.write("*")
ser.write("d10*")
print ser.read(2)

while(1):
  #ser.write("p10*")
  #print ser.read(2)
  #time.sleep(1)
  #ser.write("p11*")
  ser.write("i1*")
  print ser.read(2)
  time.sleep(1)
