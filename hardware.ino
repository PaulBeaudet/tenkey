//hardware.ino
#include <Wire.h>
#include "Adafruit_PWM.h"

Adafruit_PWM pagers = Adafruit_PWM();

//global variables that need a home..
int PWMintensity = 0; // Adjusts the intensity of the pwm
int HAPTICTIMING = 200; //ms, haptic display durration; user adjustable
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
  #ifdef UNO
    Serial.begin(9600);// Bluefruit EZ key HID
  #endif
  #ifdef LEO // in leo case bluefruit is serial1
    Serial.begin(9600);//messages in
    Keyboard.begin(); //begin wired via usb keyboard
    Serial1.begin(9600); //possible comunication Bluefruit via pins 0,1
  #endif
  #ifdef YUN
    Keyboard.begin();//begin wired via usb keyboard
    Serial.begin(115200); // debug output / message input 
    Serial1.begin(250000); // begin communication with dd-wrt ash terminal
    // make sure linux has booted and shutdown bridge
    bootCheck(); // returns true for boot, full boot takes about 60sec
    bridgeShutdown();// with bridge shutdown serial1 acts as raw shell access
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

/********** actuating pagers***************/
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
  #ifdef UNO
    if(keyPress > 128){return;} // these cases need to be translated for UNO
  #endif
  
  Serial.write(keyPress); // debug or bluefruit
  byte HIDready = keyPress;
  if(keyPress==CARIAGE_RETURN){HIDready=KEY_RETURN;keyPress = NEW_LINE;}
  
  #ifdef LEO 
    Keyboard.write(HIDready);
  #endif
  #ifdef YUN
    Keyboard.write(HIDready);
    if(terminalToggle(1)){Serial1.write(keyPress);}
  #endif
}

void stickyHandlr(byte keyPress) // TODO intergrate
{
  static boolean sticky = false;
  
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
}

void comboPress(byte first, byte second, byte third) // TODO bluefruit logic
{// more deployable macro triger, would be nice if defaults could be used
  #ifdef LEO
    Keyboard.press(first);
    Keyboard.press(second);
    if(third){Keyboard.press(third);}
    Keyboard.releaseAll();
  #endif
  #ifdef YUN
    Keyboard.press(first);
    Keyboard.press(second);
    if(third){Keyboard.press(third);}
    Keyboard.releaseAll();
  #endif
}

//---------- YUN specific --------------
#ifdef YUN
  void bridgeShutdown()
  {
    Serial1.write((uint8_t *)"\xff\0\0\x05XXXXX\x7f\xf9", 11);//shutdown bridge
    Serial1.println();//send a new line character to enter shutdown garbage
    delay(2);// wait for the buffer to fill with garbage
    while(Serial1.available()){Serial1.read();} // read out shutdown garbage
  }
  
  boolean bootCheck()
  {
    boolean booting = false;//assume complete boot
    ptimeCheck(17800);      //set timer for max distance between boot outputs
    // have recorded +16 sec between some outputs: important if reset midboot
    while(!ptimeCheck(0))   //before the timer is up
    {
      while(Serial1.available())
      {
        bootHandler(YUN_BOOT_OUTPUT);
        booting = true;    //buffer filled before user interaction was possible
      }
    }                      // timer returns true when finished exiting loop
    if (booting)
    {
      ptimeCheck(50000);   //give enough time to finish booting
      while(!ptimeCheck(0))//before time is finished     
      {
        while(Serial1.available()){bootHandler(YUN_BOOT_OUTPUT);}
      }                    //handle rest of output
    }
    return booting;        //in case of conditions looking for boot
  }

  void bootHandler(boolean startUpOutput)//pass true for verbose output
  { //mirror boot process to the serial monitor if true argument is passed
    if(startUpOutput){Serial.write(Serial1.read());} 
    else{Serial1.read();}//empty buffer with empty reads
  }
#endif

boolean serialBowl()
{ // keep the alphabits from overflowing
  static boolean printing = false;
  #ifdef YUN
    if(hapticMessage(MONITOR_MODE)) // letter played or boot has occurred
    {
      byte incoming = Serial1.read(); 
      if (incoming == 255){printing = false;} //255 = -1 in byte land
      else if (incoming && terminalToggle(1))
      {
        printing = true;
        hapticMessage(incoming);
        //Keyboard.write(incoming);
        Serial.write(incoming);
      }
    }
    if(Serial1.available() > 3){Serial1.write(XOFF);}
    else{Serial1.write(XON);}
  #endif
  return printing;
}

boolean terminalToggle(boolean returnVar)
{
  static boolean terminalMode = false;
  if(returnVar){return terminalMode;} //preclude toggle
  #ifdef YUN
    terminalMode = !terminalMode;//terminal mode possible on yun
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
      HAPTICTIMING = map(potValue, 0, 1023, 100, 2000); break; 
  } 
}

void potReturn(int potValue)
{
  byte rawNumber = map(potValue,0,1023,0,9);
  keyOut(rawNumber + 48); // turn the raw number into an ascii one
  delay(HAPTICTIMING); //give user time to see
  keyOut(BACKSPACE);
}
