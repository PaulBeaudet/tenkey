//conversion.ino- Copyright Paul Beaudet 2014 -See License for reuse info
//Depends on pgmspace
#include <avr/pgmspace.h>//explicitly stated read only memory
//--ANOTHERS layout-- keys a-n-o-t-h-e-r-s make up the homerow 
const byte chordPatterns[] PROGMEM = // alphabetical chord assignment
// each byte is a series of bits that make up a chord
{ //`,  a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,
  160,128, 10,144, 80,  4,  6, 96,  8, 48,192,  5, 24, 60,
//  n,  o,  p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z,  {,  |,  },  ~,del
   64, 32,  3,  9,  2,  1, 16, 12,129, 66, 14, 36,112,224,195,  7,165,102
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
// T4/ u / v / w / x / y / z / { / | / } / ~ /del
  ',',';','<','>','*', 92,'+','(','-',')','#',TAB_KEY
};

const byte numberParody[] PROGMEM = // option shift to numbers
{//`/ A1/ b / c / d / E6/ f / g / H5/ i / j / k / l / m  //38=ampersand
  47,'1','.','^', 47,'6','<','>','5','9','#','@','-','*',//47=forwardslash 
// N2/ O3/ p / q / R7/ S8/ T4/ u / v / w / x / y / z / { / | / } /del
  '2','3','+','%','7','8','4','0','"', 39,'x',',','=','{','|','}',TAB_KEY
};
#define PATTERNSIZE SPACEBAR //sizeof(chordPatterns)

boolean convertionMode(boolean toggle) //toggles numbers or letters mode
{
  static boolean lettersMode = true;
  if(toggle){lettersMode = !lettersMode;}
  return lettersMode;
}

#define L_THUMB 256  // int value of key data
#define R_THUMB 512  // int value of key data
byte patternToChar(int base) //returns the char value of a raw chord
{// some convertions can explicitly imediately be returned 
  if(base == L_THUMB){return BACKSPACE;}//also:2nd level shift, special chars
  if(base == R_THUMB){return SPACEBAR;}//also doubles: first shift in a chord
  if(base == 102){return 0;} //number otherwise assigned to del possition
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

byte heldASCII(byte letter)
{
  static unsigned long holdTimes = 0;
  
  if(letter){holdTimes++;}
  else{holdTimes = 0;}
  
  if(letter == SPACEBAR)
  {
    if(holdTimes == 30){return BACKSPACE;}
    if(holdTimes == 31){return 138;}// 'j' plus SPACEBAR
    return 0;
  }
  if(letter == TAB_KEY)
  {
    return 0;
  }
  
  if(holdTimes == 18 && letter > 95){return BACKSPACE;}
  if(holdTimes == 19)// first hold
  {//letters covered by main layout
    if(letter > 95){return letter-SPACEBAR;} //shift cases
    if(letter == '1'){return 0;} //empty cases return 0 
    return letter; //outside cases are repeating
  }
  if(holdTimes == 42 && letter > 95){return BACKSPACE;}
  if(holdTimes == 43) //second hold 
  {//letters covered by main layout
    if(letter > 95){return letter+SPACEBAR;} //macro cases 
    if(letter == '1'){return 129;} 
    return letter; //outside cases are repeating
  }
  if(holdTimes > 43)
  {//outside main layout letters repeat
    if(letter < 95 && letter != '1' ){return letter;}
  }
  return 0; //cases not covered
}

byte doubleToASCII(byte letter)
{
  if(letter == TAB_KEY){return 0;}
  else if(letter == SPACEBAR){return 0;}
  else if(letter == BACKSPACE){return 0;}
  else if(letter == CARIAGE_RETURN){return 0;}
  else if(letter){return pgm_read_byte(&numberParody[letter-'`']);}
  return 0;
}

byte charToPattern(byte letter)
{ // if any of 4 possible parodies line up
  for (byte i=0; i<PATTERNSIZE; i++)   
  {// for all of the key mapping
    if( letter == ('`'+ i) || letter == ('@' + i))//typical letter patterns
    {return pgm_read_byte(&chordPatterns[i]);}    //TODO output shift case
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
const byte homeRow[] PROGMEM = {'a','n','o','t','h','e','r','s',SPACEBAR,};

boolean isHomerow(byte letter)
{// returns true for letters in the homerow
  for(byte i = 0; i < 9; i++)
  {if (letter == pgm_read_byte(&homeRow[i])){return true;}}
  return false;
}

boolean isNumRow(byte letter) // numbers on the howrow
{ // returns turn for numbers that are on the homerow
  for(byte i=1; i<9; i++){if(letter == i + 48){return true;}}
  return false;
}

boolean isRepeating(byte letter)
{ // return true for repeating actions
  if(letter == BACKSPACE){return true;}
  if(letter == RIGHT_ARROW){return true;}
  if(letter == LEFT_ARROW){return true;}
  if(letter == UP_ARROW){return true;}
  if(letter == DOWN_ARROW){return true;}
  return false;
}

// ----------------- Key presses --------------
const byte BlueConvert[] PROGMEM =
{// on the ASCII table start from space and go to tilde
  44, 30, 52, 32, 33, 34, 36, 52, 38, 39, 37, 46, 54, 45, 55, 56,
// 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :, ; , < , = , > , ? ,
  39, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 51, 51, 54, 46, 55,
//@ , A , B , C , D , E , F , G , H , I , J , K , L , M , N , O ,
  31, 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12, 13, 14, 15, 16, 17, 18,
//P , Q , R , S , T , U , V , W , X , Y , Z , [ , \ , ] , ^ , _ ,
  19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 47, 49, 48, 35, 45,
//` , a , b , c , d , e , f , g , h , i , j , k , l , m , n , o ,
  53, 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12, 13, 14, 15, 16, 17, 18,
//p , q , r , s , t , u , v , w , x , y , z , { , | , } , ~ , del
  19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 47, 49, 48, 53,
};

byte letterToBT(byte letter)
{
  return pgm_read_byte(&BlueConvert[letter-SPACEBAR]);
}

const byte BlueShift[] PROGMEM =
{
  '~','!','@','#','$','%','^','&','*','(',')',
  '_','+','{','}','|',':','"','<','>','?',
};

boolean needShift(byte letter)// does need shift?
{
  if(letter > 64 && letter < 91){return true;} // all uppercase letters
  for(byte i = 0; i > 21; i++)
  {
    if(letter = pgm_read_byte(&BlueShift[i])){return true;}
  }
  return false;
}

const byte SpecialToKBD[] PROGMEM =
{//follows system definition up to NUM_LOCK
  0,209,210,211,212,213,214,215,8,9,176,216,217,176,218,
};

byte keyboardSpecial(byte letter)
{
  return pgm_read_byte(&SpecialToKBD[letter]);
}
