/********tenkey_Neotype.ino*********
* Copyright Paul Beaudet Created Fall 2014 <inof8or@gmail.com>
* Keyer that recieves and transmits text information from its user
* This program is free open source software: See licenses.txt to understand 
  reuse rights of any file associated with this project
-------------------------------------------------------------------
* Hardware discription: Arduino(atmega32u4) or uno(w/bluefruit EZKey)
* Adafruit_PWMServoDriver shield drives 8 pager moters
* ten keys take a input, 8 of which register chords
* 8 Pagers sit on top of the chord keys to be able to feed back letters
* depends on an adafruit I2C multiplex driver
More detailed discription: arduinoMicropins.txt 
below is platform choice, comment in if using, out if not using
**************** COMMENT IN ARDUINO BOARD HERE ************************/
//#define LEO // Arduino Micro or Leonardo USB HID or Bluefruit
#define YUN //Bluefruit is only compatible with yun via software serial 
#define YUN_BOOT_OUTPUT true // mark true to see yun boot msgs on serial
//#define UNO   // Arduinos using the 328p + bluefruit EZ-key HID

#define MONITOR_MODE   0
#define START_INTERUPT 1 // removes output, zeros play point: messageHandlr
#define TRIGGER        1 // set enter key to press : enterBehavior()
#define DEFAULT_MODE   1 // outputFilter: regular letters
#define CAT_OUT        2 // set message to play : messageHandlr
#define NUMBERS_MODE   2 // outputFilter: Numbers
#define RECORD         2 // enterBehavior() record command
#define MOVEMENT_MODE  3 // outputFilter() Movement
#define JOB            4 // messageHandlr "is a job set?" argument
#define CHECK_VALUE    1 // potentiometer()
#define ADJUST_PWM     2 // potentiometer()
#define ADJUST_TIMING  3 // potentiometer()
#define LINE_SIZE      80   //cause why should a line be longer?
#define TERMINATION    '\0'
#define NEW_LINE       '\n' //determines end of a message in buffer
#define BACKSPACE      8    // output keys
#define TAB_KEY        9
#define SPACEBAR       32
#define CARIAGE_RETURN 13
#define LETTER_A       97
#define LETTER_Z       122
#define L_THUMB        256  // input data
#define R_THUMB        512
#define XON            17 // control_Q resume terminal output
#define XOFF           19 // control_S stop terminal output

#include<avr/pgmspace.h>//explicitly stated read only memory

void setup()//setup the needed hardware  
{
  pagersUp();          // hardware.ino: brings vibrating motor interface up
  buttonUp();          // hardware.ino: brings button polling intreface up
  serialInterfaceUp(); // hardware.ino: brings serial output interface/s up
}

/**** Main Loop ***********************************************/
void loop() 
{
   byte messageMode = MONITOR_MODE; //default to Monitor
   byte pressState = chordLoop(buttonSample());//<--important part
   // captures the current state of the buttons
   if(pressState)
   {
     recordHandlr(pressState);//records presses to messageHandlr given active
     //note: needs to be before outputFilter, for the sake of new lines
     outputFilter(pressState);//handles output modes
     if (pressState < 128 && !recordHandlr(MONITOR_MODE))
     {// any macro      and  no recording 
       messageMode = START_INTERUPT;
     }//prevents macros from interupting messageHandlr
   }
   //EXTRA FEATURES 
   listenForMessage();// grab potential messages over usb serial
   potentiometer(MONITOR_MODE);//monitor potentiometer for setting adjustment
   messageHandlr(messageMode);//async message mangment-interupt with keystroke
   //Yun specific
   serialBowl(); // check: terminal responses
}

//********** Main functions *************
byte chordLoop(int input) // takes sample of buttons: returns true for press
{// main progam loop is abstracted here, so it can be switch with other test
  byte actionableSample= patternToChar(input); //determine chord validity
  if(actionableSample){patternVibrate(input);} //actuate pagers:if letters
  else if(!messageHandlr(JOB) && !serialBowl()){patternVibrate(0);}
  //      except for special printing cases     release:turn pagers off
  return inputFilter(actionableSample);
}//            debounce -> check hold -> return ASCII:letter or action code

/************ Output flow ********************
Actuation states are brought into main loop 
(aka a is pressed or a special is pressed)
The ASCII byte is passed to a mode function 
certain nonprinting byte values can activate various "modes"
Examples of modes could be the following
-- Shell mode: Enter behavior inserts tempBuffer into a linux shell
    and returns output to haptic display
-- Numbers mode: Outputs numbers instead of letters 
-- Record  mode: Record letters 
-- listen  mode: Waits for messages to come over serial
-- messaging mode: send messages to contacts
^ after outlining many off these can find intergation in one another
^ particularly if output is held temporarily 
    a flag could be placed for "record"
    in that mode enter can signify transfering temp to messageHandlr
-Might make sence for the enter key to have various modes
Catching the enter case into a mode switch
-numbers mode could use a seperate inputFilter 
or just shift the current layout which is likely more pragmatic
-Record mode might be practical to be happening all the time in another
buffer for the sake of type correction. In this way the record macro
will mark a place in the tempBuffer to go back to
Enter key mode is set to a one time transfer function 
that moves temp into the message hander   
*******************************************************/
