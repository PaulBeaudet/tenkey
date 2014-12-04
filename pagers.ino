//hardware.ino  Copyright Paul Beaudet 2014 See license for reuse info
//depends on wire and Adafruit_PWM
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

int pagerIntensity(int intensityChange)
{
  static int intensity = 4095;  // 0 being off and 4095 being most intense
  if(intensityChange){intensity = intensityChange;}
  else{return intensity;}
}

void patternVibrate(int pins)
{ //set state of all pagers in one function
  byte j = COUNTBACKPAGERS; // For oppisite direction, if forward remove
  for (byte i=0; i<NUMPAGERS; i++) 
  { // incoming int set bit by bit: high bits: pagers need to be active
    if (pins & (1 << i)) { pagers.setPWM( j, 0, pagerIntensity(MONITOR_MODE));}
    else                 { pagers.setPWM( j, 0, 0); }//change j to i if forward
    j--;// remove if forward oriented
  }
}

boolean ptimeCheck(uint32_t durration)
{                                 // used for checking and setting timer
  static uint32_t ptimer[2]={1,0};// create timer to modify default check=true
  if(durration)                   // given param other than zero
  {                               // time is being set
    ptimer[1]=durration;          // set durration
    ptimer[0]=millis();           // note the time set
  }                               // if the durration has elapsed return true
  else if(millis() - ptimer[0] > ptimer[1]){return true;}//time has passed  
  return false;                   //time has yet to pass
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
      patternVibrate(validPatern);
      animated = false;
    }
    else if(byte validAnimation = getFrame(0, letter))
    {//if 0 frame is availible for this letter
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
      getFrame(0,TRIGGER); //reset framer
      return true;   // animation complete
    }
    patternVibrate(getFrame(frame)); //start to play frame
    int adjustedTime = timing / 2 + timing;
    ptimeCheck(adjustedTime/NUMPAGERS);    //devides frame increments
  }
  return false;
}

//-- higher level message functions
boolean messageHandlr(byte mode)
{
  static char lineBuffer[LINE_SIZE]={};
  static byte pos = 0; // in this way buffer can be no greater than 255
  static boolean playFlag = 0;
  
  if(mode == MONITOR_MODE )
  { // returns play state and handles message output 
    if(playFlag) 
    { 
      if(hapticMessage(MONITOR_MODE))//reads true when current letter done
      {                              //also reads true on start from setup
        if(lineBuffer[pos] == NEW_LINE)
        {// Check for end case before updating further
          removeThisMany(pos);//backspace printed chars
          pos = 0; playFlag = false;//reset possition and playflag
          return false; // play has finished
        }// END CASE: MESSAGE HAS BEEN PRINTED AND REMOVED
        hapticMessage(lineBuffer[pos]);       //start next letter vib
        keyOut(lineBuffer[pos]);              //tx next letter
        pos++;                                //increment read possition
      }//false == waiting -> return -> continue main loop
    }
  }
  else if(mode == TRIGGER) // 1 Trigger interupt
  { // triggering mechinism for message interuption
    if(playFlag)
    {
      removeThisMany(pos);    //backspace printed chars
      pos = 0; playFlag = false;  //reset possition and playflag
    }
  }
  else if(mode == RECORD_CAT){playFlag = true;}//signal play to start
  //------------------ letters cases -----------------
  else if(mode == BACKSPACE){pos--;}//delete buffer entry-> happens in record
  else if(mode == NEW_LINE)
  {
    lineBuffer[pos] = NEW_LINE;    // This signifies end of message!
    pos = 0;                       // prep for read mode or write over
  }
  else if(mode < 128)         // ascii oriented value cases
  {                           // letter input is assumed
    lineBuffer[pos] = mode;   // assign incoming char to buffer
    pos++;                    // increment write position
    if(pos==LINE_SIZE){pos--;}// just take the head till the new line
  }
  return playFlag;
}

//******* Serial receive message *************** 
void listenForMessage()
{ 
  while(Serial.available())
  {
    char singleLetter = (char)Serial.read();
    messageHandlr(singleLetter);//fills up the handlr's lineBuffer
    if(singleLetter == NEW_LINE)
    {
      messageHandlr(TRIGGER);//In the middle of something? don't care
      messageHandlr(RECORD_CAT);// flag to play-> ie concat recording 
      return;
    }
  }
}
//********** message recording *********************** 
boolean recordHandlr(byte mode)
{
  static boolean active = 0; // var for the outside world to undersand state
  static byte recordLength = 0; // Keep track 
  
  if(mode == MONITOR_MODE){return active;}
  if(mode == TRIGGER)
  {// trigger/toggle recording step
    active = !active; // toggle recording state
    if(active){fastToast("rec");} // flash recording warning
    else // in the case recording has been toggled inactive
    {
      removeThisMany(recordLength);recordLength=0; // remove recording
      messageHandlr(NEW_LINE); // make sure recording is closed
    }
  }
  else if(mode == CARIAGE_RETURN)
  { // finish recording step
    messageHandlr(NEW_LINE);
    active = false; // end activity
    removeThisMany(recordLength);recordLength=0; // remove recording
  }
  else if(active && mode < 128) // valid input situations
  { // record step: passes incoming letter to the messageHandlr
    messageHandlr(mode);
    if(mode == BACKSPACE){recordLength--;}
    else{recordLength++;}
  }
  return false;
}
//****************Output Functions ****************************

void fastToast(char message[])//quick indication message
{ // TODO include haptics
  for(byte i=0;message[i];i++){keyOut(message[i]);delay(5);}
  delay(5);
  for(byte i=0;message[i];i++){keyOut(BACKSPACE);}
}

//TODO combine fast toast and alapha hint in to a message

void alphaHint()
{
  for(byte i=97;i<123;i++){messageHandlr(i);} // for all the letters
  messageHandlr(NEW_LINE);//need to know where the end of the statement is
  messageHandlr(RECORD_CAT);//mee-oow! tell it to play the message
}

void removeThisMany(int numberOfChars)
{//remove a numberOfChars...
  for(int i=0;i<numberOfChars;i++){keyOut(BACKSPACE);}
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
  {pagerIntensity(map(potValue, 0, 1023, 0, 4095));}
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
