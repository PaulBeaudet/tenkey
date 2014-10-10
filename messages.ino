//--------------messaging functions-----------------------------
void hapticAlpha()
{
  for(byte i=97;i<123;i++)
  {//interate through all the letters in the alphabet
    hapticMessage(i);//ask for a letter in the alphabet
    SERIALINTERFACE.write(i);//write the letter
    while(!hapticMessage()){;}//wait for the char to finish
    SERIALINTERFACE.write(8);//remove letter
  }
}

void toast(char message[])
{// message the appears and disapears, just like "toast" in android
  btMessage(message);//print the message
  while(hapticMessage(message) != 128){;}//wait for haptic message to finish
  rmMessage(message);// remove message
}

void btMessage(char message[])
{
  for(int pos=0;message[pos];pos++){SERIALINTERFACE.write(message[pos]);}
}//print message

void rmMessage(char message[])
{//remove a message
  for(int i=0;message[i];i++){SERIALINTERFACE.write(8);}
}

//----------------------haptic logic----------------------------
boolean ptimeCheck(uint32_t durration)
{//used for checking an setting timer
  static uint32_t ptimer[2] = { };// create timer to modify
  if(durration)
  {
    ptimer[1]=durration; //set durration
    ptimer[0]=millis();  // note the time set
  }
  else if(millis() - ptimer[0] > ptimer[1])
  {// if the durration has elapsed
    return true;
  }
  return false;
} 

void hapticMessage(byte letter) // intializing function
{ // set a letter to be "played"
  ptimeCheck(HAPTICTIMING);
  patternVibrate(charToPattern(letter));
}

boolean hapticMessage() 
{ // updating function; passing a string sets course of action
  static boolean touchPause= 0;

  if(ptimeCheck(0))
  {//time to "display" a touch has elapsed
    if(touchPause)
    {//this case allows for a pause after "display"
      touchPause=!touchPause;
      return true;
    }
    else
    {
      touchPause=!touchPause;
      patternVibrate(0);//stop the message
      ptimeCheck(HAPTICTIMING/2);
    };
  }
  return false;
}

byte hapticMessage(char message[])
{ //sending the message param set a course of action, no param executes
  static byte possition = 0;
  byte onLetter = message[possition];

  if(!onLetter)
  {
    possition = 0;
    while (!hapticMessage())
    {//finish last "touch"
      ; //figure out how to get rid of this pause latter
    }
    return 128;//signal the message is done
  }
  if (hapticMessage())//refresh display
  {
    hapticMessage(onLetter);
    possition++;
    return onLetter;
  }
  return 0;
}

boolean checkMatch(char input[], char target[])
{
  for(byte i=0;target[i];i++)
  {
    if(input[i]!=target[i])
    {
      return false;
    }
  }
  return true;
}
