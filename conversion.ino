//conversion.ino- Copyright Paul Beaudet 2014 -See License for reuse info
//Depends on pgmspace
#include<avr/pgmspace.h>//explicitly stated read only memory
//--ANOTHERS layout-- keys a-n-o-t-h-e-r-s make up the homerow 
const byte chordPatterns[] PROGMEM = // alphabetical chord assignment
// each byte is a series of bits that make up a chord
{ //a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,
  128, 10,144, 80,  4,  6, 96,  8, 48,192,  5, 24, 60,
//  n,  o,  p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z, 
   64, 32,  3,  9,  2,  1, 16, 12,129, 66, 14, 36,112,
}; // array ordered as alphabet (a->1, b->5, ect)

const byte spaceParody[] PROGMEM = // space shift posibilities
{//A1/ b / c / d / E6/ f / g / H5/ i / j / k / l / m  //38=ampersand
  '@','{','}','$','-', 47, 38, 39,'#','(',')','[',']',//47=forwardslash 
// N2/ O3/ p / q / R7/ S8/ T4/ u / v / w / x / y / z  //39=single quote
  '?','!','.','=','"', 92,',','_','<','>','*','|','+',//92=slash
};

const byte backParody[] PROGMEM = //backspace shift posibilities
{//A1/ b / c / d / E6/ f / g / H5/ i / j / k / l / m  //38=ampersand
  '`','{',':','$','-', 47, 38, 39,'#','(',')','[',']',//47=forwardslash 
// N2/ O3/ p / q / R7/ S8/ T4/ u / v / w / x / y / z  //39=single quote
  '?','!','%','=','"',';','~','_','^','>','*','|','+',//92=slash
};

const byte numberParody[] PROGMEM = // option shift to numbers
{//A1/ b / c / d / E6/ f / g / H5/ i / j / k / l / m  //38=ampersand
  '1','.','^', 47,'6','<','>','5','9','(',')','-','*',//47=forwardslash 
// N2/ O3/ p / q / R7/ S8/ T4/ u / v / w / x / y / z  //39=single quote
  '2','3','+','%','7','8','4','0','"', 39,'x',',','=',//92=slash
};
#define PATTERNSIZE 26 //sizeof(chordPatterns) //14,936

boolean convertionMode(boolean toggle = 0) //toggles numbers or letters mode
{
  static boolean lettersMode = true;
  if(toggle){lettersMode = !lettersMode;}
  return lettersMode;
}

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
      if (base & L_THUMB)//second level shift *combination with backspace
      {// 256 = 01-0000-0000 // if(9th bit is high) 
        return pgm_read_byte(&backParody[i]);
      }
      if(convertionMode()){return 'a' + i;}
      else{return pgm_read_byte(&numberParody[i]);}
    }// return plain char based on possition in the array given no shift
  }
  return 0;
}

byte charToPattern(byte letter)
{ // if any of 4 possible parodies line up
  for (byte i=0; i<PATTERNSIZE; i++)   
  {// for all of the key mapping
    if( letter == ('a'+ i) )                      //typical letter patterns
    {return pgm_read_byte(&chordPatterns[i]);}
    if( letter == pgm_read_byte(&spaceParody[i])) //space combination cases
    {return pgm_read_byte(&chordPatterns[i]);}
    if( letter == pgm_read_byte(&backParody[i]))  //back combination cases 
    {return pgm_read_byte(&chordPatterns[i]);}
    if( letter == pgm_read_byte(&numberParody[i]))//numbers parody chords
    {return pgm_read_byte(&chordPatterns[i]);}
  }// TODO return int with animation signals to differ parodies
  return 0; // no match case
}
