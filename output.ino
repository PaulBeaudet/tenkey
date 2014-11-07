//output.ino- Copyright Paul Beaudet 2014 -See License for reuse info
void outputFilter(byte letter)
{
  switch(letter)//takes in key letter
  { // execute special compand basd on long hold
    case 0: break;
    case 129:convertionMode(TRIGGER);break;//'a' toggle numbers mode
    case 130: 
      if (recordHandlr(MONITOR_MODE)){break;}    // collision prevention
      messageHandlr(RECORD_CAT); break;       //'b' tell cat to print buffer
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
    case 141:break;                          //'m' Movement Mode
    case 142:break;                          //'n' Numbers Mode
    case 143:break;                          //'o'
    case 144:potentiometer(DEFAULT_MODE);break;//'p'
    case 145:break;                          //'q'
    case 146://'r'---------------------------Record Mode
      recordHandlr(TRIGGER); //recording begins now
      break;          //'r'
    case 147: potentiometer(ADJUST_TIMING);break; //'s' haptic display speed
    case 148://'t'---------------------------Terminal Mode 
     terminalToggle(0); 
     break;  //'t' Terminal
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
