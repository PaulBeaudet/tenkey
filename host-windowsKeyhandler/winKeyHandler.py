# Copyright 2014 Bram Rausch -- GPLv2, See license for reuse detials
# you need to have several libraries installed
import serial
import time
from win32 import win32api
ser = serial.Serial("COM3", 9600) #open port com3 with a baudrare of 9600
winKeyCodes = { #all windows key codes
    "a": 0x41,
    "b": 0x42,
    "c": 0x43,
    "d": 0x44,
    "e": 0x45,
    "f": 0x46,
    "g": 0x47,
    "h": 0x48,
    "i": 0x49,
    "j": 0x4A,
    "k": 0x4B,
    "l": 0x4C,
    "m": 0x4D,
    "n": 0x4E,
    "o": 0x4F,
    "p": 0x50,
    "q": 0x51,
    "r": 0x52,
    "s": 0x53,
    "t": 0x54,
    "u": 0x55,
    "v": 0x56,
    "w": 0x57,
    "x": 0x58,
    "y": 0x59,
    "z": 0x5A
}
while True:
    letter = ser.read() #read the serial input from the configured port
    try: #try to convert var letter to ASCII
        letter.decode("ASCII")
    except UnicodeError: #if there is a error pass
        pass
    else:
        letter = letter.decode("ASCII") 
        #if converting the input letter does work convert it
        letter.replace("\r", "")
        if letter != " " and letter != '\x08' and letter != '\x00':
            win32api.keybd_event(winKeyCodes[letter.lower()], 0,0,0)
