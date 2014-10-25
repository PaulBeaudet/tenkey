/******* output.ino *******/

byte warningMessage[] = "recording";

void outputFilter(byte letter)// letter/mode
{
  static byte modeType = DEFAULT_MODE;
  
  switch(letter)
  {
    case 0:return; // return for 0 just in case of bad input
    case DEFAULT_MODE:  modeType = DEFAULT_MODE;  return;
    case NUMBERS_MODE:  modeType = NUMBERS_MODE;  return;
    case MOVEMENT_MODE: modeType = MOVEMENT_MODE; return;
  }
  
  switch(modeType)
  {
    case DEFAULT_MODE:  defaultMode(letter);  break;
    case NUMBERS_MODE:  numbersMode(letter);  break;
    case MOVEMENT_MODE: movementMode(letter); break;
  }
}

void enterBehavior(byte mode) // this function handles enter states
{ 
  static byte triggerType = 0;
  switch(mode)
  {
    case TRIGGER:
    switch(triggerType)
    {
      case 0: keyOut(CARIAGE_RETURN); break;
      case RECORD: 
        triggerType = 0; // set back to normal behavior
        recordHandlr(CAT_OUT); // finish the recording
        break; // record
      case 3: break;// command
    }
    break;
    case RECORD: triggerType = RECORD; break;// set record mode
    case 3: triggerType = 3; break;// set command mode 
  }
}

void movementMode(byte letter)
{
  switch(letter)
    {//
      //   detect homerow chars AKA unigrams
      case 'a':keyOut(KEY_LEFT_ARROW);break;//
      case 'n':break;// sticky Alt
      case 'o':break;// TODO sticky shift keyOut(KEY_LEFT_SHIFT);
      case 't':keyOut(KEY_DOWN_ARROW);break;// 
      case 'h':keyOut(KEY_UP_ARROW);break;// 
      case 'e':break;//  
      case 'r':break;// TAB
      case 's':keyOut(KEY_RIGHT_ARROW);break;//
      // detect bi and quad-gram situations 
      case 'b':keyOut('.');break;//decimal point
      case 'c':comboPress(KEY_LEFT_CTRL,'c', 0);break;//copy
      case 'd':keyOut('$');break;//dollar
      case 'f':keyOut(KEY_PAGE_UP);break;//page up
      case 'g':keyOut(KEY_PAGE_DOWN);break;//page down
      case 'i':keyOut(KEY_HOME);break;
      case 'j':keyOut('(');break;
      case 'k':keyOut(')');break;
      case 'l':keyOut(KEY_CAPS_LOCK);break;//lock
      case 'm':break;//!! note that this might activate comming into the mode
      case 'p':keyOut('%');break;//percent-modulo
      case 'q':keyOut(47) ;break;//Quotionent-devide
      case 'u':keyOut(KEY_END);break;
      case 'v':comboPress(KEY_LEFT_CTRL,'v',0);break;//paste
      case 'w':keyOut(TAB_KEY) ;break; //alt tab keys
      case 'x':comboPress(KEY_LEFT_CTRL,'x', 0);break;//cut
      case 'y':keyOut(',');break;//comma 
      case 'z':comboPress(KEY_LEFT_CTRL,'z', 0);break;//undo ctr - z  
      case TAB_KEY:keyOut('=');break;//hold space for equals
      default: defaultMode(letter);    
    }
}

void numbersMode(byte letter)
{
  switch(letter)
    {//
      //   detect homerow chars AKA unigrams
      case 'a':keyOut('1');break;//
      case 'n':keyOut('2');break;// 
      case 'o':keyOut('3');break;// 
      case 't':keyOut('4');break;// 
      case 'h':keyOut('5');break;// 
      case 'e':keyOut('6');break;// 
      case 'r':keyOut('7');break;// 
      case 's':keyOut('8');break;//
      // detect bi and quad-gram situations 
      case 'b':keyOut('.');break;//decimal point
      case 'c':keyOut('^');break;//carrot
      case 'd':keyOut('$');break;//dollar
      case 'f':keyOut('<');break;//less than
      case 'g':keyOut('>');break;//Greater than
      case 'i':keyOut('9');break;
      case 'j':keyOut('(');break;
      case 'k':keyOut(')');break;
      case 'l':keyOut('-');break;//subtract
      case 'L':keyOut('+');break;//plus
      case 'm':keyOut('*');break;//multiply
      case 'p':keyOut('%');break;//percent-modulo
      case 'q':keyOut(47) ;break;//Quotionent-devide
      case 'u':keyOut('0');break;
      case 'v':keyOut('"');break;//inches
      case 'w':keyOut(39) ;break;//feet
      //case 'x':keyOut('#');break;//pound XX variable case fall thru
      case 'y':keyOut(',');break;//comma 
      case 'z':keyOut('#');break;//pounnd  
      case TAB_KEY:keyOut('=');break;//hold space for equals
      default: defaultMode(letter);    
    }
}

void defaultMode(byte letter)
{
  switch(letter)//takes in key letter
  { // execute special compand basd on long hold
    case CARIAGE_RETURN: enterBehavior(TRIGGER); break;
    case 129:outputFilter(DEFAULT_MODE);break;//'a' set to default mode
    case 130: 
      if (recordHandlr(MONITOR_MODE)){break;}    // collision prevention
      messageHandlr(CAT_OUT); break;         //'b' print buffer
    case 131:comboPress(KEY_LEFT_CTRL,KEY_LEFT_ALT,0);break;//'c'
    //Change layout - command will vary computer to computer 
    case 132:break;                          //'d'
    case 133:break;                          //'e' Enter; confirm
    case 134:break;                          //'f'
    case 135:break;                          //'g' game
    case 136: alphaHint(); break;            //'h' hints alphabet
    case 137:potentiometer(ADJUST_PWM);break;//'i' pwm intensity
    case 138:break;	                         //'j'
    case 139:break;	                         //'k'
    case 140:break;	                         //'l'
    case 141:outputFilter(MOVEMENT_MODE); break; //'m' Movement Mode
    case 142:outputFilter(NUMBERS_MODE);break;//'n' Numbers Mode
    case 143:break;                          //'o'
    case 144:potentiometer(CHECK_VALUE);break;//'p'
    case 145:break;                          //'q'
    case 146:
      fastToast(warningMessage); 
      recordHandlr(TRIGGER); break;          //'r'
    case 147: potentiometer(ADJUST_TIMING);break; //'s' haptic display speed
    case 148:break;                          //'t' Transmit send cache
    case 149:break;                          //'u'
    case 150:break;                          //'v' varify 
    case 151:break;                          //'w'
    case 152:break;                          //'x'
    case 153:break;                          //'y'
    case 154:break;                          //'z'
    case 155:break;                          //'openbracket'
    case 156:break;                          //'pipe'
    case 157:break;	                         //'closebrack'
    case 158:break;	                         //'tilde'
    default: keyOut(letter);//send ascii given no exception
  }
}
