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
int PWMintensity = 100; // Adjusts the intensity of the pwm
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
   pagerTesting();
}

/********** Main functions *************
*************************************/
void pagerTesting()
{
  //testing capibilities of the adafruit shield soon
  for(int i=0; i<8; i++)
  {// for every pager
    
  }
}

void chordLoop(int input)
{// mainloop is abstracted for testing purposes 
  byte actionableSample= patternToChar(input);// 0 parameter == reverse lookup
  //?? mask out thumb keys for vibration?
  if(actionableSample){patternVibrate(input, PWMintensity);}
  //fire the assosiated pagers! given action
  else{patternVibrate(0, 0);}//otherwise be sure the pagers are off
  outputFilter(inputFilter(actionableSample));
  //further filter input to "human intents" pass to output handler
}

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

byte chordPatterns[] 
{ // each byte is a series of bits that make up a chord
  1,5,48,56,2,24,33,6,4,14,28,12,40,30,7,18,31,3,16,32,51,45,8,35,54,49,
}; // array ordered as alphabet (a->1, b->5, ect)
#define PATTERNSIZE sizeof(chordPatterns)

byte patternToChar(int base) //returns the char value of a raw chord
{// some convertions can explicitly imediately be returned 
  if(base == L_THUMB){return BACKSPACE;} 
  // Backspace doubles as second level shift for special chars
  if(base == R_THUMB){return SPACEBAR;}
  // Space also doubles as the first shift in a chord
  if(base == 63){return CARIAGE_RETURN;}//combination: space + backspace
  // ?? (R_THUMB | L_THUMB) ??
  
  for (byte i=0; i<PATTERNSIZE; i++)   
  {// for all of the key mapping   
    if ( (base & 63) == chordPatterns[i] ) 
    {//patern match regardless most significant 2 bits 
    // 63 = 0011-1111 // mask the 6th and 7th bit out
      if ((base & 192) == 192){break;}
      //third level shift *combination holding space and backspace
      if (base & 64)//first level shift *combination with space
      {// 64 = 0100-0000 // if( 6th bit is fliped high )
        //if(lower shift, less than 10th result) {return corrisponding number}
        if(i<10) {return '0' + i;} //a-j cases (ascii numbers)
        if(i<25) {return 23 + i;}  //k-y cases ( !"#$%&'()*+'-./ )
        if(i==26){break;}          //z case (unassigned)
      } 
      if (base & 128)//second level shift *combination with backspace
      {//128 = 1000-0000 // if(7th bit is high) 
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
// ----------------input interpertation-------------

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
    case 144:break; //'p'
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

byte spacerTimer(byte reset)
{
    #define DELAYTIME 1 //the delay time corisponds to action values
    #define TIMESTARTED 0 // Denotes when each action starts
    #define SPACER 10 // ms
    static uint32_t timer[2] = {};// holds time started and delay time
    static byte progress=0; //keeps the progress of the actions 
    
    if(reset)
    {
    progress=0;//set everything back to the begining
        timer[DELAYTIME]=SPACER; //set the intial timing
    timer[TIMESTARTED]=millis();  // note the start time of the transition
    }
    else if(millis() - timer[TIMESTARTED] > timer[DELAYTIME])
    { 
    progress++;//increment the progress of the time table
    timer[DELAYTIME]=SPACER; //set durration baseded on progress level
    timer[TIMESTARTED]=millis();  // note the start time of the transition
    return progress; //return which level of progress has ellapsed
    }
    return 0;// in most cases called, time will yet to be ellapsed 
}

byte inputFilter(byte input)
{//debounces input: interprets hold states for capitilization + other functions
  static byte lastInput=0;//remembers last entry to debounce	
    
  if(input)//give something other than 0
  {//Given values and the fact values are the same as the last
    if(input == lastInput){return holdFilter(input);}
    if(lastInput == 0){spacerTimer(1);}//fall through; reset for regular press
  }
  lastInput=input; // hold the place of the current value for next loop
  return 0; // typical, no input case
}

/**************************
-HOLD FLOW-
1. debounce- accept valid input
2. Holdover- Remover char in preperation for upper cases
3. Capitilize- print upper case chare
4. Holdover- Remover char in preperation for special commands
5. Special Cases- Programed 'command' cases for special features
**************************/

byte holdFilter(byte input)
{// instantiate progress: set to current timing
if( byte progress = spacerTimer(0) )
  {// *V* hold logic *V*
    if(progress==2){return input;}//intial debounce
    if(input == BACKSPACE)                //Backspace case: before debounce 
    { // if holding more than X return when timeing is devisible by 3 or 12
      if(progress > 31 && progress % 3 == 0 || progress % 12 == 0)
      {return BACKSPACE;} 
      return 0; // terminate outside backspace cases
    }
    if(input == SPACEBAR)
    {// space cases
      if(progress == 40){return TAB_KEY;}//hold for tab case
      return 0; // terminate outside space cases
    }
    //---------------------8 or 32 terminate themselves   
    if(progress==40){ return BACKSPACE;}
    //delete currently printed char in preperation for a caps //holdover
    if(input < 91){return 0;}//in special char cases, go no further
    if(progress==60){return input-SPACEBAR;}
    //downshift subtract 32 to get caps
    if(progress==90){return BACKSPACE;}
    //delete currently printed char in preperation for a special commands
    if(progress==110){return input + SPACEBAR;}
    //upshift turns various input into commands
  }
return 0;//if the timer returns no action: typical case
}
