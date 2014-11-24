//input.ino- Copyright Paul Beaudet 2014 -See License for reuse info
byte inputFilter(byte input)
{ // prevent output from being the same as the last
  static byte lastEvent = 0;
  byte output = 0;
  
  if(byte event = holdFilter(input))// was there an event?
  {// only allow it if it is differant then the last event
    if(isRepeating(event)){lastEvent = event; return event;} 
    // include held doubles for backspace
    if (event != lastEvent)
    { // we have differant events 
      output = event; // good, its allowed to be and event
      lastEvent = event; // note current event for next check
      if(uniAfterMulti(event, lastEvent)){output = 0;}//unless uniAfterMulti
    }//TODO test if uniAfterMulti actually works
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
    byte progress = spacerTimer(MONITOR_MODE); //poll current time
    if(isRepeating(input))   {output = repeatingKeys(input, progress);}
    else if(isNumRow(input)) {output = numberRow(input, progress);}
    else if(isHomerow(input)){output = homerow(input, progress);}
    else if(input < 128 && input > 95) //multi key letter presses
                             {output = chordActions(input, progress);}
    else if(progress == 3){output = input;} //outside cases play
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
  if(input == SPACEBAR && progress > 20){return 0;} //TODO reintroduce ALT
  return holdTiming(input, progress);
}

byte numberRow(byte input, byte progress)
{
  if(progress==7)   {return input;}
  if(progress == 70){return BACKSPACE;}
  if(progress == 75){return 129;} // signal switch back to letters
  return 0;
}

byte repeatingKeys(byte input, byte progress)
{// if holding more than X return when timeing is devisible by 3 or 12
  if(progress == 5 || progress > 30 && progress % 3 == 0){return input;} 
  return 0; // terminate outside backspace cases
}

byte chordActions(byte input, byte progress)
{
  if(progress == 2)
  {
    if(input == 127){return TAB_KEY;}
    return input;
  }
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
