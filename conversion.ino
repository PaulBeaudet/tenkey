//conversion.ino- Copyright Paul Beaudet 2014 -See License for reuse info
//Depends on pgmspace
#include<avr/pgmspace.h>//explicitly stated read only memory
//--ANOTHERS layout-- keys a-n-o-t-h-e-r-s make up the homerow 
const byte chordPatterns[] PROGMEM = // alphabetical chord assignment
// each byte is a series of bits that make up a chord
{ //`,  a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,
  160,128, 10,144, 80,  4,  6, 96,  8, 48,192,  5, 24, 60,
//  n,  o,  p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z,  {,  |,  },  ~,
   64, 32,  3,  9,  2,  1, 16, 12,129, 66, 14, 36,112,224,195,  7,165,
}; // array ordered as alphabet (a->1, b->5, ect)

const byte spaceParody[] PROGMEM = // space shift posibilities
{//` , A
  '^',LEFT_ARROW,
// b / c / d / E6/ f / g / H5/ i / j / k / l / m  //38=ampersand
  '%',':','$','!', 47, 38,'?','"', 39,'_','[',']',//39=single quote
  DOWN_ARROW,//n2                                 //47=forwardslash
// O3/ p / q                                      //92=slash
  '.','+','=',
  UP_ARROW, RIGHT_ARROW,// R7/ S8
// T4/ u / v / w / x / y / z / { / | / } / ~
  ',',';','<','>','*', 92,'+','(','-',')','#'
};

const byte numberParody[] PROGMEM = // option shift to numbers
{//`/ A1/ b / c / d / E6/ f / g / H5/ i / j / k / l / m  //38=ampersand
  47,'1','.','^', 47,'6','<','>','5','9','#','@','-','*',//47=forwardslash 
// N2/ O3/ p / q / R7/ S8/ T4/ u / v / w / x / y / z / { / | / } 
  '2','3','+','%','7','8','4','0','"', 39,'x',',','=','{','|','}',
};
#define PATTERNSIZE 31 //sizeof(chordPatterns)

boolean convertionMode(boolean toggle) //toggles numbers or letters mode
{
  static boolean lettersMode = true;
  if(toggle){lettersMode = !lettersMode;}
  return lettersMode;
}

#define L_THUMB        256  // int value of key data
#define R_THUMB        512  // int value of key data
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
        return pgm_read_byte(&spaceParody[i]);
      } //convert letters to special characters
      if(convertionMode(MONITOR_MODE)){return '`' + i;}
      else{return pgm_read_byte(&numberParody[i]);}
    }// return plain char based on possition in the array given no shift
  }
  return 0;
}

byte charToPattern(byte letter)
{ // if any of 4 possible parodies line up
  for (byte i=0; i<PATTERNSIZE; i++)   
  {// for all of the key mapping
    if( letter == ('`'+ i) )                      //typical letter patterns
    {return pgm_read_byte(&chordPatterns[i]);}
    if( letter == pgm_read_byte(&spaceParody[i])) //space combination cases
    {return pgm_read_byte(&chordPatterns[i]);}
    if( letter == pgm_read_byte(&numberParody[i]))//numbers parody chords
    {return pgm_read_byte(&chordPatterns[i]);}
  }// TODO return int with animation signals to differ parodies
  return 0; // no match case
}
//----------Animations---------------
const byte frameStore[] PROGMEM =
{// 0 possition identifies type; other possitions are frames
  128, 64, 32, 16, 8, 4, 2, 1, //spacebar
};

byte getFrame(byte frame, byte type = 0)
{  //Default No activity value ***;
  static byte inProgressType = 255; // refers to dementions in the frame store
  
  if      (type == TRIGGER){inProgressType = 255;}//One is the reset signal
  else if(type == SPACEBAR){inProgressType = 0;}//first dimention
  //TODO make room for future animations
  if (inProgressType == 255){return 0;}
  else {return pgm_read_byte(&frameStore[frame]);}
}

// ------- contextual --------------
const byte homeRow[] PROGMEM = {'a','n','o','t','h','e','r','s',};

boolean isHomerow(byte letter)
{
  for(byte i = 0; i < 8; i++)
  {
    if (letter == pgm_read_byte(&homeRow[i])){return true;}
  }
  return false;
}
