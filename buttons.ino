//button.ino -- copyright(c)2014 Paul Beaudet, see licence for details
#define BOUNCETIME 5//ms  /wait this long to be sure of a legit press
#define HOLDSTATE 50//ms /wait this long for a hold cycle
#define DOUBLETIME 230// GO! BILLY BANKS STYLE!
#define TRIGRAMTIME  40 //time it takes for a three letter chord to recognize
#define BIGRAMTIME   43 //weee snaw!
#define UNIGRAMTIME  47 //time it takes for a single press to be recognised
//--------- Button pin-out
byte buttons[] = { 11,10,9,8,7,6,5,4,13,12 };// up to 16 possible

void buttonUp()// it's cold out there, set up the buttons 
{ //  set every button as an input with internal pull-up resistence
  for (byte set=0; set < sizeof(buttons); set++)
  {pinMode(buttons[set], INPUT_PULLUP);}
}//pull-up -> 20k 5v rail| Pin-> button -> ground:reads low pressed

int buttonState(byte bitNumber)
{
  static int state = 0;
  
  if(bitNumber == MONITOR_BUTTONS){return state;}
  else if(bitNumber < 16){bitWrite(state, bitNumber, 1);}//release state
  else{bitWrite(state, bitNumber-16, 0);}//press state
}

int trueChord(boolean mode) //returns chord when 
{  
  static unsigned long chordTime = 0;
  static int chordSize = 0;
  static boolean chordHappening = false;
  
  if(mode)//TRIGGER activate
  {
    chordSize++;
    if(chordSize == 1) //given this is first "note" in the chord
    {
      chordHappening = true; //note possible chord begining
      chordTime = millis();    //note the time in which this occured
    }
    return 0;              //no further action
  }
  //timing //4 buttons == chord full! Just return it!
  int actuation = 0; //zero will be return untill a true chord is detected
  if(chordSize == 4 && chordHappening)
  {actuation = buttonState(MONITOR_BUTTONS);}
  else if(chordSize == 3 && chordHappening)
  {
    if(millis()-chordTime > TRIGRAMTIME)
    {actuation = buttonState(MONITOR_BUTTONS);}
  }
  else if(chordSize == 2 && chordHappening)
  {
    if(millis()-chordTime > BIGRAMTIME)
    {actuation = buttonState(MONITOR_BUTTONS);}
  }
  else if(chordSize == 1 && chordHappening)
  {
    if(millis()-chordTime > UNIGRAMTIME)
    {actuation = buttonState(MONITOR_BUTTONS);}
  }
  if(actuation){chordHappening = false; chordSize = 0;}
  return actuation;//returnState = alternating press/release events
}

void buttonUpdate() //returns press and release states
{//expected return: bit high release / bit high press <- this alternates
  static int releaseStarted = 0; //confirms started release
  static int detectedState = 0;
  static unsigned long time[sizeof(buttons)]= {}; //record
  
  for (byte i=0; i < sizeof(buttons); i++) //for all possible button states
  {
    if(bitRead(buttonState(MONITOR_BUTTONS), i)) 
    {//check if press state has occured, if so proceed to debounce release
      if(digitalRead(buttons[i]) == HIGH)
      { //was the button released since last pressed?
        if(bitRead(releaseStarted, i))       //has a release started
        {
          if(millis()-time[i] > BOUNCETIME)  //sure it is released
          {
            buttonState(i+16);               //set "pressed" back to zero 
            bitWrite(releaseStarted, i, 0);  //remove press note
          }
        }
        else  //if release has yet to start set note that it has started
        {
          bitWrite(releaseStarted, i, 1);   //note release has started
          time[i] = millis();               //time possible release bounce
        }
      }
    }
    else // given this state has yet to be flipped look for an event
    {
      if(digitalRead(buttons[i]) == LOW)
      {
        if(bitRead(detectedState, i)) //given prepared for a read
        {
          if(millis()-time[i] > BOUNCETIME)// !! ACTUATION STATE !!
          {//bits are only flipped when debounced
            buttonState(i); //signal press has begun
            trueChord(true);//signal chord detection function
          }
        }
        else // this the first measure
        {
          bitWrite(detectedState, i, 1); // Show timing has started
          time[1] = millis();          // start timing
        }
      }
      else{bitWrite(detectedState, i, 0);}//given no detected state
    }
  }
}

byte doubleEvent(byte pressState)
{
  static byte firstPress = 0; //signal that first press was complete
  static unsigned long time = 0; //record
  static boolean secondTime = false;
  
  if(pressState)
  {
    if(pressState == firstPress && !secondTime)
    {
      if(millis()-time < DOUBLETIME)
      {
        secondTime = true;
        return pressState;
      }
      else{firstPress = 0;} //opportunity has passed
    }
    else
    {
      firstPress = pressState;
      secondTime = false;
      time = millis();
    }//first case condition
  }
  return 0;
}

byte holdHandlr(byte pressState)//cause I just wanna
{
  static int firstPress = 0;
  static byte heldChar = 0;
  static unsigned long time = 0; //record 
  
  if(firstPress)
  {//held state
    if(firstPress == buttonState(MONITOR_BUTTONS))
    {
      if(millis() - time > HOLDSTATE)//hold turn complete
      {
        time = millis();
        return heldChar;
      }
    }
    else
    {
      firstPress = 0;
      heldChar = 0;
    }//hold broken
  }
  else
  {
    if(pressState)
    {
      time = millis();
      firstPress = buttonState(MONITOR_BUTTONS);
      heldChar = pressState;
    }
  }
  return 0;
}
