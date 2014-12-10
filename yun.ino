//yun.ino -- Copyright Paul Beaudet 2014 See license for reuse info
// dependancies specific to the yun
#define YUN_BOOT_OUTPUT true // mark true to see yun boot msgs on serial

void serialInterfaceUp()
{
  Serial.begin(9600);// COM with Bluefruit EZ key HID or pyserial
  Keyboard.begin();//begin wired via usb keyboard
  Mouse.begin();   //begin wired mouse interactions
  Serial1.begin(250000); // begin communication with dd-wrt ash terminal
  // make sure linux has booted and shutdown bridge
  bootCheck(); // returns true for boot, full boot takes about 60sec
  bridgeShutdown();// with bridge shutdown serial1 acts as raw shell access
  Serial.write('>');//ready signal
}
//-------------Writing keys to host----------
void keyOut(byte keyPress)
{
  static boolean terminalMode = false;
  static boolean keyboardMode = true; //default output mode
  if(keyPress == 't' + SPACEBAR){terminalMode = !terminalMode; return;}
  if(keyPress == 'k' + SPACEBAR){keyboardMode = !keyboardMode; return;}

  if(keyboardMode){Keyboard.write(keyboardConvert(keyPress));} //defualt op
  else{Serial.write(ttlConvert(keyPress));} //conection via pyserial or debug

  if(terminalMode){Serial1.write(ttlConvert(keyPress));}//Yun communication
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

#define REFRESH_TIME 20 //poll rate of pins
#define X_AXIS A2        //analog pin used for x
#define Y_AXIS A3        //analog pin used for y
#define RANGE 12

//EEPROM data locations
#define XMIN 0
#define XMAX 2
#define YMIN 4
#define YMAX 6 //was a load of crap
#define SESSION_REC 8
#define SESSION_KEY 66
void writeReading(int data, byte location)
{
  EEPROM.write(location, highByte(data));
  EEPROM.write(location + 1, lowByte(data));
}

void EEPROMsetup()
{
  if(EEPROM.read(SESSION_REC) == SESSION_KEY)
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
    int xReading = analogRead(X_AXIS);
    int yReading = analogRead(Y_AXIS);
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
      if(abs(yMapped) != 1 && yMapped != 0 && abs(yMapped) != 2 && abs(yMapped) != 3)
      {Mouse.move(0,yMapped,0);}
      if(abs(xMapped) != 1 && xMapped != 0 && abs(xMapped) != 2 && abs(xMapped) != 3)
      {Mouse.move(xMapped,0,0);}
    }
    time = millis();
  }
}
//---------- YUN specific --------------
void bridgeShutdown()
{
  Serial1.write((uint8_t *)"\xff\0\0\x05XXXXX\x7f\xf9", 11);//shutdown bridge
  Serial1.println();//send a new line character to enter shutdown garbage
  delay(2);// wait for the buffer to fill with garbage
  while(Serial1.available()){Serial1.read();} // read out shutdown garbage
}

void bootHandler(boolean startUpOutput)//pass true for verbose output
{ //mirror boot process to the serial monitor if true argument is passed
  if(startUpOutput){Serial.write(Serial1.read());} 
  else{Serial1.read();}//empty buffer with empty reads
}

boolean bootCheck()
{
  boolean booting = false;//assume complete boot
  ptimeCheck(17800);      //set timer for max distance between boot outputs
  // have recorded +16 sec between some outputs: important if reset midboot
  while(!ptimeCheck(0))   //before the timer is up
  {
    while(Serial1.available())
    {
      bootHandler(YUN_BOOT_OUTPUT);
      booting = true;    //buffer filled before user interaction was possible
    }
    buttonUpdate();
    if(patternToChar(trueChord(MONITOR_MODE)) == 's' && !booting)//esc
    {ptimeCheck(1); break;}//prep timer for possible imediatete rec case.
  }                      // timer returns true when finished exiting loop
  if (booting)
  {
    ptimeCheck(50000);   //give enough time to finish booting
    while(!ptimeCheck(0))//before time is finished     
    {
      while(Serial1.available()){bootHandler(YUN_BOOT_OUTPUT);}
    }                    //handle rest of output
  }
  return booting;        //in case of conditions looking for boot
}

#define XON            17 // control_Q resume terminal output
#define XOFF           19 // control_S stop terminal output
boolean serialBowl(boolean terminalToggle)
{ // keep the alphabits from overflowing
  static boolean terminalMode = false;
  static boolean printing = false;    //signal activity to outside loop
  static byte letterInWaiting = 0;

  if(terminalToggle)
  {
    terminalMode = !terminalMode;
    keyOut('t' + SPACEBAR); // also toggle terminal mode in keyOut routine
  }

  if(terminalMode)
  {
    if(!letterInWaiting){letterInWaiting = Serial1.read();}
    if(letterInWaiting == 255){printing = false; letterInWaiting = 0;}
    else if(hapticMessage(MONITOR_MODE))   //letter played or boot has occurred
    {
      if(letterInWaiting)
      {
        printing = true;               //prevents stop case
        hapticMessage(letterInWaiting);       //set incoming byte to be played
        Keyboard.write(letterInWaiting);      //show user char via Keyboard
        letterInWaiting = 0;
      }
    }
  }
  //turning ASH on and off, in order to keep up with output
  if(Serial1.available() > 3){Serial1.write(XOFF);}
  else{Serial1.write(XON);} //resume output of ash
  return printing;
}
