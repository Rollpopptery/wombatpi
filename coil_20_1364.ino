/*
---------------------------------------------------------------------
1364 uH coil

Diameter:   20cm
Inductance: 1364
Turns:      58
Wire:       0.5mm copper


Damping resistor ~ 

---------------------------------------------------------------------
*/

#include "coil_20_1364.h"

#include "voice.h"

// target sense calculation happens if above this threshold
//
#define THRESHOLD (20)



double COIL_20_1364::soundSignal()
{
  double tempF = 0.0;
  tempF += (averages[0]-longAverages[0]);  // 33uSEC   
  
 
  soundSignalValue = tempF;  

  tempF = averages[4]-longAverages[4];

  soundSignalConductivity = tempF / soundSignalValue;
   
  return(soundSignalValue);  // strongest response for particular coil 
}

//--------------------------------------------------------------------------------------
// called after each set of samples, i.e at ~ 10Hz
//
//--------------------------------------------------------------------------------------
void COIL_20_1364::targetSense()
{
  static double maxF = 0;
    
  double tempF = 0;  
  double S1 ;
  double S2 ;
   
  tempF = soundSignalValue;  


  
  if(FLAG_TARGET_ID)
  {
     timerTargetID++;

     // we need to have held over target for at least '10' counts

     if(timerTargetID < 10 )
     {
      
     }
     else
     {
      // we exit FLAG_TARGET_ID state when the signal falls below the locked average 
      // looking at the earliest time-sample only
      // i.e we moved off the target
      //
      if ((averages[0] - lockedAverage[0]) < 0)
      {
        timerTargetID = 0;
        FLAG_TARGET_ID = false;
      }

      
      // we are holding above the target and taking a long sample to get accurate ID
      // stay here until we drop below running average,  OR we time-out
      //

      for(int i = 0 ; i < TIME_POINTS; i++)
      {
        lockedSignal[i][lockedSignalCount] = (averages[i] - lockedAverage[i]);
      }
            
      lockedSignalCount++;   

      if(lockedSignalCount > (ID_SET-1))
      {
        // We held on target until we collected a complete set of samples
        // time to do target ID
        //
        targetDiscrimination();
       
        FLAG_TARGET_ID = false;
        
        // reset the locked signal
        //
        for(int i = 0; i < TIME_POINTS; i++)
        {
          lockedSignal[TIME_POINTS][0] = 0;
        }         
        lockedSignalCount = 0;    
      }
    }
  }
  // signal ?
  //                  
  if(tempF > THRESHOLD)                           // above our threshold ?
  {         
    //targetID = NO_TARGET; // reset
                       
    if(tempF > maxF)
    {
      maxF = tempF;
      // going up (signal is increasing) 
      // store the average signal (i.e our reference)
      //
      if ( !FLAG_TARGET_ID)  // not in ID mode yet
      {
        // Set the Target ID flag  'we are in target ID mode'
        //
        FLAG_TARGET_ID = true;
        for(int i = 0; i < TIME_POINTS; i++)
        {
            lockedAverage[i] = longAverages[i];          
        }
        lockedSignalCount = 0;  // new set of samples 
      //  Serial.println("*");
        // reset the targetID count
        // 
        timerTargetID = 0;  
      }      
    }            
  }
  else
  {
    if(tempF < 0)
    {   
      maxF = 0; // reset the peak      
    }      
  }  
}


void COIL_20_1364::targetDiscrimination()
{  
  double tempF;
  double tempF2;
  int tempIndex ;;

  lockedSignalCount = 0;
  
     for(int i = 0 ; i < TIME_POINTS; i++)
     {
       // calculate averages
       //
       tempF = 0;
       for(int i2 = 0 ; i2 < ID_SET; i2++)
       {

         tempF += lockedSignal[i][i2] ;
       }
       tempF /= ID_SET;
       signalCurve[i] = tempF;

       //Serial.println(lockedSignal[i]);
     }

    //normalise(signalCurve, TIME_POINTS, TIME_POINTS-1, 0);

    // this ratio is our conductivity (slope or speed of the discharge curve)
    //
    conductivity = signalCurve[4] / signalCurve[0] ;    
    
    tempF = signalCurve[7];
    tempF += signalCurve[8];
    tempF += signalCurve[9];

    tempF /= 3.0;

    iron = tempF  / signalCurve[4];
    
    // caluclate the IRON number using the IRON table
    // 
    // Is our actual value above or below this ?
    // If above, then is iron
    //
    tempF = getIRONValue(conductivity);

        
    Serial.print(conductivity);
    Serial.print(",");
    Serial.print(iron);    
    Serial.print(",");
    iron -= tempF;
    Serial.println(iron); // is it above (+iron) or below (-noniron) the line ?       
    
    /*
    Serial.print("expected iron:" );
   
    Serial.println(tempF);
    */
                         
    // Discrimination algorithm
    //
    TARGET_SENSE::targetID = OK_BIG;
    if(iron > 0)
    {
      if((conductivity < 0.77) && (conductivity > 0.50))
      {
        TARGET_SENSE::targetID = Fe;
      }
    }
      
    if(TARGET_SENSE::targetID == Fe )
    {
             
    }
    else
    {
      // non iron target, ring the Bell
      //
      queueSound(BELL);   
    }
    
    queueNumber(conductivity);

    startSounds();
            
      //Serial.println(TARGET_SENSE::targetID);     

      #ifdef USB_SERIAL_ENABLED
           //  Serial.print(sampleArray[SAMPLE_COUNT_MAX-1]);  // print the last raw reading for offset adjustment
           //  Serial.println();                
      #endif

}


// called repeatedly every sample, i.e at up to 500Hz,
// must be efficient
//
void COIL_20_1364::doSampleAveraging()
{
  static int averageCount = 0;  

  double tempF;
  int index;
  double oldSample;

  // reference sample
  //
  uint16_t lastSample = sampleArray[SAMPLE_COUNT_MAX-1]; 

  for(index = 0 ; index < TIME_POINTS; index++)
    {
      
      oldSample = samples[index][averageCount];      

      samples[index][averageCount] = sampleArray[( INDEX_30uSEC + index )];   // new raw sample from the set of samples
      //samples[index][averageCount] -= lastSample;

      sums[index] -= oldSample;    // subtract old value from the sum
      sums[index] += samples[index][averageCount];  // add the new value to the sum      
    }

    // recalculate the running averages
    //    
    for(index = 0; index < TIME_POINTS; index++)
    {
      averages[index] = (sums[index] / SAMPLE_BUFFER_LENGTH);    
    }
    
    averageCount++;   

    if(averageCount >= SAMPLE_BUFFER_LENGTH)
    {     
      // full set of samples complete
      //
      averageCount = 0;

      for(index = 0 ; index < TIME_POINTS; index++)
      {
        // re-calculate long averages
        //          
        tempF = (averages[index] -  longAverages[index]);
        tempF /= LONG_AVERAGE_FACTOR;          
        longAverages[index] += tempF;     

        // fast recovery, after the average has 'wound-up' due to being held on a target.
        // i.e The average follows the signal down quicker than it follows the signal up.
        //
        if(tempF < 0)
        { 
           // add it again
           //
           longAverages[index] += tempF;    
        }
      }     
    }
}



// use the ratio to look up a value in the iron table 
//
float COIL_20_1364::getIRONValue(float ratio)
{
  int index1 ;
  int index2 ;
  float tempF ;
  float v1;
  float v2;

  if(ratio >= 1)
  {
    return(0); // invalid
  }
  if (ratio < 0)
  {
    return(0); // invalid
  }

  // convert the value of 0.00 and 1.00 to an index between 0 and 10
  //
  tempF = ratio * 10;
  index1 = (int)tempF;
  index2 = index1 + 1;

  // lookup the two values in the table and interpolate between them
  // 
  v2 = IRON_TABLE[index2];
  v1 = IRON_TABLE[index1];

  v2 -= v1;
  tempF = tempF - (float)index1;  // the remainder
  tempF /- 10.0;
  tempF *= v2;
  tempF += v1;

  return(tempF);
}

void COIL_20_1364::debug()
{
  Serial.println(averages[3]-longAverages[3]);  
 

}
