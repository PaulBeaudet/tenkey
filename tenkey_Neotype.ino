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
* depends on an adafruit I2C multiplex driver
More detailed discription: arduinoMicropins.txt 
**********************************/
#include<avr/pgmspace.h>//explicitly stated read only memory

void setup()//setup the needed hardware  
{
  pagersUp();          // hardware.ino: brings vibrating motor interface up
  buttonUp();          // hardware.ino: brings button polling intreface up
  serialInterfaceUp(); // hardware.ino: brings serial output interface/s up
  delay(6600);         // wait for the yun to stop interfering.. suposedly
}

/************* MAIN *****************/
#define MONITOR_MODE      0
#define START_INTERUPT    1 // removes output, zeros play point: messageHandlr
#define CAT_OUT           2 // set message to play : messageHandlr
void loop() 
{
   byte pressState = chordLoop(buttonSample());//<--important part
   // captures the current state of the buttons
   listenForMessage();// grab potential messages over serial
   potentiometer(MONITOR_MODE);//monitor potentiometer for setting adjustment
   messageHandlr(pressState);
   //async message mangment - interupt with keystroke
}

/********** Main functions *************
*************************************/
byte chordLoop(int input) // takes sample of buttons: returns true for press
{// main progam loop is abstracted here, so it can be switch with other test
  byte actionableSample= patternToChar(input); //determine chord validity
  if(actionableSample){patternVibrate(input);} //actuate pagers:if letters
  //else{patternVibrate(0);}                     //release:turn pagers off
  byte filteredInput = inputFilter(actionableSample);
  if(filteredInput == 0){return 0;}            //invalid input case
  outputFilter(filteredInput); if(filteredInput==130){return 0;}//exception
  return 1;       //valid output case 
}//            debounce -> check hold -> return ASCII:letter or action code

// ----------------output interpertation-------------
#define CHECK_VALUE   1
#define ADJUST_PWM    2
#define ADJUST_TIMING 3

void outputFilter(byte letter)
{// long holds shift bytes up; the following switch covers special options
  switch(letter)//takes in key letter
  { // execute special compand basd on long hold
    case 0: return;//typical case; move on
    case 129:break; //'a'
    case 130: messageHandlr(CAT_OUT); break; //'b'
    case 131:break; //'c' copy; cache message
    case 132:break; //'d'
    case 133:break; //'e' Enter; confirm
    case 134:break; //'f'
    case 135:break; //'g' game
    case 136: alphaHint(); break;//'h' //hatically displays alphabet
    case 137: potentiometer(ADJUST_PWM); break;	//'i' pwm intensity
    case 138:break;	//'j'
    case 139:break;	//'k'
    case 140:break;	//'l'
    case 141:break; //'m' Message; cat cache
    case 142:break; //'n' nyan
    case 143:break; //'o'
    case 144: potentiometer(CHECK_VALUE);break; //'p'
    case 145:break; //'q'
    case 146:break; //'r'
    case 147: potentiometer(ADJUST_TIMING);break; //'s' haptic display speed
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
    default: keyOut(letter);//send ascii given no exception
  }
}
