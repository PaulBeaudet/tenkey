//timing.ino -- Copyright Paul Beaudet 2014 See license for reuse info
byte spacerTimer(boolean reset)
{
  #define SPACER 10 // ms
  static uint32_t timer;// holds time started and delay time
  static byte progress=0; //keeps the progress of the actions

  if(reset)
  {
    progress=0;        // set everything back to the begining
    timer = millis();  // note the start time of the transition
  }
  else if(millis() - timer > SPACER)
  {
    progress++;       // increment the progress of the time table
    timer = millis(); // reset for the next stop
    return progress;  // return which level of progress has ellapsed
  }
  return 0;// in most cases called, time will yet to be ellapsed
}

boolean ptimeCheck(uint32_t durration)
{                                 // used for checking and setting timer
  static uint32_t ptimer[2]={1,0};// create timer to modify default check=true
  if(durration)                   // given param other than zero
  {                               // time is being set
    ptimer[1]=durration;          // set durration
    ptimer[0]=millis();           // note the time set
  }                               // if the durration has elapsed return true
  else if(millis() - ptimer[0] > ptimer[1]){return true;}//time has passed  
  return false;                   //time has yet to pass
}
