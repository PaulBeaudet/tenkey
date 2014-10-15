/********tenkey_Neotype.ino*********
* Copyright Paul Beaudet Created Fall 2014 <inof8or@gmail.com>
* Keyer that recieves and transmits text information from its user
* This program is free open source software: See licenses.txt to understand 
  reuse rights of any file associated with this project
-------------------------------------------------------------------
* Hardware discription: Arduino(atmega32u4) or uno(w/bluefruit EZKey)
* Adafruit_PWMServoDriver shield drives 8 pager moters
* ten keys take a input, 8 of which register chords
* 8 Pagers sit on top of the chord keys to be able to feed back letters
More detailed discription: arduinoMicropins.txt 
**********************************/
//depends on an I2C multiplex driver 
int HAPTICTIMING = 1400; //ms, haptic display durration, Future; user adjustable
#define SERIALINTERFACE Serial // change depending on board

/********Set-up outline ***********
pagersUp() brings vibrating motor interface online
buttonUp() brings button polling intreface up
*********************************/
void setup() 
{
  pagersUp(); // hardware.ino
  buttonUp(); // hardware.ino
  SERIALINTERFACE.begin(9600);//start communication with bluefruit 
}

/********* Main Loop outline***********

*************************************/
void loop() 
{
   chordLoop(buttonSample());
   adjustPWM();//checks a potentiometer to adjust pwm
   //if(buttonSample()){pagerTesting();} 
   //alignTool();
   //releaseDebounce(buttonSample()); 
}

/********** Main functions *************
*************************************/

void chordLoop(int input)
{// main progam loop is abstracted here, so it can be switch with other test
  byte actionableSample= patternToChar(input); //determine chord validity
  if(actionableSample){patternVibrate(input);} //actuate pagers:if letters
  else{patternVibrate(0);}                     //release:turn pagers off
  outputFilter(inputFilter(actionableSample)); //final output interpertation
}//            debounce -> check hold -> return ASCII:letter or action code

/********* Conversion Program Flow *********************************
buttonSample() get the state of the buttons: Depends on: hardware.h
returns and byte where each bit is a button

patternToChar() turn the button sample into a represented character
  chordPatterns 
  
patternVibrate() drive pagers, pass a byte where each bit is a motor

* Important note: Valid letter data is debounced, not individual keys
inputFilter() debounce cumalitive input and detect holds
  holdFilter()
    
outputFilter() 

********ASCII KEY******** Letter debouncing and convertion *************/
#define BACKSPACE 8 // output keys
#define TAB_KEY 9
#define SPACEBAR 32
#define CARIAGE_RETURN 13
#define LETTER_A 97
#define LETTER_Z 122
#define L_THUMB 256 // input data
#define R_THUMB 512

//   --ANOTHERS-- layout a-n-o-t-h-e-r-s make up the homerow 
byte chordPatterns[] // each byte is a series of bits that make up a chord
{ //a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,
  128, 10, 65, 80,  4,  6, 96,  8,160,192,  5, 24,153,
//  n,  o,  p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z, 
   64, 32, 12, 48,  2,  1, 16,132,129, 66, 90, 36, 60,
}; // array ordered as alphabet (a->1, b->5, ect)
#define PATTERNSIZE sizeof(chordPatterns)

byte patternToChar(int base) //returns the char value of a raw chord
{// some convertions can explicitly imediately be returned 
  if(base == L_THUMB){return BACKSPACE;}//also:2nd level shift, special chars
  if(base == R_THUMB){return SPACEBAR;}//also doubles: first shift in a chord
  if(base == (R_THUMB | L_THUMB)){return CARIAGE_RETURN;}
  //combination: space + backspace == Enter
  
  for (byte i=0; i<PATTERNSIZE; i++)   
  {// for all asignments in key mapping   
    if ( (base & ~(R_THUMB | L_THUMB)) == chordPatterns[i] ) 
    {//incoming chord ignoring thumbs     check for matching chord 
      if (base & R_THUMB)//first level shift *combination with space
      {// 512 = 10-0000-0000 // if( 10th bit is fliped high )
        //if(lower shift, less than 10th result) {return corrisponding number}
        if(i<10) {return '0' + i;} //a-j cases (ascii numbers)
        if(i<25) {return 23 + i;}  //k-y cases ( !"#$%&'()*+'-./ )
        if(i==26){break;}          //z case (unassigned)
      } 
      if (base & L_THUMB)//second level shift *combination with backspace
      {// 256 = 01-0000-0000 // if(9th bit is high) 
        if(i<7){return ':' + i;}  //a-g cases ( :;<=>?@ )
        if(i<13){return 84 + i;}  //h-m cases ( [\]^_`  )
        if(i<17){return 110 + i;} //n-q cases( {|}~    ) 
        break;                    //other casses unassigned
      }
      return 'a' + i;
    }// return plain char based on possition in the array given no shift
  }
  return 0;
}

byte charToPattern(byte letter)
{
  if(letter == SPACEBAR){return 64;}//Express convertion: Space 
  // Space also doubles as the first shift in a chord
  for (byte i=0; i<PATTERNSIZE; i++)   
  {// for all of the key mapping
    if ( letter == ('a'+ i) ){return chordPatterns[i];}
    //return typicall letter patterns
    if ( letter < 58 && letter == ('0' + i) ) 
      {return chordPatterns[i] | 64;} 
    // in numbers shift case return pattern with 6th bit shift
    if ( letter > 32 && letter < 48 && letter == (23 + i) ) 
      {return chordPatterns[i] | 64;}
    //k-y cases ( !"#$%&'()*+'-./ )return 6th bit shift
    if ( letter < 65 && letter == (':' + i) ) {return chordPatterns[i] | 128;}
    //a-g cases  (:;<=>?@ ), return 7th bit shift
    if ( letter > 90 && letter < 97 && letter == (84 + i) ) 
      {return chordPatterns[i] | 128;}
      // h-m cases  ([\]^_`  ), return 7th bit shift
    if ( letter > 122 && letter < 127 && letter == (110 + i) ) 
    {return chordPatterns[i] | 128;}//n-q cases( {|}~   )return 7th bit shift
  }
  return 0;
}
// ----------------output interpertation-------------

void outputFilter(byte letter)
{// long holds shift bytes up; the following switch covers special options
  switch(letter)//takes in key letter
  { // execute special compand basd on long hold
    case 0: return;//typical case; move on
    case 129:break; //'a'
    case 130:break; //'b'
    case 131:break; //'c' copy; cache message
    case 132:break; //'d'
    case 133:break; //'e' Enter; confirm
    case 134:break; //'f'
    case 135:break; //'g' game
    case 136:hapticAlpha();break;//'h' //hatically displays alphabet
    case 137:break;	//'i' pwm intensity
    case 138:break;	//'j'
    case 139:break;	//'k'
    case 140:break;	//'l'
    case 141:break; //'m' Message; cat cache
    case 142:break; //'n' nyan
    case 143:break; //'o'
    case 144:potCheck();break; //'p'
    case 145:break; //'q'
    case 146:break; //'r'
    case 147:break; //'s' haptic display speed
    case 148:break; //'t' Transmit send cache
    case 149:break; //'u'
    case 150:break; //'v' varify 
    case 151:break; //'w'
    case 152:break; //'x'
    case 153:break; //'y'
    case 154:break; //'z'
    case 155:break; //'openbracket'
    case 156:break; //'pipe'
    case 157:break;	//'closebrack'
    case 158:break;	//'tilde'
    default: SERIALINTERFACE.write(letter);//send ascii given no exception
  }
}
