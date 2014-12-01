/********tenkey_Neotype.ino*********
* Copyright Paul Beaudet Created Fall 2014 <inof8or@gmail.com>
* Keyer that recieves and transmits text information from its user
* This program is free open source software: See licenses.txt to understand 
  reuse rights of any file associated with this project
********* See readme.md for hardware discription **************/
#include <Wire.h>          // i2c 
#include "keyDefinitions.h" //define key numbers for various output types
//switch board named file with your board found in "hardwareOptions" folder
//eg -> yun.ino <-with-> hardwareOptions/uno.ino (if )

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
}

void loop() 
{
  byte pressState;
  if(gameMode(MONITOR_MODE)){pressState = gameLoop();}
  else{pressState = chordLoop();}//<--important part
  // captures the current state of the buttons
  if(pressState)
  {   
    if (pressState < 128)//reduces to letters
    {
      recordHandlr(pressState);//records presses to messageHandlr given active
      keyOut(pressState);      //actuate the press as a keystroke
      messageHandlr(TRIGGER);  //letters interupt messages
    } 
    else if(pressState > 159){keyOut(pressState);}//special cases like arrows
    else // if a macro pressState
    {
      macros(pressState);
      messageHandlr(MONITOR_MODE);//TODO how does messagehandlr work here?
    } // otherwise read messages as available
  }//macros are exempt from interupting messageHandlr
  //EXTRA FEATURES 
  listenForMessage();// grab potential messages over usb serial
  potentiometer(MONITOR_MODE);//monitor potentiometer for setting adjustment
  //Yun specific
  serialBowl(); // check: terminal responses
}

//-- main chord interpertation flow control
byte chordLoop()
{
  buttonUpdate();//updates internal button state
  int chord = trueChord(MONITOR_MODE);
  byte pressState = patternToChar(chord);
  
  if(pressState){feedback(chord);}
  else{feedback(MONITOR_MODE);}
  
  byte actuation = 0;
  byte hold = 0;
  if(byte doubleClick = doubleToASCII(doubleEvent(pressState)))
  {
    keyOut(BACKSPACE);
    actuation = doubleClick;
    hold = doubleClick;
  }
  else if(pressState)
  {
    actuation = pressState;
    hold = pressState;
  }
  
  byte varifiedHold = 0;
  hold = holdHandlr(hold);//check if there was a hold
  if(hold)
  {
    varifiedHold = heldASCII(hold);//feed a true hold to held ASCII
    if(varifiedHold && varifiedHold != hold){keyOut(BACKSPACE);}
  }//only feed in hold events
  else if(!buttonState(MONITOR_BUTTONS)){heldASCII(0);}//feed in release cases

  if(varifiedHold){actuation = varifiedHold;}
  return actuation;
}

byte gameLoop() //loop that serves as a gamepad layout
{
  buttonUpdate();
  int chord = trueChord(MONITOR_MODE);
  byte pressState = patternToGame(chord);
  //add feedback if desired
  byte actuation = 0;
  byte hold = 0;
  if(pressState)
  {
    actuation = pressState;
    hold = pressState;
  }
  byte varifiedHold = 0;
  hold = holdHandlr(hold);//check if there was a hold
  if(hold){varifiedHold = gameHold(hold);}//only feed in hold events
  else if(!buttonState(MONITOR_BUTTONS)){gameHold(0);}//feed in release cases
  if(varifiedHold){actuation = varifiedHold;}
  return actuation;
}

//-- feedback & state handling
void feedback(int chord)
{
  if(chord){patternVibrate(chord, 0);}
  else if(!buttonState(MONITOR_BUTTONS) && //no press -- release state
          !messageHandlr(MONITOR_MODE) &&  //no messages in the que
          !serialBowl())                   //no incomming serial interaction
  {
    patternVibrate(0, 0);
    //TODO realease all key command
  }//
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
  else if(letter == 'g' + SPACEBAR){keyOut(LEFT_GUI);} //search on many OSes
  else if(letter == 'h' + SPACEBAR){alphaHint();} // play alphabetical hint
  else if(letter == 'i' + SPACEBAR){potentiometer(ADJUST_PWM);}//Toggle to pwm
  else if(letter == 'l' + SPACEBAR){comboPress(LEFT_CTRL|LEFT_ALT,0,0);}
  else if(letter == 'p' + SPACEBAR){potentiometer(DEFAULT_MODE);}//show value
  else if(letter == 'r' + SPACEBAR){recordHandlr(TRIGGER);}//start recording
  else if(letter == 's' + SPACEBAR){potentiometer(ADJUST_TIMING);}//toggle 
  else if(letter == 't' + SPACEBAR){terminalToggle(0);}//toggle terminal shell
  else if(letter == 'z' + SPACEBAR){gameMode(TRIGGER);}//toggle gamemode
}

boolean gameMode(boolean toggle)
{
  static boolean onOff = false;
  if(toggle){onOff = !onOff;}
  return onOff;
}
