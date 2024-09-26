
#ifndef COIL_20_1364_H
#define COIL_20_1364_H



#include "target_sense.h"

// samples on the discharge curve
//
#define TIME_POINTS (10)

// The number of samples to average
//
#define SAMPLE_BUFFER_LENGTH (50)

// the greater the number, the less often the main loop updates the sound
//
#define SOUND_UPDATE_COUNT (4)


// greater value means slower recovery, longer averaging
//
#define LONG_AVERAGE_FACTOR (5.0)

// the number of samples we use when calculating a Median for target ID
// don't change this
//
#define ID_SET (21)


// The pulse period
// in multiples of 25uSec
// 133 = 300 Hz 
// 88 = 450 Hz
// 80 = 500 Hz
// 60 = 667 Hz
// 40 = 1kHz
const uint16_t V_PULSE_PERIOD = 88;


// The pulse width
// in multiples of 25uSec
// 3 = 75 uSec pulse width
// 9 = 225 uSec
// 10 = 250 uSec
//
#define V_PULSE_WIDTH ( 9 )

// The long sample delay
// When does the Long sample occur ? , timed from the start of the pulse
// in multiples of 25uSec
// 
// 40 = 1000 uSec after the START of the pulse
//
#define V_LONG_SAMPLE_DELAY (40)


int peakSampleCount = 0;  

// raw analog samples to be averaged 
// Buffer SAMPLE_BUFFER_LENGTH long, contains a set of samples at different sample delays 'TIME_POINTS',
//
float samples[TIME_POINTS][SAMPLE_BUFFER_LENGTH];


class COIL_20_1364
{

  public:  

  // return the value that produces the sound
  //
  double soundSignal();
  
  // calculated in soundSignal Fucntion
  //
  double soundSignalValue = 0;
  double soundSignalConductivity = 0;


  double conductivity = 0.0;
  double iron = 0.0;


  // called every sample average ~ 5 to 20 Hz
  //
  void targetSense();

  // Do the target discrimination algorithm for this particular coil
  //
  void targetDiscrimination();


  TARGETID targetID;
   
    
  // custom function that is called every sample
  // Performs averaging and signal processing using the raw sample set
  //
  void doSampleAveraging();

  float getIRONValue(float ratio);

  void debug();
  
  // For running averaging
  //
  double sums[TIME_POINTS] = {0.0};
  double sum_last = 0.0;

  double averages[TIME_POINTS] = {0.0};
  double longAverages[TIME_POINTS] = {0.0};

  double average_last = 0.0;
  double mediumAverage_last = 0.0;
  double longAverage_last = 0.0;

  double averageConduct[TIME_POINTS] =  {0.0};

  double normalised_average_curve[TIME_POINTS] = {0.0};
  double normalised_curve[TIME_POINTS] = {0.0};

  double signalCurve[TIME_POINTS] = {0.0};
  double peak_signal_curve[TIME_POINTS] = {0.0};

  double lockedAverage[TIME_POINTS] = {0.0};

  float lockedSignal[TIME_POINTS][ID_SET] = {0.0};
  
  int lockedSignalCount = 0;
 
  bool FLAG_TARGET_ID = false;
  int  timerTargetID = 0;
};


// use the early slope as an index  to look up a 'IRON' value
// this 11 point curve is used to interpolate
// if above the value that is looked up, then target is iron
// 
// Below setup for 20cm diameter coil with 430uH inductance tuned for particular coil characteristics
//
#define IRON_TABLE_SIZE (11)
float IRON_TABLE[IRON_TABLE_SIZE] = {\
  0.0,  /* 0 */\
  0.3,  /* 0.1 */\
  0.3,  /* 0.2 */\
  0.34,  /* 0.3 */\
  0.44, /* 0.4 */\
  0.57, /* 0.5 most critical area */\
  0.64, /* 0.6 most critical area */\
  0.76, /* 0.7 most critical area */\
  0.90, /* 0.8 */\
  1.00, /* 0.9 */\
  1.00  /* 1.0 */\
};

//-----------------------------------------------------------------------


 


#endif // COIL_20_1364_H

