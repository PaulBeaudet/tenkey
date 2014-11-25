//leo.ino -- Copyright Paul Beaudet 2014 See license for reuse info

void serialInterfaceUp()
{
  Serial.begin(9600);// COM with Bluefruit EZ key HID or pyserial
  Keyboard.begin();//begin wired via usb keyboard
  Serial1.begin(9600);// possible communication with Bluefruit 
}

void keyOut(byte keyPress)
{
  if(keyPress == CARIAGE_RETURN){Keyboard.write(KEY_RETURN);} 
  else {Keyboard.write(keyPress);}
  if(keyPress == CARIAGE_RETURN){keyPress = NEW_LINE;}//linux return call
  if(terminalToggle(1)){Serial1.write(keyPress);}
  Serial.write(keyPress); // bluefruit or the uno or conection with pyserial
}

void comboPress(byte modifiers, byte key1, byte key2)
{//USB HID
  for(byte i = 0; i < 8; i++)//cycle through modifier cases
  {
    if(modifiers & (1<<i)){Keyboard.press(i+128);}
  }
  Keyboard.press(second);
  if(third){Keyboard.press(third);}
  Keyboard.releaseAll();
  // to pyserial
  if(needShift(key1)){modifiers=modifiers|LEFT_SHIFT;}//NOTE affects key2
  key1 = letterToBT(key1);
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

//spacers to overlook Yun features
boolean serialBowl(){return false;}
boolean terminalToggle(boolean null){return false;}
