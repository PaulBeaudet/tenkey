//input.ino- Copyright Paul Beaudet 2014 -See License for reuse info
byte inputFilter(byte input)
{ // prevent output from being the same as the last
  static byte lastEvent = 0;
  byte output = 0;
  
  if(byte event = holdFilter(input))// was there an event?
  {// only allow it if it is differant then the last event
    if(event == BACKSPACE){lastEvent = event; return event;} 
    // include held doubles for backspace
    if (event != lastEvent)
    { // we have differant events 
      output = event; // good, its allowed to be and event
      lastEvent = event; // note current event for next check
      if(uniAfterMulti(event, lastEvent)){output = 0;}//unless uniAfterMulti
    }   
  } // if the input is zero and enough time has elapsed reset the condition
  else if (!input && spacerTimer(0)>9){lastEvent = 0;} 
  //button has been let go long enough to discount odd stuff
  return output; 
}

boolean uniAfterMulti(byte currentInput, byte lastInput)
{ // was the last event a uni-gram? if so 
  boolean lastWasUni = false;//guilty till proven inocent
  if(isHomerow(lastInput) || isNumRow(lastInput)){lastWasUni = true;}
  else // given last input was a multi
  {    // check if current press was on the homerow
    if (isHomerow(currentInput) || isNumRow(currentInput)){return true;}
  }//signal true if uni after multi
  return false;
}

byte holdFilter(byte input)
{
  static byte lastInput = 0;
  byte output = 0;// output defaults to zero
  
  if (input != lastInput){spacerTimer(TRIGGER);}// given change reset clock
  else if (input)// if the current input is consitent with the last
  { // check how long the input has been pressed
    if(input == BACKSPACE)    {output = backActions(spacerTimer(0));}
    else if(input == SPACEBAR){output = spaceActions(spacerTimer(0));}
    else if(isNumRow(input))  {output = numberRow(input, spacerTimer(0));}
    else if(isHomerow(input)) {output = homerow(input, spacerTimer(0));}
    else if(input < 127 && input > 95) //multi key letter presses
                              {output = chordActions(input, spacerTimer(0));}
    else if(spacerTimer(0) == 3){output = input;} //outside cases play
  }                                               // on first step
  lastInput = input;
  return output;
}
/**************************
-HOLD FLOW-
1. debounce- accept valid input
2. Holdover- Remover char in preperation for upper cases
3. Capitilize- print upper case chare
4. Holdover- Remover char in preperation for special commands
5. Special Cases- Programed 'command' cases for special features
**************************/
byte homerow(byte input, byte progress)
{
  if(progress == 7) {return input;}
  return holdTiming(input, progress);
}

byte numberRow(byte input, byte progress)
{
  if(progress==7)   {return input;}
  if(progress == 70){return BACKSPACE;}
  if(progress == 75){return 129;} // signal switch back to letters
  return 0;
}

byte backActions(byte progress)
{// if holding more than X return when timeing is devisible by 3 or 12
  if(progress == 5 || progress > 30 && progress % 3 == 0){return BACKSPACE;} 
  return 0; // terminate outside backspace cases
}

byte spaceActions(byte progress)
{
  if(progress == 7){return SPACEBAR;}
  if(progress == 35){return BACKSPACE;}//be sure the tab is a true tab
  if(progress == 40){return TAB_KEY;}//hold for tab case
  return 0; // terminate outside space cases
}

byte chordActions(byte input, byte progress)
{
  if(progress == 2) {return input;}
  return holdTiming(input, progress);
}

byte holdTiming(byte input, byte progress)
{
  if(progress == 25){return BACKSPACE;}
  if(progress == 30){return input-SPACEBAR;}
  if(progress == 70){return BACKSPACE;}
  if(progress == 75){return input+SPACEBAR;}
  return 0;
}
