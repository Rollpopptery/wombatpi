
/*
WOMBAT PI Metal detector
wombatpi.net
Modified: 31-Mar-2024

Communication over the serial port and WiFi

 --- Not used. ---

Commands in:
 
Data Out:  Sent out the Serial port


*/

// include the stdlib, so atoi function works
//
#include <stdlib.h>

#include "wombat.h"

#define MAX_COMMAND (10)
#define END_CHAR (13)

char rxBuffer[MAX_COMMAND];
int rxCount = 0;



// Returns 
// 0 if success
//-1 if failure

int setPulseWidth(int newPulseWidth)
{
 
  return(-1);
  
}

int sendPulseWidth()
{
  // not used
}



//--------------------------------------------------------------------------------------------
// Processing of all Serial / Wifi Commands
//
void checkCommands()
{
  char inChar = 0;
  bool commandIn = false;
  int tempValue = 0;

  // ensure quick return if no serial
  //
  if(Serial.available() <= 0)
  {
    return;
  } 
  
  
  while(Serial.available() > 0)
  {
    inChar = Serial.read();  
    rxBuffer[rxCount] = inChar;  

    // end of command character ?
    if(inChar == END_CHAR)
    {
      rxBuffer[rxCount] = 0;  // null terminated the buffer
      commandIn = true;
      break;
    }    
    else
    {
      rxCount++;
      if(rxCount >= MAX_COMMAND)
      {
        // avoid buffer overflow
        //
        rxCount = 0;
      }       
    }    
  }
  // read everything else and ignore
  //
  while(Serial.available() > 0)
  {
    Serial.read();
  } 
  rxCount = 0;

  // do we have a command ?
  //
  if(commandIn == true)
  {    
    // process the command
    //
    if(rxBuffer[0] == 'P')
    {           
      // set the pulse width
      //
      // value checking is in the setPulseWidth function
      //
      tempValue = atoi(&rxBuffer[1]);
      if(setPulseWidth(tempValue) == 0)
      {
        // success
        //
        sendPulseWidth();        
      }                   
     }
     else if(rxBuffer[0] == 's')
     {
        
     }
     else if(rxBuffer[0] == 'A')
     {
        // how many samples to Average in the single sample mode
        //
        
     }
     else if(rxBuffer[0] == 'M')
     {
      
      // Set Mode
      //
      switch(rxBuffer[1])
      {        
        case('S'):
        {      
          mode = S;
        }
        break;       
        case('a'):
        {      
          mode = a;
        }
        break;          
        default:
        {
          // do nothing
        }
        break;        
      }
    }    
  }  
}
