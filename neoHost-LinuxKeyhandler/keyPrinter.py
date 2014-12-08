#! /usr/bin/python

#    Serial Reader for ARDUINO
#    usefull when tail -f /dev/ttyXXXX doesn't work
#    Change ttyACM0 for your own tty interface
import serial, time, sys, subprocess

# The second argument is the baudrate,
# change according to the baudrate you gave to your Serial.begin command
port0 = serial.Serial("/dev/ttyACM0", 9600)

# To avoid communication failure due to bad timings
port0.setDTR(True)
time.sleep(1)
port0.setDTR(False)

# give and option to select serialport

#port0.write("hey")#say hello to the arduino
while port0.isOpen:
	letter = port0.read()
	if letter > 31 and letter < 127:
		ps = subprocess.Popen('xdotool','key',letter)
    #if raw_input() == 'q': #given quit argument exit program
        #sys.exit()
# if loop has broken this means port closed
print "closed" #cry foul
