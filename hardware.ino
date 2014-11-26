//hardware.ino  Copyright Paul Beaudet 2014 See license for reuse info
//depends on timing.ino, wire and Adafruit_PWM

//---------------- Haptics / Pagers / Vib motors -----------------
#include "Adafruit_PWM.h" // see readme for harware notes
#define NUMPAGERS 8 // can use up to 16
#define COUNTBACKPAGERS 8 - 1
Adafruit_PWM pagers = Adafruit_PWM();// create pagers object

void pagersUp() // to speed up i2c, go into 'fast 400khz I2C' mode
{               // might not work when sharing the I2C bus
  pagers.begin();
  pagers.setPWMFreq(1600);  // This is the maximum PWM frequency
  uint8_t twbrbackup = TWBR;// save I2C bitrate
  // must be changed after calling Wire.begin() (inside pwm.begin())
  TWBR = 12; // upgrade to 400KHz!
}

void patternVibrate(int pins, int intensityChange)
{ //set state of all pagers in one function
  static int intensity = 4095;  // 0 being off and 4095 being most intense
  if (intensityChange){intensity = intensityChange; return;}
  
  byte j = COUNTBACKPAGERS; // For oppisite direction, if forward remove
  for (byte i=0; i<NUMPAGERS; i++) 
  { // incoming int set bit by bit: high bits: pagers need to be active
    if (pins & (1 << i)) { pagers.setPWM( j, 0, intensity); }//<--- j to i
    else/*set pager off*/{ pagers.setPWM( j, 0, 0); }//change j to i if forward
    j--;// remove if forward oriented
  }
}

boolean hapticMessage(byte letter, int spacing = 0) 
{ // updating function; passing a string sets course of action
  static boolean animated = false; // animated or typical letter?
  static int timing = 250; //default timing
  
  if(spacing){timing = spacing; return false;}//change timing call
  
  if(letter)
  {    
    if(byte validPatern = charToPattern(letter))
    {
      ptimeCheck(timing);
      patternVibrate(validPatern, 0);
      animated = false;
    }
    else if(byte validAnimation = getFrame(0, letter))
    {//if 0 frame is availible for this letter
      int adjustedTime = timing / 2 + timing;
      ptimeCheck(adjustedTime/NUMPAGERS); // a fraction of alotted time
      patternVibrate(validAnimation, 0);
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
      patternVibrate(0, 0);         //stop letter feedback
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
      getFrame(0,TRIGGER); //reset framer
      return true;   // animation complete
    }
    patternVibrate(getFrame(frame), 0); //start to play frame
    int adjustedTime = timing / 2 + timing;
    ptimeCheck(adjustedTime/NUMPAGERS);    //devides frame increments
  }
  return false;
}

//--------------- Buttons  ---------------
/*const uint8_t buttons[] = { 11,10,9,8,7,6,5,4,13,12 };// up to 16 possible
// pins can be aligned in here if miswired: try to do it right in hardware
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
  else if(!messageHandlr(MONITOR_MODE) && !serialBowl()){patternVibrate(0);}
  //      except for special printing cases     release:turn pagers off
  return inputFilter(actionableSample);
}//            debounce -> check hold -> return ASCII:letter or action code
*/
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
