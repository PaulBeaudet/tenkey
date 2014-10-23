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
**********************************/
#include<avr/pgmspace.h>//explicitly stated read only memory

void setup()//setup the needed hardware  
{
  pagersUp();          // hardware.ino: brings vibrating motor interface up
  buttonUp();          // hardware.ino: brings button polling intreface up
  serialInterfaceUp(); // hardware.ino: brings serial output interface/s up
  delay(6600);         // wait for the yun to stop interfering....
}

/*** ^End setup^ ***** GLOBAL DEFINITIONS  *****************/
#define MONITOR_MODE   0
#define START_INTERUPT 1 // removes output, zeros play point: messageHandlr
#define TRIGGER        1 // set enter key to press : enterBehavior()
#define CAT_OUT        2 // set message to play : messageHandlr
#define RECORD         2 // enterBehavior() record command
#define JOB            3 // messageHandlr "is a job set?" argument
#define CHECK_VALUE    1 // potentiometer()
#define ADJUST_PWM     2 // potentiometer()
#define ADJUST_TIMING  3 // potentiometer()
#define BUFFER_SIZE    80   //cause why should a line be longer?
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

/**** Main Loop ***********************************************/

void loop() 
{
   byte messageMode = MONITOR_MODE; //default to Monitor
   byte pressState = chordLoop(buttonSample());//<--important part
   // captures the current state of the buttons
   if(pressState)
   {
     outputFilter(pressState);//handles output modes
     recordHandlr(pressState);//records presses to messageHandlr given active
     if (pressState < 128 && !recordHandlr(MONITOR_MODE))
     {// any letter      and  no recording 
       messageMode = START_INTERUPT;
     }//exclude macros
   }
   //EXTRA FEATURES 
   listenForMessage();// grab potential messages over serial
   potentiometer(MONITOR_MODE);//monitor potentiometer for setting adjustment
   messageHandlr(messageMode);//async message mangment-interupt with keystroke
}

/********** Main functions *************
*************************************/
byte chordLoop(int input) // takes sample of buttons: returns true for press
{// main progam loop is abstracted here, so it can be switch with other test
  byte actionableSample= patternToChar(input); //determine chord validity
  if(actionableSample){patternVibrate(input);} //actuate pagers:if letters
  else if(!messageHandlr(JOB)){patternVibrate(0);}//release:turn pagers off
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
Enter key mode is set to a one time transfer function that moves temp into the message hander   
*******************************************************/

boolean recordHandlr(byte mode)
{
  static boolean active = 0;
  static byte recordLength = 0;
  
  switch(mode)
  {
    case MONITOR_MODE: return active; 
    case TRIGGER: 
      active = 1;
      enterBehavior(RECORD); 
      break;
    case CAT_OUT: 
      active = 0;
      removeThisMany(recordLength);
      recordLength = 0;
      messageHandlr(NEW_LINE); 
      messageHandlr(CAT_OUT);
      break;
    default: //letter is feed into the record
      if(active)
      {
        messageHandlr(mode);
        recordLength++;
      } 
  }return 0;
}

void enterBehavior(byte mode) // this function handles enter states
{ 
  static byte triggerType = 0;
  switch(mode)
  {
    case TRIGGER:
    switch(triggerType)
    {
      case 0: keyOut(CARIAGE_RETURN); break;
      case RECORD: 
        triggerType = 0; // set back to normal behavior
        recordHandlr(CAT_OUT); // finish the recording
        break; // record
      case 3: break;// command
    }
    break;
    case RECORD: triggerType = RECORD; break;// set record mode
    case 3: triggerType = 3; break;// set command mode 
  }
}

byte warningMessage[] = "recording";

void outputFilter(byte letter)
{// long holds shift bytes up; the following switch covers special options
  switch(letter)//takes in key letter
  { // execute special compand basd on long hold
    case CARIAGE_RETURN: enterBehavior(TRIGGER); break;
    case 129:break;                          //'a'
    case 130: 
      if (recordHandlr(MONITOR_MODE)){break;}    // collision prevention
      messageHandlr(CAT_OUT); break;         //'b' print buffer
    case 131:break;                          //'c' copy; cache message
    case 132:break;                          //'d'
    case 133:break;                          //'e' Enter; confirm
    case 134:break;                          //'f'
    case 135:break;                          //'g' game
    case 136: alphaHint(); break;            //'h' hatically displays alphabet
    case 137:potentiometer(ADJUST_PWM);break;//'i' pwm intensity
    case 138:break;	                         //'j'
    case 139:break;	                         //'k'
    case 140:break;	                         //'l'
    case 141:break;                          //'m' Message; cat cache
    case 142:break;                          //'n' nyan
    case 143:break;                          //'o'
    case 144:potentiometer(CHECK_VALUE);break;//'p'
    case 145:break;                          //'q'
    case 146:
      fastToast(warningMessage); 
      recordHandlr(TRIGGER); break;          //'r'
    case 147: potentiometer(ADJUST_TIMING);break; //'s' haptic display speed
    case 148:break;                          //'t' Transmit send cache
    case 149:break;                          //'u'
    case 150:break;                          //'v' varify 
    case 151:break;                          //'w'
    case 152:break;                          //'x'
    case 153:break;                          //'y'
    case 154:break;                          //'z'
    case 155:break;                          //'openbracket'
    case 156:break;                          //'pipe'
    case 157:break;	                         //'closebrack'
    case 158:break;	                         //'tilde'
    default: keyOut(letter);//send ascii given no exception
  }
}
