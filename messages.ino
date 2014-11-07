/***********messages.ino **************************
Fuctions involved in returning messages to the user
Message handeling flow
messageHandlr() has two types of call, an istantiation and matianance
0-fill global message lineBuffer -> static in handeler  <--\
1-intiate message playing flag -> static in handler <---\- one time
 Note: new message and letters intrerupt presenting issues of placekeeping
 new letter will interupt message: as message is still in buffer
 it can be played back whenever by holding B
 backspaces will be handled by an external placeKeep function
2-
**********messaging functions*********************/

boolean messageHandlr(byte mode)
{
  static char lineBuffer[LINE_SIZE]={};
  static byte pos = 0; // in this way buffer can be no greater than 255
  static boolean playFlag = 0;
  
  switch(mode)
  {
    case MONITOR_MODE:
      if(playFlag)
      {
        if(lineBuffer[pos]==NEW_LINE)
        {// Check for end case before updating further
          removeThisMany(pos);//backspace printed chars
          playFlag=0; pos = 0;//reset possition and playflag
          return 0;
        }// END CASE: MESSAGE HAS BEEN PRINTED AND REMOVED
        if(hapticMessage(MONITOR_MODE))//<---Updates Letter display
        {//true == single letter display finished   
          hapticMessage(lineBuffer[pos]);       //start next letter vib
          keyOut(lineBuffer[pos]);//tx next letter
          pos++;//increment read possition
        }//false == waiting -> return -> continue main loop
      }//playFlag false == no directive to play ->continue main loop
      return 0;//in any case return to avoid falling thru
    case START_INTERUPT://1 completly interupts message 
      if (playFlag) 
      {
        removeThisMany(pos);    //backspace printed chars
        pos = 0; playFlag = 0;  //reset possition and playflag
      }
      return 0; 
    case CAT_OUT://2
      playFlag = 1;
      hapticMessage(lineBuffer[pos]);
      keyOut(lineBuffer[pos]);
      pos++;
      return 0;
    case JOB: return playFlag; // 4
    default://SPACE-Z cases concat into buffer
      if (mode > 128){break;}//ignore special cases
      if (mode == BACKSPACE){ pos--; break;} //delete buffer entry "RECORD"
      lineBuffer[pos] = mode; // assign incoming char to buffer
      if (mode == NEW_LINE){pos = 0;}//done recieving: zero possition
      else {pos++;} // increment write possition for more chars
      if(pos==LINE_SIZE){pos--;}//just take the head till the new line
  }  
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
      messageHandlr(START_INTERUPT);//In the middle of something? don't care
      messageHandlr(CAT_OUT);// flag to play 
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
  else if(mode == RECORD)
  { // finish recording step
    active = false; // end activity
    removeThisMany(recordLength);recordLength=0; // remove recording
  }
  else if(active) // catches all other values accept for the mode ones
  { // record step: passes incoming letter to the messageHandlr
    messageHandlr(mode);
    if(mode == BACKSPACE){recordLength--;}
    else if (mode > 127){;}//nonprinting char skip
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
  messageHandlr(CAT_OUT);//mee-oow! tell it to play the message
}

void removeThisMany(int numberOfChars)
{//remove a numberOfChars...
  for(int i=0;i<numberOfChars;i++){keyOut(BACKSPACE);}
}
