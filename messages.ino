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
  static byte lineBuffer[BUFFER_SIZE]={};
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
      //else{patternVibrate(0);}//!!! sys wide release:turn pagers off!!!
      return 0;//in any case return to avoid falling thru
    case START_INTERUPT:// completly interupts message 
      if (playFlag) 
      {
        removeThisMany(pos);    //backspace printed chars
        pos = 0; playFlag = 0;  //reset possition and playflag
      }
      return 0; 
    case CAT_OUT:
      playFlag = 1;
      hapticMessage(lineBuffer[pos]);
      keyOut(lineBuffer[pos]);
      pos++;
      return 0;
    case JOB: return playFlag;
    default://SPACE-Z cases concat into buffer
      lineBuffer[pos] = mode; // assign incoming char to buffer
      if (mode == NEW_LINE){pos = 0;}//done recieving: zero possition
      else {pos++;} // increment write possition for more chars
      if(pos==BUFFER_SIZE){pos--;}//just take the head till the new line
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
//****************Output Functions ****************************
void fastToast(byte message[])//quick indication message
{
  for(byte i=0;message[i];i++){keyOut(message[i]);}
  for(byte i=0;message[i];i++){keyOut(BACKSPACE);}
}

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

//************lower level haptic logic***************
boolean ptimeCheck(uint32_t durration)
{//used for checking an setting timer
  static uint32_t ptimer[2] = { };// create timer to modify
  if(durration)
  {
    ptimer[1]=durration; //set durration
    ptimer[0]=millis();  // note the time set
  }
  else if(millis() - ptimer[0] > ptimer[1]){return true;}
  // if the durration has elapsed
  return false;
} 

boolean hapticMessage(byte letter) 
{ // updating function; passing a string sets course of action
  static boolean touchPause= 0; // pause between displays
  
  if(letter)
  {
    ptimeCheck(HAPTICTIMING);//set the time for a letter to display
    patternVibrate(charToPattern(letter));  //start vibrating that letter
    return false;//why bother checking time... we just set it
  }
  //---------- 0 or "monitor" case ------- aka no letter check if done
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
      ptimeCheck(HAPTICTIMING/2);//set pause time
    };
  }
  return false;  //signals letter in process of being played 
}
