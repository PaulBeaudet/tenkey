//hardware.ino
#include <Wire.h>
#include "Adafruit_PWM.h"

Adafruit_PWM pagers = Adafruit_PWM();

int PWMintensity = 1000; // Adjusts the intensity of the pwm

//----------PINOUT DEFINITIONS-------------------
byte buttons[] = { 10,9,7,8,5,4,6,3,2,11 };// pin out oppisite to pagers
// pins can be aligned in software: try to do it in hardware
#define NUMBUTTONS sizeof(buttons) // up to 16 possible
#define NUMPAGERS 8 // can use up to 16
//------------HARDWARE SETUP --------------------------
void pagersUp() // to speed up i2c, go into 'fast 400khz I2C' mode
{               // might not work when sharing the I2C bus
  pagers.begin();
  pagers.setPWMFreq(PWMintensity);  // This is the maximum PWM frequency
  uint8_t twbrbackup = TWBR;// save I2C bitrate
  // must be changed after calling Wire.begin() (inside pwm.begin())
  TWBR = 12; // upgrade to 400KHz!   
}

void buttonUp()// it's cold out there, set up the buttons 
{ //  set every button as an input with internal pull-up resistence
  for (byte set=0;set<NUMBUTTONS;set++){ pinMode(buttons[set], INPUT_PULLUP);}
}//pull-up -> 20k 5v rail| Pin-> button -> ground:reads low pressed

//-------------- actuating pagers---------------
void patternVibrate(int pins)
{ //set state of all pagers in one function
  for (byte i=0; i<NUMPAGERS; i++) 
  { // imagine incoming int as an array of 16 bits, one for each pager
    if (pins & (1 << i)) { pagers.setPWM( i, PWMintensity, 0); }
    else/*set pager off*/{ pagers.setPWM( i, 0, 0); }
  }
}
//---------------Sampling buttons-------------
int buttonSample()
{ // sample the keys and mask/combine them into an interger/"chord"
  int sample=0;//return value to be masked
  for (byte i=0; i<NUMBUTTONS; i++)  
  { // when button pressed              set selected bit in sample high
    if(digitalRead(buttons[i]) == LOW) {bitWrite(sample, i, 1);}   
    else                               {bitWrite(sample, i, 0);} 
  }//otherwise                          set the bit low
  return sample;
}// returning and int, allows 16 possible buttons states to be combined

void pagerTesting()
{
  //testing capibilities of the adafruit shield soon
  for (int i=0; i<4096; i += 8) 
  {
    for (byte pwmnum=0; pwmnum < 8; pwmnum++) 
    {
      pagers.setPWM(pwmnum, 10, (i + (4096/16)*pwmnum) % 4096 );
    }
  }
  for (int i=0; i<4096; i += 8) 
  {
    for (byte pwmnum=0; pwmnum < 8; pwmnum++) 
    {
      pagers.setPWM(pwmnum, 0, 0);
    }
  }
}
