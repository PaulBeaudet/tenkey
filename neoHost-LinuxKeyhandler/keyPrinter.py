#! /usr/bin/python

#    Serial Reader for ARDUINO
#    usefull when tail -f /dev/ttyXXXX doesn't work
#    Change ttyACM0 for your own tty interface
import serial, time, subprocess

# The second argument is the baudrate,
# change according to the baudrate you gave to your Serial.begin command
port0 = serial.Serial("/dev/ttyACM0", 9600)

# To avoid communication failure due to bad timings
port0.setDTR(True)
time.sleep(1)
port0.setDTR(False)

#output function
def keyOut(letter):
    if letter >= ' ':
        ps = subprocess.call(['xdotool','key',letter])

#Main loop
while True:
    if port0.inWaiting() > 0:
        try:
            keyOut(port0.read())
        except Exception, e:
            print "marbles lost! ", e
            continue

#still getting an error, the python is also taking up a lot of process cycles doing all of this. 
#device reports readiness to read but returned no data (device disconnected or multiple access on port?)
