//leo.ino -- Copyright Paul Beaudet 2014 See license for reuse info

void serialInterfaceUp()
{
  Serial.begin(9600); // COM with Bluefruit EZ key HID or pyserial
  Keyboard.begin();   //begin wired via usb keyboard
  Mouse.begin();      //begin wired mouse interactions
  Serial1.begin(9600);// possible communication with Bluefruit 
}

void keyOut(byte keyPress)
{
  static boolean keyboardMode = true; //default output mode
  if(keyPress == 139){keyboardMode = !keyboardMode; return;}//k macro

  if(keyboardMode){Keyboard.write(keyboardConvert(keyPress));} //defualt op
  else{Serial.write(ttlConvert(keyPress));} //conection via pyserial or debug
}

void releaseKey()
{
  Keyboard.releaseAll();
}

void comboPress(byte modifiers, byte key1, byte key2)
{//USB HID
  if(modifiers)
  {
    for(byte i = 0; i < 8; i++)//cycle through modifier cases
    {if(modifiers & (1<<i)){Keyboard.press(i+128);}}
  }
  if(key1){Keyboard.press(key1);}
  if(key2){Keyboard.press(key2);}
}

//spacers to overlook Yun features
boolean serialBowl(boolean null){return false;}
boolean terminalToggle(boolean null){return false;}

//mousey stuff
void mouseClick(byte click)
{
  if(click == LEFT_CLICK_IN){Mouse.press(LEFT_CLICK_OUT);}
  if(click == RIGHT_CLICK_IN){Mouse.press(RIGHT_CLICK_OUT);}
}

void mouseRelease()//be free critter!
{
  Mouse.release(LEFT_CLICK_OUT);
  Mouse.release(RIGHT_CLICK_OUT);
  Mouse.release(MIDDLE_CLICK_OUT);
}

//set x and y axis in pin_definitions.h
#define REFRESH_TIME 21 //poll rate of pins
#define RANGE 8

//EEPROM data locations
#define XMIN 0
#define XMAX 2
#define YMIN 4
#define YMAX 6
#define SESSION_REC 8
#define SESSION_KEY 61
void writeReading(int data, byte location)
{
  EEPROM.write(location, highByte(data));
  EEPROM.write(location + 1, lowByte(data));
}

void EEPROMsetup()
{
  if(EEPROM.read(SESSION_REC) != SESSION_KEY)
  {
    writeReading(512, XMIN);
    writeReading(512, XMAX);
    writeReading(512, YMIN);
    writeReading(512, YMAX);
    EEPROM.write(SESSION_REC, SESSION_KEY);//notify intial calibration occured
  }
}
void mouseMovement()
{
  static unsigned long time = 0;
  
  if(millis() - time > REFRESH_TIME)
  {//EEPROM the calibration data so it is centered every boot
    int xReading = analogRead(MOUSE_X_PIN);
    int yReading = analogRead(MOUSE_Y_PIN);
    int xmin = word(EEPROM.read(XMIN),EEPROM.read(XMIN +1));
    int xmax = word(EEPROM.read(XMAX),EEPROM.read(XMAX +1));
    int ymin = word(EEPROM.read(YMIN),EEPROM.read(YMIN +1));
    int ymax = word(EEPROM.read(YMAX),EEPROM.read(YMAX +1));
    
    if(xReading < xmin){writeReading(xReading, XMIN);} //4 directions of 
    else if(xReading > xmax){writeReading(xReading, XMAX);} //Calibration
    else if(yReading < ymin){writeReading(yReading, YMIN);}
    else if(yReading > ymax){writeReading(yReading, YMAX);}
    else //outside of calibration cases 
    {
      char xMapped = map(xReading, xmin, xmax, RANGE, -RANGE);
      char yMapped = map(yReading, ymin, ymax, -RANGE, RANGE);
      if(abs(yMapped) != 1 && yMapped != 0 &&
           abs(yMapped) != 2 && abs(yMapped) != 3)//if(abs(yMapped)>3){dostuff}
      {Mouse.move(0,yMapped,0);}
      if(abs(xMapped) != 1 && xMapped != 0 &&
           abs(xMapped) != 2 && abs(xMapped) != 3)
      {Mouse.move(xMapped,0,0);}
    }
    time = millis();
  }
}
