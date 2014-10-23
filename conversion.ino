/********* conversion.ino - Program Flow *******************************
buttonSample() get the state of the buttons: Depends on: hardware.h
returns and byte where each bit is a button

patternToChar() turn the button sample into a represented character
  chordPatterns 
  
patternVibrate() drive pagers, pass a byte where each bit is a motor

* Important note: Valid letter data is debounced, not individual keys
inputFilter() debounce cumalitive input and detect holds
  holdFilter()
********ASCII KEY******** Letter debouncing and convertion *************/

//   --ANOTHERS-- layout a-n-o-t-h-e-r-s make up the homerow 
const byte chordPatterns[] PROGMEM =
// each byte is a series of bits that make up a chord
{ //a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,
  128, 10,144, 80,  4,  6, 96,  8, 48,192,  5, 24, 60,
//  n,  o,  p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z, 
   64, 32,  3,  9,  2,  1, 16, 12,129, 66, 14, 36,112,
}; // array ordered as alphabet (a->1, b->5, ect)
#define PATTERNSIZE 26 //sizeof(chordPatterns)

byte patternToChar(int base) //returns the char value of a raw chord
{// some convertions can explicitly imediately be returned 
  if(base == L_THUMB){return BACKSPACE;}//also:2nd level shift, special chars
  if(base == R_THUMB){return SPACEBAR;}//also doubles: first shift in a chord
  if(base == (R_THUMB | L_THUMB)){return CARIAGE_RETURN;}
  //combination: space + backspace == Enter
  
  for (byte i=0; i<PATTERNSIZE; i++)   
  {// for all asignments in key mapping   
    if ( (base & ~(R_THUMB | L_THUMB)) == pgm_read_byte(&chordPatterns[i])) 
    {//incoming chord ignoring thumbs     check for matching chord 
      if (base & R_THUMB)//first level shift *combination with space
      {// 512 = 10-0000-0000 // if( 10th bit is fliped high )
        //if(lower shift, less than 10th result) {return corrisponding number}
        return spaceParody(i);//convert letters to special characters
      } 
      if (base & L_THUMB)//second level shift *combination with backspace
      {// 256 = 01-0000-0000 // if(9th bit is high) 
        return backParody(i);
      }
      return 'a' + i;
    }// return plain char based on possition in the array given no shift
  }
  return 0;
}

byte spaceParody(byte letterPos)
{//second level space chords AKA space + a,b,c,d ext
  switch(letterPos)
  {
    case 0:  return '@';//a
    case 1:  return '{';//b
    case 2:  return '}';//c -> maybe carot with auto closing brackets
    case 3:  return '$';//d
    case 4:  return '-';//e
    case 5:  return 47 ;//f -> forward slash
    case 6:  return 38 ;//g -> ampersand
    case 7:  return 39 ;//h -> single quote
    case 8:  return '#';//i
    case 9:  return '(';//j
    case 10: return ')';//k -> maybe tilde with type correction
    case 11: return '[';//l
    case 12: return ']';//m -> maybe apostorpy with type correction
    case 13: return '?';//n
    case 14: return '!';//o
    case 15: return '.';//p
    case 16: return '=';//q
    case 17: return '"';//r
    case 18: return 92 ;//s ->slash 
    case 19: return ',';//t ->comma
    case 20: return '_';//u
    case 21: return '<';//v
    case 22: return '>';//w
    case 23: return '*';//x
    case 24: return '|';//y
    case 25: return '+';//z which leaves (~ ` ^ % ;:) leftover
  }
}

byte backParody(byte letterPos)
{//second level back chords: AKA back + a,b,c,d ext
  switch(letterPos)
  {
    case 0: return '`';//a
    case 1://b
    case 2: return ':';//c
    case 3://d
    case 4://e
    case 5://f
    case 6://g
    case 7://h
    case 8://i
    case 9://j
    case 10://k
    case 11://l
    case 12://m
    case 13://n
    case 14://o
    case 15: return '%';//p
    case 16://q
    case 17://r
    case 18: return ';';//s
    case 19: return '~';//t
    case 20://u
    case 21: return '^';//v
    case 22://w
    case 23://x
    case 24://y
    case 25: return 0;//z //no cases for now
  }
}

byte charToPattern(byte letter)
{
  if(letter == SPACEBAR){return 0;}//Express convertion: Space 
  //skip space for now, maybe animate a signal in the future
  // Space also doubles as the first shift in a chord
  for (byte i=0; i<PATTERNSIZE; i++)   
  {// for all of the key mapping
    if ( letter == ('a'+ i) ){return pgm_read_byte(&chordPatterns[i]);}
    //return typicall letter patterns
    if ( letter == spaceParody(i)) {return pgm_read_byte(&chordPatterns[i]);}
    // space chord combination cases = special characters
    if ( letter == backParody(i)) {return pgm_read_byte(&chordPatterns[i]);}
    // back chord combination cases 
  }
  return 0;
}
