/********tenkey_Neotype.ino*********
* Copyright Paul Beaudet Created Fall 2014 <inof8or@gmail.com>
* Keyer that recieves and transmits text information from its user
* This program is free open source software: See licenses.txt to understand 
  reuse rights of any file associated with this project
********* See readme.md for hardware discription **************/
#include <Wire.h>          // i2c
#include <EEPROM.h>
#include "keyDefinitions.h" //define key numbers for various output types
//switch board named file with your board found in "hardwareOptions" folder
//eg -> yun.ino <-with-> hardwareOptions/uno.ino

#define MONITOR_BUTTONS 33 // signal to monitor buttons
#define MONITOR_MODE   0 // goto default behavior for multi-mode functions
#define TRIGGER        1 // set enter key to press : enterBehavior()
#define DEFAULT_MODE   1 // outputFilter: regular letters, potentiometer check
#define NUMBERS_MODE   2 // outputFilter: Numbers
#define RECORD_CAT     2 // enterBehavior() cat or record or record cat
#define ADJUST_PWM     2 // potentiometer()
#define ADJUST_TIMING  3 // potentiometer()
#define LINE_SIZE      80   //cause why should a line be longer?

void setup()//setup the needed hardware  
{
  pagersUp();          // hardware.ino: brings vibrating motor interface up
  buttonUp();          // hardware.ino: brings button polling intreface up
  serialInterfaceUp(); // hardware.ino: brings serial output interface/s up
  EEPROMsetup();
}

void loop() 
{
  mainIOroutine(); //handles key input and output
  //EXTRA FEATURES
  feedbackAndRelease();//controls pager feedback and release control
  messageHandlr(MONITOR_MODE);//handles incoming messages
  listenForMessage();// grab potential messages over usb serial
  potentiometer(MONITOR_MODE);//monitor potentiometer for setting adjustment
  mouseMovement();
  //Yun specific
  serialBowl(MONITOR_MODE); // check: terminal responses
}
//-- main IO flow control
byte mainIOroutine()
{
  byte pressState = chordLoop();//<--important part
  // captures the current state of the buttons
  if(pressState)
  {   
    if (pressState < 128)//reduces to letters
    {
      recordHandlr(pressState);//records presses to messageHandlr given active
      keyOut(pressState);      //actuate the press as a keystroke
      messageHandlr(TRIGGER);  //letters interupt messages
    } 
    else if(pressState > 159){mouseClick(pressState);}//special cases
    else{macros(pressState);} //in all other cases check for macros
  }//macros are exempt from interupting messageHandlr
  return pressState;
}

//-- main chord interpertation flow control
byte chordLoop()
{
  static byte doubleActuation;

  byte actuation = 0;
  if(doubleActuation)
  {
    actuation = doubleActuation;
    doubleActuation = 0;
    return actuation;
  }
  
  buttonUpdate();//updates internal button state
  int chord = trueChord(MONITOR_MODE);
  byte pressState = patternToChar(chord);
  byte hold = 0;
  if(doubleActuation = doubleToASCII(doubleEvent(pressState)))
  {
    actuation = BACKSPACE;
    hold = doubleActuation;
  }
  else if(pressState)
  {
    actuation = pressState;
    hold = pressState;
  }
  
  byte varifiedHold = 0;
  hold = holdHandlr(hold);//check if there was a hold
  if(hold){varifiedHold = heldASCII(hold);}//only feed in hold events
  else if(!buttonState(MONITOR_BUTTONS)){heldASCII(0);}//feed in release cases

  if(varifiedHold){actuation = varifiedHold;}
  return actuation;
}

//-- feedback & state handling
void feedbackAndRelease()
{
  static boolean held = false;

  int currentState = buttonState(MONITOR_BUTTONS);
  if( held && currentState == 0 ) //at the moment of a release
  {
    if(vibInactive()){patternVibrate(0);}
    releaseKey();
    mouseRelease(); //he wants to be free!
    held = false;
  }
  else if(patternToChar(currentState))
  { // if that state is a pattern 
    patternVibrate(currentState);
    held = true;
  }
}

boolean vibInactive() //check other function controling vibrators
{ //extended AND opperation for readability
  if(serialBowl(MONITOR_MODE)){return false;} //if terminal mode is printing
  if(messageHandlr(MONITOR_MODE)){return false;} //if message mode is printing
  return true; // as long as everything is inactive return true
}

//-- Special macro functions 
void macros(byte letter)
{
  if     (letter == 'a' + SPACEBAR){convertionMode(TRIGGER);}//toggle numbers
  else if(letter == 'b' + SPACEBAR) // play message buffer
  {
    if(recordHandlr(MONITOR_MODE)){;}
    else{messageHandlr(RECORD_CAT);}
  }
  else if(letter == 'h' + SPACEBAR){alphaHint();} // play alphabetical hint
  else if(letter == 'i' + SPACEBAR){potentiometer(ADJUST_PWM);}//Toggle to pwm
  else if(letter == 'j' + SPACEBAR){comboPress(LEFT_ALT,0,0);}
  else if(letter == 'l' + SPACEBAR){comboPress(LEFT_CTRL|LEFT_ALT,0,0);}
  else if(letter == 'p' + SPACEBAR){potentiometer(DEFAULT_MODE);}//show value
  else if(letter == 'r' + SPACEBAR){recordHandlr(TRIGGER);}//start recording
  else if(letter == 's' + SPACEBAR){potentiometer(ADJUST_TIMING);}//toggle 
  else if(letter == 't' + SPACEBAR){serialBowl(TRIGGER);}//toggle terminal
}
