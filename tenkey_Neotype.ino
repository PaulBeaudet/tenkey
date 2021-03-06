/********tenkey_Neotype.ino*********
* NOTE This is the main .ino sketch project
  |--> parent folder name should be "tenkey_Neotype" to recognize as such
* Compiled with Arduino.cc 1.6 IDE
* Copyright Paul Beaudet originally created fall 2014 <inof8or@gmail.com>
* Keyer that recieves and transmits text information from its user
* This program is free open source software: See licenses.txt to understand
  reuse rights of any file associated with this project
********* See readme.md for hardware discription **************/
#include <Wire.h>           // i2c
#include <EEPROM.h>         // store mouse calibration
#include "keyDefinitions.h" // define key numbers for various output types
#include "pin_definitions.h"// Pin arrangements per type of board
// board named file with your board found in "hardwareOptions" folder
// eg -> yun.ino <-with-> hardwareOptions/uno.ino

#define MONITOR_BUTTONS 33 // signal to monitor buttons
#define MONITOR_MODE   0   // goto default behavior for multi-mode functions
#define TRIGGER        1   // set enter key to press : enterBehavior()
#define DEFAULT_MODE   1   // potentiometer check
#define NUMBERS_MODE   2   // outputFilter: Numbers
#define RECORD_CAT     2   // cat or record or record cat
#define ADJUST_PWM     2   // potentiometer()
#define ADJUST_TIMING  3   // potentiometer()
#define LINE_SIZE      80  // cause why should a line be longer?

void setup()//setup the needed hardware
{
  pagersUp();          // pagers.ino: brings vibrating motor interface up
  buttonUp();          // buttons.ino: brings button polling intreface up
  serialInterfaceUp(); // yun/uno/leo.ino: brings serial output interface/s up
  EEPROMsetup();       // yun/uno/leo.ino: manages first calibration session
}

void loop()
{
  mainIOroutine();            // handles key input and output
  feedbackAndRelease();       // controls pager feedback and release control
  messageHandlr(MONITOR_MODE);// handles incoming messages
  listenForMessage();         // grab potential messages over usb serial
  potentiometer(MONITOR_MODE);// monitor potentiometer: pagers.ino
  mouseMovement();            // monitor joystick for mouse actions
  serialBowl(MONITOR_MODE);   // check: terminal responses
}

//---- main chord interpertation flow control ----
byte mainIOroutine()
{
  byte pressState = chordLoop(); // captures the current state of the buttons
  if(pressState)
  {
    if (pressState < 128)      // narrows values to letters
    {
      recordHandlr(pressState);// records presses to messageHandlr given active
      keyOut(pressState);      // actuate the press as a keystroke
      messageHandlr(TRIGGER);  // letters interupt messages
    }
    else if(pressState > 159){mouseClick(pressState);}//special cases
    else{macros(pressState);} // in all other cases check for macros
  } // macros are exempt from interupting messageHandlr
  return pressState;
}

byte chordLoop()
{
  static byte doubleActuation = 0;

  byte actuation = 0;            // establish var that will be returned
  if(doubleActuation)            // backspace needed to be sent last iteration
  {                              // double actuation holds the intended char
    actuation = doubleActuation;
    doubleActuation = 0;
    return actuation;
  }

  buttonUpdate();                      // updates internal button state
  int chord = trueChord(MONITOR_MODE); // monitor chord posibilities
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
  hold = holdHandlr(hold);                  // check if there was a hold
  if(hold){varifiedHold = heldASCII(hold);} // only feed in hold events
  else if(!buttonState(MONITOR_BUTTONS)){heldASCII(0);}//feed in release cases

  if(varifiedHold){actuation = varifiedHold;}
  return actuation;
}

//---- feedback & state handling ----
void feedbackAndRelease()
{
  static boolean held = false;
  // general press hold, for holdstates see conversions.ino -> heldASCII()
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
  if(serialBowl(MONITOR_MODE)){return false;} // if terminal mode is printing
  if(messageHandlr(MONITOR_MODE)){return false;} // if message mode is printing
  return true; // as long as everything is inactive return true
}

//---- Special macro functions ----
void macros(byte letter)
{
  if     (letter == 'a' + SPACEBAR){convertionMode(TRIGGER);} // toggle numbers
  else if(letter == 'b' + SPACEBAR) // play message buffer
  {
    if(recordHandlr(MONITOR_MODE)){;}
    else{messageHandlr(RECORD_CAT);}
  }
  else if(letter == 'h' + SPACEBAR){alphaHint();} // play alphabetical hint
  else if(letter == 'i' + SPACEBAR){potentiometer(ADJUST_PWM);} // Toggle to pwm
  else if(letter == 'j' + SPACEBAR){comboPress(LEFT_ALT,0,0);}
  else if(letter == 'k' + SPACEBAR){keyOut(letter);} // toggle keyboard mode
  else if(letter == 'l' + SPACEBAR){comboPress(LEFT_CTRL|LEFT_ALT,0,0);}
  else if(letter == 'p' + SPACEBAR){potentiometer(DEFAULT_MODE);} // show value
  else if(letter == 'r' + SPACEBAR){recordHandlr(TRIGGER);} // start recording
  else if(letter == 's' + SPACEBAR){potentiometer(ADJUST_TIMING);} // toggle
  else if(letter == 't' + SPACEBAR){serialBowl(TRIGGER);} // toggle terminal
}
