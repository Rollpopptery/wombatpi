/*
--------------------------------------------------------------------
wombatpi.net

target_sense.ino


Target ID and discrimination

Modified 01-Jun-2024

--------------------------------------------------------------------
*/

#include "target_sense.h"
 

#define MIN_FLOAT (-32000.0)
#define MAX_FLOAT (32000.0)

// normalise the array to values 1.0 to 0.0
// Quickest, used on curves where the min and max are known.
//
void normalise(double arr[], int sz, int minIndex, int maxIndex)
{
  double min = arr[minIndex];
  double max = arr[maxIndex];
  double range;  

  // normalise
  //
  range = max-min;
  if(range == 0)
  {
    range = 1.0; // catch divide-by-zero
  }
  for(int i = 0 ; i < sz; i++)
  {
    arr[i] -= min;
    arr[i] /= range;    
  }
}


// normalise the array to values 1.0 to 0.0
// used on unsorted curves where the min and max are not obvious and unknown
// i.e the Target-shape curve
//
void normalise(double arr[], int sz)
{
  double min = MAX_FLOAT;
  double max = MIN_FLOAT;
  double range;

  // find max and min
  //
  for(int i = 0 ; i < sz; i++)
  {
    if(min > arr[i])
    {
      min = arr[i];
    }
    if (max < arr[i])
    {
      max = arr[i];
    }
  }

  // normalise
  //
  range = max-min;
  if(range == 0)
  {
    range = 1.0; // catch divide-by-zero
  }
  for(int i = 0 ; i < sz; i++)
  {
    arr[i] -= min;
    arr[i] /= range;    
  }
}


// Debugging utility
//
void printArray(double arr[], int sz, double multiplier)
{
  for(int i = 0; i < sz-1; i++)
  {
    Serial.println(arr[i] * multiplier);
   // Serial.print(",");
  }
  Serial.println(arr[sz-1] * multiplier);  
  Serial.println() ;
}



