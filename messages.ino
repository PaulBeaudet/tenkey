// messages.ino --Copyright Paul Beaudet 2014 See license for reuse info

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
      {
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
  else if(mode == RECORD_CAT) // 2 Concat out lineBuffer
  {
    playFlag = true; // be sure pos is zero, signal playing
    hapticMessage(lineBuffer[pos]); //TODO are the following three redundant?
    keyOut(lineBuffer[pos]);
    pos++;
  }
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

byte positionHandlr(byte mode)
{
  static byte position = 0; //default to possition zero
  
  if     (mode == TRIGGER)              
  {
    position++;
    if(position == LINE_SIZE){position--; Serial.println("#!");}
  }
  else if(mode == RECORD_CAT)           {position=0;}
  else if(mode == BACKSPACE && position){position--;}
  return position;
}

/******* incoming messages **********************
Serial receive message
************************************************/
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
