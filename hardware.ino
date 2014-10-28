//hardware.ino
#include <Wire.h>
#include "Adafruit_PWM.h"

Adafruit_PWM pagers = Adafruit_PWM();

//global variables that need a home..
int PWMintensity = 0; // Adjusts the intensity of the pwm
int HAPTICTIMING = 300; //ms, haptic display durration; user adjustable
//--------expermental hardware--------------
#define BUZZER 11
#define ADJUST_POT A1

//----------PINOUT DEFINITIONS-------------------
byte buttons[] = { 10,9,7,8,5,4,6,A3,A2,A0 };// pin out oppisite to pagers
// pins can be aligned in software: try to do it in hardware
#define NUMBUTTONS sizeof(buttons) // up to 16 possible
#define NUMPAGERS 8 // can use up to 16
//------------HARDWARE SETUP --------------------------
void serialInterfaceUp()
{// all three interfaces are brought up in the ATMEGA32u4 case
  Serial.begin(9600);// serial on uno: bluefruit/message in LEO: message in
  #ifdef LEO // in leo case bluefruit is serial1
    Keyboard.begin(); //begin wired via usb keyboard
    Serial1.begin(9600);
  #endif
  #ifdef YUN
    Bridge.begin();
  #endif
}

void pagersUp() // to speed up i2c, go into 'fast 400khz I2C' mode
{               // might not work when sharing the I2C bus
  pagers.begin();
  pagers.setPWMFreq(1600);  // This is the maximum PWM frequency
  uint8_t twbrbackup = TWBR;// save I2C bitrate
  // must be changed after calling Wire.begin() (inside pwm.begin())
  TWBR = 12; // upgrade to 400KHz!   
}

void buttonUp()// it's cold out there, set up the buttons 
{ //  set every button as an input with internal pull-up resistence
  for (byte set=0;set<NUMBUTTONS;set++){ pinMode(buttons[set], INPUT_PULLUP);}
}//pull-up -> 20k 5v rail| Pin-> button -> ground:reads low pressed

/*********** Harware Functions ************

********** actuating pagers***************/
void patternVibrate(int pins)
{ //set state of all pagers in one function
  for (byte i=0; i<NUMPAGERS; i++) 
  { // incoming int set bit by bit: high bits: pagers need to be active
    if (pins & (1 << i)) { pagers.setPWM( i, 0, PWMintensity); }
    else/*set pager off*/{ pagers.setPWM( i, 0, 0); }
  }
}
//-------------Writing keys to host----------
void keyOut(byte keyPress)
{
  static boolean sticky = false;
  
  #ifdef UNO
    if(keyPress > 128){return;} // these cases need to be translated for UNO
    Serial.write(keyPress); // serves both bluefruit and serial port
  #endif
  #ifdef LEO // in the case of using the ATMEGA32u4 write both interfaces
    Serial1.write(keyPress); // Send out to bluefruit
    if(keyPress==CARIAGE_RETURN){keyPress=KEY_RETURN;}
    if(keyPress > 127 && keyPress < 136) //ctrl, alt, shift, gui
    {
      Keyboard.press(keyPress);
      sticky = true;
    }
    if(sticky && keyPress == SPACEBAR)//stick release cases
    {
      Keyboard.releaseAll();
      sticky = false;
      return;       
    }
    Keyboard.press(keyPress);
    Keyboard.release(keyPress);
  #endif
}

void comboPress(byte first, byte second, byte third) // TODO bluefruit logic
{// more deployable macro triger, would be nice if defaults could be used
  #ifdef LEO
    Keyboard.press(first);
    Keyboard.press(second);
    if(third){Keyboard.press(third);}
    Keyboard.releaseAll();
  #endif
}

void keyRelease()// TODO bluefruit logic
{
  #ifdef LEO
    Keyboard.releaseAll();
  #endif
}

void keyHold(byte key) // TODO bluefruit logic
{
  #ifdef LEO
    Keyboard.press(key);
  #endif
}

void keyPrint(byte message)
{
  #ifdef UNO
     Serial.print(message); // serves both bluefruit and serial port
  #endif
  #ifdef LEO // in the case of using the ATMEGA32u4 write both interfaces
     Serial1.print(message);
     Keyboard.print(message);
  #endif
}
//---------- YUN specific --------------

void useTheCommandLine(char command[])
{
  #ifdef YUN
    linux.runShellCommand(command);
    while(linux.running()){;}
    while(linux.available())
    {
      byte currentOutput = linux.read();
      hapticMessage(currentOutput);
      keyOut(currentOutput);
      while(!hapticMessage(MONITOR_MODE)){;}//wait for letter to "play"
      if(buttonSample()){break;}//break output upon any input
    }
  #endif
}
//---------------Sampling buttons-------------
int buttonSample()
{ // sample the keys and mask/combine them into an interger/"chord"
  int sample=0;//return value to be masked
  for (byte i=0; i<NUMBUTTONS; i++)  
  { // when button pressed              set selected bit in sample high
    if(digitalRead(buttons[i]) == LOW) {bitWrite(sample, i, 1);}   
    else                               {bitWrite(sample, i, 0);} 
  }//otherwise                          set the bit low
  return sample;
}// returning and int, allows 16 possible buttons states to be combined
//----------------adjusting settings with pontentiometer---------
#define PWM_ADJUST 4
#define TIMING_ADJUST 5
void potentiometer(byte mode)
{
  static byte adjustMode = PWM_ADJUST;
  int potValue = analogRead(ADJUST_POT);
  
  if(mode == MONITOR_MODE){mode=adjustMode;}
  //monitor mode; check to do adjustments 
  switch(mode)// modes can check pot or switch modes
  {
    case CHECK_VALUE: potReturn(potValue); break;
    case ADJUST_PWM://switch to pwm adjustment
      adjustMode = PWM_ADJUST; potReturn(potValue);break;
    case ADJUST_TIMING://switch to timing adjustment
      adjustMode = TIMING_ADJUST; potReturn(potValue); break;
    case PWM_ADJUST://mode that does adjusting 
      PWMintensity = map(potValue, 0, 1023, 0, 4095); break;
    case TIMING_ADJUST://mode that does adjusting
      HAPTICTIMING = map(potValue, 0, 1023, 200, 2000); break; 
  } 
}

void potReturn(int potValue)
{
  keyPrint(map(potValue,0,1023,0,9));
  delay(HAPTICTIMING); //give user time to see
  keyOut(BACKSPACE);
}
