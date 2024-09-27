/*
----------------------------------------------------------------------------

WOMBAT PI Metal detector
Arduino UNO R4 Minima Version

wombatpi.net

Last Modified:  27-Sep-2024


----------------------------------------------------------------------------
*/

#define VERSION "27SEP2024"

#include "wombat.h"
#include "wombat_analog.h"
#include "target_sense.h"

#define BAUD_RATE (115200)

#define MAX_TX_BUFFER (30)
char txBuffer[MAX_TX_BUFFER];


void setup() 
{  

  Serial.begin(BAUD_RATE); 
  delay(200) ;  // charge the capacitor
  setupSample();
  setup_soundWave();    // Audio and pulsing    

  // Not used
  //
  if(WIFI_SERIAL_ENABLED)
  {
    Serial1.begin(BAUD_RATE);
  }
}



void loop() 
{  
  static int soundUpdateCount = -4000; // this counts up, and provides an initial delay before outputting sound
  
  static double sum = 0.0;
  static int serialCheckCount = 0;   // check the serial port periodically
  static int printOutCount = 0;
  static int newAverageCounter;
  static double maxF;
  static int medianCounter = 0;
  static int loopCounter = 0;   

  int oldSample; 
  int index = 0;  
  double tempF ; 
  double signal;    
  
  if(sampleReady)
  {
    sampleReady = false;      
     
    theCoil.doSampleAveraging();         
    

    // Do our Discrimination and Target ID here if it's time
    //    
    // i.e If 600 Hz pulse rate, with 50-sample buffer, we will do this 600/50 = 12 times per second) 
    //
    if (printOutCount++  > SAMPLE_BUFFER_LENGTH)
    {  
      printOutCount = 0;    
      theCoil.targetSense(); 
      //theCoil.debug();     
    } 

    // start off as large negative number and we don't need a separate startup counter
    //    
    if (soundUpdateCount++ > SOUND_UPDATE_COUNT)
    {
      soundUpdateCount = 0;
      //do the audio update 
      //        
      tempF = theCoil.soundSignal();
      soundAlgorithm3(theCoil.soundSignalValue, theCoil.soundSignalConductivity);
    }   
  }
} 

