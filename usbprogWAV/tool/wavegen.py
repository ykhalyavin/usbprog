#!/usr/bin/python

# This file is part of the usbprogWAV project.
# (C) 2007 Benedikt Sauter

import wave
import math
import struct

wavefile = wave.open('test.wav','w')

wavefile.setnchannels(1)
wavefile.setsampwidth(1)
wavefile.setframerate(44000)

for i in range(1,44000):
    #data=struct.pack("h",int(math.sqrt(math.sin(i)*math.sin(i))*100))
    #data=struct.pack("B",int((math.sin(i)+1)*110))
    data=struct.pack("b",i % 120)
    wavefile.writeframesraw(data)


wavefile.close()

