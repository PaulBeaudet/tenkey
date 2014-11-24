//yun.ino -- Copyright Paul Beaudet 2014 See license for reuse info
// dependancies specific to the yun
#define YUN_BOOT_OUTPUT true // mark true to see yun boot msgs on serial

void serialInterfaceUp()
{
  Serial.begin(9600);// COM with Bluefruit EZ key HID or pyserial
  Keyboard.begin();//begin wired via usb keyboard
  Serial1.begin(250000); // begin communication with dd-wrt ash terminal
  // make sure linux has booted and shutdown bridge
  bootCheck(); // returns true for boot, full boot takes about 60sec
  bridgeShutdown();// with bridge shutdown serial1 acts as raw shell access
  Serial.write('>');//ready signal
}
//-------------Writing keys to host----------
void keyOut(byte keyPress)
{
  if(keyPress < FUNC_F1){Keyboard.write(keyboardSpecial(keyPress));} 
  else {Keyboard.write(keyPress);}
  if(keyPress == CARIAGE_RETURN){keyPress = NEW_LINE;}//linux return call
  if(terminalToggle(1)){Serial1.write(keyPress);}
  Serial.write(keyPress); // bluefruit or the uno or conection with pyserial
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
  Keyboard.releaseAll();
  // to pyserial
  if(needShift(key1)){modifiers=modifiers|LEFT_SHIFT;}//NOTE affects key2
  key1 = letterToBT(key1);
  if(key2){key2 = letterToBT(key2);} // in the rare case a second mod is needed
  //key sequence for EZ-Key and Host application
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

//---------- YUN specific --------------
#define XON            17 // control_Q resume terminal output
#define XOFF           19 // control_S stop terminal output

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
    if(inputFilter(patternToChar(buttonSample())) == 's' && !booting)//esc
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

boolean serialBowl()
{ // keep the alphabits from overflowing
  static boolean printing = false;    //signal activity to outside loop
  if(hapticMessage(MONITOR_MODE))   //letter played or boot has occurred
  {
    byte incoming = Serial1.read(); //read off a byte regardless
    if (incoming == 255){printing = false;}  //255 = -1 in byte land
    else if (incoming && terminalToggle(1))
    {
      printing = true;               //prevents stop case
      hapticMessage(incoming);       //set incoming byte to be played
      Keyboard.write(incoming);
      //Serial.write(incoming);        
    }
  }
  if(Serial1.available() > 3){Serial1.write(XOFF);}//turn off ash to keep up
  else{Serial1.write(XON);} //resume output of ash
  return printing;
}

boolean terminalToggle(boolean returnVar)
{
  static boolean terminalMode = false;
  if(returnVar){return terminalMode;} //preclude toggle
  terminalMode = !terminalMode;//terminal mode possible on yun
}
