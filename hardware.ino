//hardware.ino

#include <Wire.h>
#include "Adafruit_PWM.h"

Adafruit_PWM pagers = Adafruit_PWM();

//----------PINOUT DEFINITIONS-------------------
byte buttons[] = { 5,4,6,9,7,8,10,3,2,11, };
// !! ---set the desired button pins here--- !!
#define NUMBUTTONS sizeof(buttons)
#define NUMPAGERS 8 // can use up to 16
//------------HARDWARE SETUP --------------------------

void pagersUp() // to speed up i2c, go into 'fast 400khz I2C' mode
{               // might not work when sharing the I2C bus
  pagers.begin();
  pagers.setPWMFreq(1600);  // This is the maximum PWM frequency
  uint8_t twbrbackup = TWBR;// save I2C bitrate
  // must be changed after calling Wire.begin() (inside pwm.begin())
  TWBR = 12; // upgrade to 400KHz!   
}

void buttonUp()// it's cold out there
{ 
  for (byte set=0;set<NUMBUTTONS;set++)
  { //set up the buttons 
    pinMode(buttons[set], INPUT_PULLUP);
    //sets pull-up resistor/ie input through 20k to 5v
  }//Pin-> button -> ground: will read low when pressed
}

//-------------- actuating pagers---------------

void patternVibrate(byte pins, byte pwm)//
{
  for (byte i=0; i<NUMPAGERS; i++) 
  { // imagine incoming byte as an array of 8 bits, one for each pager
    if (pins & (1 << i)) { pagers.setPWM( i, pwm, 0); }
    else/*set pager off*/{ pagers.setPWM( i, 0, pwm);   }
  }
}

//---------------Sampling buttons-------------
byte chordSample()
{ // sample the keyes assigned to chords
  byte sample=0;
  for (byte i=0; i<8; i++)  
  {
    if(digitalRead(buttons[i]) == LOW) {bitWrite(sample, i, 1);}
    // set the selected bit high !!
    else                               {bitWrite(sample, i, 0);}
     // set the selected bit low !!  
  }
  return sample;
}

byte thumbSample()
{ // sample the thumb keys
  byte sample=0;
  for (byte i=8; i<10; i++)  
  {
    byte bitPlace = i - 8;
    if(digitalRead(buttons[i]) == LOW) {bitWrite(sample, bitPlace, 1);}
    // set the selected bit high !!
    else                               {bitWrite(sample, bitPlace, 0);}
     // set the selected bit low !!  
  }
  return sample;
}
