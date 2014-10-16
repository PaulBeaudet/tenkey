/*****Timing Functions*********
*******************************/

byte spacerTimer(byte reset)
{
    #define DELAYTIME 1 //the delay time corisponds to action values
    #define TIMESTARTED 0 // Denotes when each action starts
    #define SPACER 10 // ms
    static uint32_t timer[2] = {};// holds time started and delay time
    static byte progress=0; //keeps the progress of the actions 
    
    if(reset)
    {
      progress=0;//set everything back to the begining
      timer[DELAYTIME]=SPACER; //set the intial timing
      timer[TIMESTARTED]=millis();  // note the start time of the transition
    }
    else if(millis() - timer[TIMESTARTED] > timer[DELAYTIME])
    { 
      progress++;//increment the progress of the time table
      timer[DELAYTIME]=SPACER; //set durration baseded on progress level
      timer[TIMESTARTED]=millis();  // note the start time of the transition
      return progress; //return which level of progress has ellapsed
    }
    return 0;// in most cases called, time will yet to be ellapsed 
}