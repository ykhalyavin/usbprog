#!/usr/bin/python

# This file is part of the usbprogWAV project.
# (C) 2007 Benedikt Sauter

import wave
import math
import struct

wavefile = wave.open('test.wav','w')

wavefile.setnchannels(1)
wavefile.setsampwidth(1)
wavefile.setframerate(1000)
wavefile.setnframes(1000)

for i in range(1,1000):
    #data=struct.pack("h",int(math.sin(i) * 10))
    data=struct.pack("h",i % 255)
    wavefile.writeframesraw(data)


wavefile.close()

