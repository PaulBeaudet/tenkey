#! /usr/bin/python
# Copyright 2014 Paul Beaudet -- GPLv2, See license for reuse detials
# xdotool needs to be installed for this script to work
# sudo apt-get install xdotool <--
# Serial host testing for the Neotype keyer
import serial, time, subprocess

# The second argument is the baudrate,
# change according to the baudrate you gave to your Serial.begin command
port0 = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
time.sleep(1)

#output function
def keyOut(letter):
    if letter == chr(8): #BACKSPACE
        subprocess.call(['xdotool','key','BackSpace'])#not working
    if letter == ' ': #SPACEBAR
        subprocess.call(['xdotool','key','space'])
    if letter > ' ':
        subprocess.call(['xdotool','key',letter])

#Main loop
while True:
    try:
        incoming = port0.read()
        if incoming > 0:    
            keyOut(incoming)
    except Exception, e:
        print "error! ", e
        continue

#this works but I'm still getting the following error, the python is also taking up a lot of process cycles doing all of this.
#device reports readiness to read but returned no data (device disconnected or multiple access on port? 
