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
  static boolean practiceMode = false;
  static boolean keyboardMode = true; //default output mode
  if(keyPress == 148){terminalMode = !terminalMode; return;}//t macro
  if(keyPress == 139){keyboardMode = !keyboardMode; return;}//k macro
  if(keyPress == 'g' + SPACEBAR){practiceMode = !practiceMode; return;}
  

  if(keyboardMode){Keyboard.write(keyboardConvert(keyPress));} //defualt op
  else{Serial.write(ttlConvert(keyPress));} //conection via pyserial or debug

  if(terminalMode){Serial1.write(ttlConvert(keyPress));}//Yun communication
  else if(practiceMode){practicePrompt(keyPress);}
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

#define REFRESH_TIME 21 //poll rate of pins
#define X_AXIS A2        //analog pin used for x
#define Y_AXIS A3        //analog pin used for y
#define RANGE 8

//EEPROM data locations
#define XMIN 0
#define XMAX 2
#define YMIN 4
#define YMAX 6 //was a load of crap
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
      if(abs(yMapped) != 1 && yMapped != 0 && abs(yMapped) != 2 && abs(yMapped) != 3)//if(abs(yMapped)>3){dostuff}
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
void bowlControl()
{//turning ASH on and off, in order to keep up with output
  if(Serial1.available() > 3){Serial1.write(XOFF);}
  else{Serial1.write(XON);} //resume output of ash 
}

void dumpThis(byte numberBytes)
{
  for(numberBytes; numberBytes == 0; numberBytes--)
  {
    Serial.write(Serial1.read());
    //delayMicroseconds(250);
    bowlControl();
  }
}

void serialDump() //dump whatever task was at hand
{
  Serial1.write(XON);
  while(Serial1.available() > 0)
  {
    Serial.write(Serial1.read());
    //delayMicroseconds(250);
  }
}

boolean serialBowl(boolean terminalToggle)
{ // keep the alphabits from overflowing
  static boolean terminalMode = false;
  static boolean printing = false;    //signal activity to outside loop
  static byte letterInWaiting = 0;

  if(terminalToggle)
  {
    if(terminalMode){serialDump();}
    terminalMode = !terminalMode;
    keyOut('t' + SPACEBAR); // also toggle terminal mode in keyOut routine
  }

  if(terminalMode)
  {
    if(!letterInWaiting){letterInWaiting = Serial1.read();}
    if(letterInWaiting == 255){letterInWaiting = 0;}

    if(hapticMessage(MONITOR_MODE))   //letter played or boot has occurred
    {
      if(letterInWaiting)
      {
        printing = true;               //prevents stop case
        hapticMessage(letterInWaiting);       //set incoming byte to be played
        Keyboard.write(letterInWaiting);      //show user char via Keyboard
        letterInWaiting = 0;
      }
      else{printing = false;}
    }
  }
  else{printing = false;}
  bowlControl(); //part that keeps the serial in the bowl
  return printing;
}

void practiceWord()
{

}

void practicePrompt(byte atemptedLetter)
{
  static byte letterInWaiting = 0;
  
  if(atemptedLetter == 0xff){letterInWaiting = 0;}
  else if(atemptedLetter == letterInWaiting)
  {
    delayMicroseconds(250);
    letterInWaiting = Serial1.read();
    bowlControl();
    if(letterInWaiting == 0xff){typingPractice();}
    else if(letterInWaiting == SPACEBAR){patternVibrate(240);}
    else if(letterInWaiting < 'A'){practicePrompt(letterInWaiting);} 
    else if(byte valid = charToPattern(letterInWaiting)){patternVibrate(valid);}
    else{practicePrompt(letterInWaiting);}
  }
}

void typingPractice()
{
  static boolean practiceMode = false;
  
  serialDump();           //remove and existing cruft
  if(practiceMode)        //exiting practice mode case
  {
    pagerActivity(0);     //set pager blocking inactive
    practicePrompt(0xff); //turn prompt off
  }
  else                    //entering practice mode case
  {
    pagerActivity(TRIGGER);
    Serial1.print("cat /mnt/sda1/arduino/alice.txt");
    serialDump();
    Serial1.write(NEW_LINE);
    practicePrompt(0); //call for first prompt
    //Serial1.write(XOFF);
    //dumpThis(31); //number of bytes that will be returned?
  }
  practiceMode = !practiceMode;
  keyOut('g' + SPACEBAR);
}


//--------- Performance testing functions ---------------

void pressTime(byte trigger)
{
  static unsigned long durration = 0;
  static byte letter = 0;
  
  if(trigger)
  {
    durration = millis();
    letter = trigger;
  }
  else
  {
    Serial.write(letter);
    Serial.print(F(" -pressed- "));
    Serial.println(millis() - durration);
    durration = 0;
  }
}
