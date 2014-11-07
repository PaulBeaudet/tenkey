//hardware.ino  Copyright Paul Beaudet 2014 See license for reuse info
//depends on timing.ino, wire and Adafruit_PWM
// ------------ BOARD SELECT ----- comment in used board
//#define LEO // Arduino Micro or Leonardo USB HID or Bluefruit
#define YUN //Bluefruit is only compatible with yun via software serial 
#define YUN_BOOT_OUTPUT true // mark true to see yun boot msgs on serial
//#define UNO   // Arduinos using the 328p + bluefruit EZ-key HID
//---------------- Haptics / Pagers / Vib motors ------------------------
#include <Wire.h>
#include "Adafruit_PWM.h"
Adafruit_PWM pagers = Adafruit_PWM();

void pagersUp() // to speed up i2c, go into 'fast 400khz I2C' mode
{               // might not work when sharing the I2C bus
  pagers.begin();
  pagers.setPWMFreq(1600);  // This is the maximum PWM frequency
  uint8_t twbrbackup = TWBR;// save I2C bitrate
  // must be changed after calling Wire.begin() (inside pwm.begin())
  TWBR = 12; // upgrade to 400KHz!   
}

#define NUMPAGERS 8 // can use up to 16
void patternVibrate(int pins, int intensityChange = 0)
{ //set state of all pagers in one function
  static int intensity = 4095;  // 0 being off and 4095 being most intense
  if (intensityChange){intensity = intensityChange; return;}
   
  for (byte i=0; i<NUMPAGERS; i++) 
  { // incoming int set bit by bit: high bits: pagers need to be active
    if (pins & (1 << i)) { pagers.setPWM( i, 0, intensity); }
    else/*set pager off*/{ pagers.setPWM( i, 0, 0); }
  }
}


boolean hapticMessage(byte letter, int spacing = 0) 
{ // updating function; passing a string sets course of action
  static boolean animated = false; // animated or typical letter?
  static int timing = 250; //default timing
  
  if(spacing){timing = spacing; return false;}//change timing call
  
  if(letter)
  {
    byte validPatern = charToPattern(letter);
    if(validPatern)
    {
      ptimeCheck(timing);
      patternVibrate(validPatern);
      animated = false;
    }
    byte validAnimation = getFrame(0, letter);
    if(validAnimation)
    {
      int adjustedTime = timing / 2 + timing;
      ptimeCheck(adjustedTime/NUMPAGERS); // a fraction of alotted time
      patternVibrate(validAnimation);
      animated = true;
    } // in this way invalid entries are skipped message reads true for such
    return false;//why bother checking time... we just set it
  }
  //---------- 0 or "monitor" case ------- aka no letter check if done
  if (animated){return animatedProcess(timing);}
  else {return typicalLetter(timing);}
}

boolean typicalLetter(int timing)
{
  static boolean touchPause= 0; // pause between displays
  
  if(ptimeCheck(0))
  {               //time to "display" a touch / pause elapsed
    if(touchPause)//given that we are at the pause stage FINAL
    {             //this case allows for a pause after "display"
      touchPause=!touchPause; //prep for next letter
      return true;//Send confirmation this letter has been "played"
    }
    else          //durring the letter buzz phase
    {
      touchPause=!touchPause;    //flag pause time to start
      patternVibrate(0);         //stop letter feedback
      ptimeCheck(timing/2);//set pause time
    };
  }
  return false;  //signals letter in process of being played
}

boolean animatedProcess(int timing)
{
  static byte frame = 0;
 
  if(ptimeCheck(0))//if timer has been tripped
  {
    frame++;          //zero frame is accounted for in hapticMessage
    if(frame == NUMPAGERS) // reached maxium number of frames
    {
      frame = 0;     //Start back at frame zero
      getFrame(0,1); //reset framer
      return true;   // animation complete
    }
    patternVibrate(getFrame(frame)); //start to play frame
    int adjustedTime = timing / 2 + timing;
    ptimeCheck(adjustedTime/NUMPAGERS);    //devides frame increments
  }
  return false;
}

//--------------- Buttons  ---------------
byte buttons[] = { 10,9,7,8,5,4,6,A3,A2,A0 };// up to 16 possible
// pins can be aligned in software: try to do it in hardware
void buttonUp()// it's cold out there, set up the buttons 
{ //  set every button as an input with internal pull-up resistence
  for (byte set=0; set < sizeof(buttons); set++)
  {pinMode(buttons[set], INPUT_PULLUP);}
}//pull-up -> 20k 5v rail| Pin-> button -> ground:reads low pressed

int buttonSample()
{ // sample the keys and mask/combine them into an interger/"chord"
  int sample=0;//return value to be masked
  for (byte i=0; i < sizeof(buttons); i++)  
  { // when button pressed              set selected bit in sample high
    if(digitalRead(buttons[i]) == LOW) {bitWrite(sample, i, 1);}   
    else                               {bitWrite(sample, i, 0);} 
  }//otherwise                          set the bit low
  return sample;
}// returning and int, allows 16 possible buttons states to be combined

//********** Main chord interpertation function *************
byte chordLoop(int input) // takes sample of buttons: returns true for press
{// main progam loop is abstracted here, so it can be switch with other test
  byte actionableSample= patternToChar(input); //determine chord validity
  if(actionableSample){patternVibrate(input);} //actuate pagers:if letters
  else if(!messageHandlr(JOB) && !serialBowl()){patternVibrate(0);}
  //      except for special printing cases     release:turn pagers off
  return inputFilter(actionableSample);
}//            debounce -> check hold -> return ASCII:letter or action code

//------------SERIAL SETUP --------------------------
void serialInterfaceUp()
{
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
  static boolean printing = false;    //signal activity to outside loop
  #ifdef YUN                          //only do any of this for the yun
    if(hapticMessage(MONITOR_MODE))   //letter played or boot has occurred
    {
      byte incoming = Serial1.read(); //read off a byte regardless
      if (incoming == 255){printing = false;}  //255 = -1 in byte land
      else if (incoming && terminalToggle(1))
      {
        printing = true;               //prevents stop case
        hapticMessage(incoming);       //set incoming byte to be played
        Keyboard.write(incoming);
        //Serial.write(incoming);        
      }
    }
    if(Serial1.available() > 3){Serial1.write(XOFF);}//turn off ash to keep up
    else{Serial1.write(XON);} //resume output of ash
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

//----------------adjusting settings with pontentiometer---------
#define ADJUST_POT A1
#define PWM_ADJUST 4
#define TIMING_ADJUST 5
void potentiometer(byte mode)
{
  static byte adjustMode = PWM_ADJUST;
  int potValue = analogRead(ADJUST_POT);
  
  if(mode == MONITOR_MODE){mode=adjustMode;}//check to do adjustments 
  if(mode == DEFAULT_MODE){potReturn(potValue);}
  else if (mode == ADJUST_PWM)
  {
    adjustMode = PWM_ADJUST;
    potReturn(potValue);
  }
  else if (mode == ADJUST_TIMING)
  {
    adjustMode = TIMING_ADJUST;
    potReturn(potValue);
  }
  else if (mode == PWM_ADJUST)
  {patternVibrate(0, map(potValue, 0, 1023, 0, 4095));}
  else if (mode == TIMING_ADJUST)
  {hapticMessage(0, map(potValue, 0, 1023, 100, 2000));}
}

void potReturn(int potValue)
{
  byte rawNumber = map(potValue,0,1023,0,9);
  keyOut(rawNumber + 48); // turn the raw number into an ascii one
  hapticMessage(rawNumber + 48); // start feedback
  while(!hapticMessage(MONITOR_MODE)){;} //break loop when letter is finished
  keyOut(BACKSPACE); //key message
}
