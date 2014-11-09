/********tenkey_Neotype.ino*********
* Copyright Paul Beaudet Created Fall 2014 <inof8or@gmail.com>
* Keyer that recieves and transmits text information from its user
* This program is free open source software: See licenses.txt to understand 
  reuse rights of any file associated with this project
********* See readme.md for hardware discription **************/
#define MONITOR_MODE   0 // goto default behavior for multi-mode functions
#define TRIGGER        1 // set enter key to press : enterBehavior()
#define DEFAULT_MODE   1 // outputFilter: regular letters, potentiometer check
#define NUMBERS_MODE   2 // outputFilter: Numbers
#define RECORD_CAT     2 // enterBehavior() cat or record or record cat
#define ADJUST_PWM     2 // potentiometer()
#define ADJUST_TIMING  3 // potentiometer()
#define LINE_SIZE      80   //cause why should a line be longer?
#define NEW_LINE       '\n' //determines end of a message in buffer
#define BACKSPACE      8    // output keys
#define TAB_KEY        9
#define SPACEBAR       32
#define CARIAGE_RETURN 13
#define XON            17 // control_Q resume terminal output
#define XOFF           19 // control_S stop terminal output

void setup()//setup the needed hardware  
{
  pagersUp();          // hardware.ino: brings vibrating motor interface up
  buttonUp();          // hardware.ino: brings button polling intreface up
  serialInterfaceUp(); // hardware.ino: brings serial output interface/s up
}

void loop() 
{
  byte pressState = chordLoop(buttonSample());//<--important part
  // captures the current state of the buttons
  if(pressState)
  {   
    if (pressState < 128) //if letter press state
    {
      recordHandlr(pressState);//records presses to messageHandlr given active
      keyOut(pressState);      //actuate the press as a keystroke
      messageHandlr(TRIGGER);  //letters can interupt
    } 
    else // if a macro pressState
    {
      macros(pressState);
      messageHandlr(MONITOR_MODE);
    } // otherwise read messages as available
  }//macros are exempt from interupting messageHandlr
  //EXTRA FEATURES 
  listenForMessage();// grab potential messages over usb serial
  potentiometer(MONITOR_MODE);//monitor potentiometer for setting adjustment
  //Yun specific
  serialBowl(); // check: terminal responses
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
  else if(letter == 'i' + SPACEBAR){potentiometer(ADJUST_PWM);} //Toggle to pwm
  else if(letter == 'p' + SPACEBAR){potentiometer(DEFAULT_MODE);}//show value
  else if(letter == 'r' + SPACEBAR){recordHandlr(TRIGGER);}//start recording
  else if(letter == 's' + SPACEBAR){potentiometer(ADJUST_TIMING);}//toggle 
  else if(letter == 't' + SPACEBAR){terminalToggle(0);}//toggle terminal shell
}
