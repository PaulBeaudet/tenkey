#! /usr/bin/python
# Copyright 2014 Paul Beaudet -- GPLv2, See license for reuse detials
# xdotool needs to be installed for this script to work
# sudo apt-get install xdotool <--
# Serial host testing for the Neotype keyer
import serial, time, sys, subprocess

#convertion
def convertToXdo(letter):
    if letter == chr(8):
        return '0xff08'
    if letter == chr(9):
        return '0xff09'
    if letter == chr(10): #newline char
        return '0xff0d' #carrige return
    return hex(ord(letter)) 
    #http://cgit.freedesktop.org/xorg/proto/x11proto/plain/keysymdef.h
#output function
def keyOut(letter):
    xdoLetter = convertToXdo(letter)
    subprocess.call(['xdotool','key',xdoLetter])

#Main
while True:
    try:
        ser = serial.Serial("/dev/ttyACM0", 9600)
        print "conected ACM0"
        time.sleep(1)
        break
    except Exception, e:
        print e
        sys.exc_clear()
    try: #sometimes the atmega32u4 based boards like to switch ports
        ser = serial.Serial("/dev/ttyACM1", 9600)
        print "conected ACM1"
        time.sleep(1)
        break
    except Exception, e:
        print "plug it in! ", e
        sys.exc_clear()
        time.sleep(30)
        continue
while True:
    try:
        if ser.inWaiting() > 0:
            incoming = ser.read()
            if incoming > 0:
                keyOut(incoming)
    except Exception, e:
        print "error! ", e
        continue

#this works but I'm still getting the following error, the python is also taking up a lot of process cycles doing all of this.
#device reports readiness to read but returned no data (device disconnected or multiple access on port?
