/*input.ino**********debouncing and chord interpertation******
Gives fine grain control over every posible input case
allowing for differant timings depending on the chords

        FLOW
main call from loop -> inputFilter(input) returns actuations from raw samples
     Character cases : differant button behave in unique ways
     homerow   : keys count as "pressed" upon 
Note about spacerTimer
decide when to reset in -> inputFilter
poll inside of ----------> homerow, backActions, spaceActions, chordActions
*************************************************************/
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
  else if (!input && spacerTimer(0)>10){lastEvent = 0;} 
  //button has been let go long enough to discount odd stuff
  return output; 
}

boolean uniAfterMulti(byte currentInput, byte lastInput)
{ // was the last event a uni-gram? if so 
  boolean lastWasUni = false;//guilty till proven inocent
  switch(lastInput)
  {
    case 'a':
    case 'n':
    case 'o':
    case 't':
    case 'h':
    case 'e':
    case 'r':
    case 's': lastWasUni = true;
  }
  if (lastWasUni == false)
  {
    switch(currentInput)
    {
      case 'a':
      case 'n':
      case 'o':
      case 't':
      case 'h':
      case 'e':
      case 'r':
      case 's': return true; // uni followed by a bi,tri,or Quad gram
    } // this is honestly and unlikey case within a small time frame
  }
  return false;
}

byte holdFilter(byte input)
{
  static byte lastInput = 0;
  byte output = 0;// output defaults to zero
  
  if (input != lastInput){spacerTimer(1);}// given change reset clock
  else // if the current input is consitent with the last
    { // check how long the input has been pressed
    switch(input)
    {//
      case 0: break; // input is zero? great that is what output defaults to!
      //   detect special chars
      case CARIAGE_RETURN: if(spacerTimer(0)==1){output = input;}break;
      case BACKSPACE: output = backActions(spacerTimer(0));break;
      case SPACEBAR:  output = spaceActions(spacerTimer(0));break;
      //   detect homerow chars AKA unigrams
      case '1': output = oneException(spacerTimer(0)); break;
      case 'a':// |
      case 'n':// |
      case 'o':// |
      case 't':// |
      case 'h':// |
      case 'e':// |
      case 'r':// v
      case 's': output = homerow(input, spacerTimer(0)); break;
      // detect bi and quad-gram situations 
      case 'b':
      case 'c':
      case 'd':
      case 'f':
      case 'g':
      case 'i':
      case 'j':
      case 'k':
      case 'l':
      case 'm':
      case 'p':
      case 'q':
      case 'u':
      case 'v':
      case 'w':
      case 'x':
      case 'y':
      case 'z':output = chordActions(input, spacerTimer(0));break; 
      //special character situations (<91)
      default: if(spacerTimer(0)==1){output = input;}break;    
    }
  }
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
  switch(progress)
  {
    case 7:  return input;// typical print
    case 40: return BACKSPACE;// delete to make room for a cap
    case 45: return input-SPACEBAR;//downshift subtract 32 to get caps
    case 80: return BACKSPACE;//delete char: prep for special commands
    case 85: return input+SPACEBAR;//upshift: special commands
    default: return 0;
  }
}

byte oneException(byte progress)
{
  if(progress==7) {return '1';}
  else if(progress == 80){return BACKSPACE;}
  else if(progress == 85){return 129;} // signal a hold
  return 0;
}

byte backActions(byte progress)
{// if holding more than X return when timeing is devisible by 3 or 12
  if(progress == 5 || progress > 30 && progress % 3 == 0){return BACKSPACE;} 
  return 0; // terminate outside backspace cases
}

byte spaceActions(byte progress)
{
  if(progress == 2){return SPACEBAR;}
  if(progress == 35){return BACKSPACE;}//be sure the tab is a true tab
  if(progress == 40){return TAB_KEY;}//hold for tab case
  return 0; // terminate outside space cases
}

byte chordActions(byte input, byte progress)
{
  switch(progress)
  {
    case 2:  return input;
    case 40: return BACKSPACE;
    case 45: return input-SPACEBAR;
    case 80: return BACKSPACE;
    case 85:return input+SPACEBAR;
    default: return 0;
  }
}
