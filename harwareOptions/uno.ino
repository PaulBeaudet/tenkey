//uno.ino -- Copyright Paul Beaudet 2014 See license for reuse info
// dependancies specific to the uno
void serialInterfaceUp()
{
  Serial.begin(9600);// COM with Bluefruit EZ key HID or pyserial
}
//-------------Writing keys to host----------
void keyOut(byte keyPress)
{
  if(keyPress == CARIAGE_RETURN){keyPress = NEW_LINE;}//linux return call
  Serial.write(keyPress); // bluefruit or the uno or conection with pyserial
}

void comboPress(byte modifiers, byte key1, byte key2)
{
  if(needShift(key1)){modifiers=modifiers|LEFT_SHIFT;}//NOTE affects key2
  if(key1){key1 = letterToBT(key1);}
  if(key2){key2 = letterToBT(key2);} // in the rare case a second mod is needed
  Serial.write(0xFD); // our command: 0xfd
  Serial.write(modifiers); // modifier!
  Serial.write((byte)0); // 0x00
  Serial.write(key1); // key code #1
  Serial.write(key2); // key code #2
  Serial.write((byte)0); // key code #3
  Serial.write((byte)0); // key code #4
  Serial.write((byte)0); // key code #5
  Serial.write((byte)0); // key code #6
}

void releaseKey()
{
  Serial.write(0xFD); // our command: 0xfd
  Serial.write((byte)0); // modifier!
  Serial.write((byte)0); // 0x00
  Serial.write((byte)0); // key code #1
  Serial.write((byte)0); // key code #2
  Serial.write((byte)0); // key code #3
  Serial.write((byte)0); // key code #4
  Serial.write((byte)0); // key code #5
  Serial.write((byte)0); // key code #6
}

//spacers to overlook features outside of current 328p support
//EEPROM data locations
#define XMIN 0
#define XMAX 2
#define YMIN 4
#define YMAX 6 //was a load of crap
#define SESSION_REC 8
#define SESSION_KEY 66
void EEPROMsetup()//future mouse support
{/*
  if(EEPROM.read(SESSION_REC) == SESSION_KEY)
  {
    writeReading(512, XMIN);
    writeReading(512, XMAX);
    writeReading(512, YMIN);
    writeReading(512, YMAX);
    EEPROM.write(SESSION_REC, SESSION_KEY);//notify intial calibration occured
  }*/
}

boolean serialBowl(boolean null){return false;}
boolean terminalToggle(boolean null){return false;}
void mouseClick(byte click){;}
void mouseRelease(){;}
void mouseMovement(){;}
