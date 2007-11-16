#!/usr/bin/python

# This file is part of the usbprogWAV project.
# (C) 2007 Benedikt Sauter

import wave
import math

wavefile = wave.open('test.wav','w')

wavefile.setnchannels(1)
wavefile.setsampwidth(1)
wavefile.setframerate(1000)



for i in range(1,100):
    wavefile.writeframes(str(1))
    wavefile.writeframes(str(255))

wavefile.close()

