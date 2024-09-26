/*
wombat_dac_audio

wombatpi.net

Modified:  09-June-2024

This code is "AS IS" without warranty or liability. 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.


Modifications:

09-June-2024  RVW  signed int for sample array so subtract of long sample works even if long sample is greater 
11-June-2024  RVW  'Pop' prevention on wav sounds by using ramp up or down to DAC sample
16-June-2024  RVW  2 digit conductivity. Non Iron Bell sound
06-July-2024  RVW  soundAlgorithm3() added
 
 
*/
 


/*

We use Triangle waves; They sound good and are simple to make

*/

#include <FspTimer.h>

#include "target_sense.h"
#include "R4_defines.h"

#include "voice.h"

FspTimer soundWave;

double volume;
extern double midTone;  // Hz

class TRIANGLEWAVE {

  private:
    float speed;       // added or subtracted every interrupt, calculated using frequency
    float amplitude;   // value between 0 and 1
    float volume = 0.5;    // value between 0 and 1
    int goingup = 1;

    const double HALF_CLOCK_FREQUENCY = 10000.0;  // the interrupt frequency / 2
    uint16_t MIDDLE_VOLTAGE = 2048;  // an offset so signal is an AC signal
    const float DAC_MULTIPLIER = 2000;   // this is max value of the DAC at volume = 1.0, i.e the peak to peak ~ 2  Volts
      
  public: 
        
    uint16_t  INT_amplitude; 
    void setVolume(double v)
    {      
      if(v > 1.0)
      {
        volume = 1.0;
      }
      else if( v < 0)
      {
        volume = 0;
      }
      else
      {
        volume = v;
      }      
    }

    void setFrequency(double f)
    {
      if(f > 3000)
      {
        f = 3000;  // maximum 3 kHz
      }
      speed = (f / HALF_CLOCK_FREQUENCY);  // the value added to the amplitude every interrupt      
    }

    // called every interrupt very quickly, i.e at 20kHz !
    //
    void advance()
    {        
      amplitude += (goingup * speed);
      if(amplitude > 1)
      {
        goingup = -1;   // going down, i.e down slope of triangle
        amplitude = 1;  // not ideal
      }
      else if (amplitude < 0)
      {
        amplitude = 0; // not ideal
        goingup = 1;  // going up, i.e up slope of the triangle
      }
      INT_amplitude = MIDDLE_VOLTAGE;
      INT_amplitude += (uint16_t)(amplitude * volume * DAC_MULTIPLIER);      
    }
};


TRIANGLEWAVE wave1;


volatile uint8_t wavDivider = 0;
volatile uint8_t sampleDivider = 0;
volatile uint16_t pulse_counter = 0;


// number of samples to take in sample-window
//
volatile int sampleCounter = 0;
volatile int16_t sampleArray[SAMPLE_COUNT_MAX];
volatile int16_t longSample;

//----------------------------------------------------------------------------------------------
// 40kHz
// This gives an interrupt period of 25uSec
// Because the interrupt system is complex, and the sound interrupt
// will always clash with the PWM and pulse-width interrupt if they are independent...
// We do everything (Pulse, Sampling and Sound) from this single 40kHz interrupt
//
//----------------------------------------------------------------------------------------------
void timer_DACOut_Interrupt(timer_callback_args_t __attribute((unused)) *p_args)
{
  float tempF;
  
  pulse_counter++;
  
  if (pulse_counter == V_LONG_SAMPLE_DELAY)    
  {
     noInterrupts();
    // take a single sample ~1 mSec  after our other samples
    // and subtract it from all our other samples
    //
    //*PFS_P107PFS_BY = 0x05;         // digitalWrite(monitorPinD7, HIGH)  
    longSample = 0;
    for (sampleCounter = 0 ; sampleCounter < 4; sampleCounter++)
    {
    *ADC140_ADCSR |= (0x01 << 15);  // Next ADC conversion = write to register c. 300nS
    while((*ADC140_ADCSR &= (0x01 << 15) ) != 0x0000);  // if things not setup right, endless loop    

    longSample +=  *ADC140_ADDR01;  
    }    
    interrupts();

    // divide by 4 (average of 4 samples)
    //
    longSample >>= 2; 

    // subtract it from all our samples
    //
    for(sampleCounter = 0 ; sampleCounter < SAMPLE_COUNT_MAX; sampleCounter++)
    {       
      sampleArray[sampleCounter] -= longSample;
    }
    
    // The samples are now ready for processing elsewhere
    //
    sampleReady = true;
  }

  if (pulse_counter == V_PULSE_WIDTH)
  {
    noInterrupts();

    // stop the pulse
    //
    *PFS_P104PFS_BY = 0x04;      // Pulse off on D3 

    // do the sampling
    //          
    for(sampleCounter = 0 ; sampleCounter < SAMPLE_COUNT_MAX; sampleCounter++)
    {       
      *ADC140_ADCSR |= (0x01 << 15);  // Next ADC conversion = write to register c. 300nS
      while((*ADC140_ADCSR &= (0x01 << 15) ) != 0x0000);  // if things not setup right, endless loop    
      sampleArray[sampleCounter] = *ADC140_ADDR01;              
    } 
    interrupts();     

  }

  if(pulse_counter == V_PULSE_PERIOD)
  {    
    // start the pulse
    //
    pulse_counter = 0;
    *PFS_P104PFS_BY = 0x05;      // Pulse on D3 
  }
  

  // every second time, we do the audio
  // (ie at 20kHz)
  //
  wavDivider ^= 1;
  if(wavDivider == 0)
  {    
    if (VOICE_ACTIVE)
    { 
      voiceSample = soundPointer[voiceSampleIndex]; 
      voiceSample <<= 3;

      // prevent pop by ramping DAC to first sample
      //
      if(VOICE_LEADIN)
      {
        if (*DAC12_DADR0 == voiceSample)
        {
          // after we have played the wav data, do a lead out
          VOICE_LEADIN = false; // exit         
        }
        else if (*DAC12_DADR0 > voiceSample)
        {
          *DAC12_DADR0 -= 1; // ramp down
        }
        else if (*DAC12_DADR0 < voiceSample)
        {
          *DAC12_DADR0 += 1; // ramp up
        }        
      }
      else if(VOICE_LEADOUT)
      {      
        // wav is finished
        // prevent pop by ramping signal to current sound sample before resuming the normal tone sound
        //          
        if (*DAC12_DADR0 == wave1.INT_amplitude)          
        {       
          VOICE_ACTIVE = false;  // finished WAV sound       
          VOICE_LEADOUT = false; // finished leadout
        }
        else if (*DAC12_DADR0 > wave1.INT_amplitude)
        {
          *DAC12_DADR0 -= 1;
        }
        else if (voiceSample < wave1.INT_amplitude)
        {
          *DAC12_DADR0 += 1;
        }                   
      }
      else
      {
        // playing the wav
        //
        *DAC12_DADR0 = voiceSample;
        voiceSampleIndex--;
        if(voiceSampleIndex < 0)
        {
          // Finish playing the wav, are there any more ?         
          // clear the last played sound
          //
          SOUND_QUEUE::queue[SOUND_QUEUE::playWav] = -1;

          SOUND_QUEUE::playWav++;
          if(SOUND_QUEUE::playWav >= MAX_SOUND_QUEUE )
          {
            SOUND_QUEUE::playWav = 0;
          }
          if (SOUND_QUEUE::queue[SOUND_QUEUE::playWav] == -1)
          {
            // the wav index is blank, we have finished
            //
            voiceSampleIndex = 0;
            VOICE_LEADOUT = true;  
            SOUND_QUEUE::playWav = 0;
          }
          else
          {
            // start playing the next wav
            //
            voiceSampleIndex = soundSizes[SOUND_QUEUE::queue[SOUND_QUEUE::playWav]] ;   // the  index of the first raw sample 
            soundPointer =  soundPointers[SOUND_QUEUE::queue[SOUND_QUEUE::playWav]] ;   // the wav array
          }         
        }
      }     
    }
    else
    {
      wave1.advance();   
      *DAC12_DADR0 = wave1.INT_amplitude;         // DAC update            - takes c. 210nS  - DAC ignores top 4 bits
    }   
  }
}


void setup_dac(void)       // Note make sure ADC is stopped before setup DAC
{ 
  
  *MSTP_MSTPCRD &= ~(0x01 << MSTPD20);  // Enable DAC12 module
  *DAC12_DADPR    = 0x00;               // DADR0 Format Select Register - Set right-justified format
  *DAC12_DAADSCR  = 0x80;               // D/A A/D Synchronous Start Control Register - Enable
  
// *DAC12_DAADSCR |= (0x01 << 7);     // set the D/A A/D sync bit 

//  *DAC12_DAADSCR  = 0x00;             // D/A A/D Synchronous Start Control Register - Default

// 36.3.2 Notes on Using the Internal Reference Voltage as the Reference Voltage
  *DAC12_DAVREFCR = 0x00;               // D/A VREF Control Register - Write 0x00 first - see 36.2.5
  *DAC12_DADR0    = 0x0000;             // D/A Data Register 0 
   delayMicroseconds(10);               // Needed delay - see datasheet
  *DAC12_DAVREFCR = 0x01;               // D/A VREF Control Register - Select AVCC0/AVSS0 for Vref           
  *DAC12_DACR     = 0x5F;               // D/A Control Register - 
  
   delayMicroseconds(5);                // 
  *DAC12_DADR0    = 0x0800;             // D/A Data Register 0 
  *PFS_P014PFS   = 0x00000000;          // Port Mode Control - Make sure all bits cleared
  *PFS_P014PFS  |= (0x1 << 15);         // ... use as an analog pin
  }




// Startup tune
//
void startTune() 
{
   wave1.setVolume(0.5);
  
   wave1.setFrequency(freq_c1);  
   delay(300);
   wave1.setFrequency(freq_e1);     
   delay(300);
   wave1.setFrequency(freq_c2);
   delay(300); 
   wave1.setFrequency(freq_c1);
}


// Main sound setup
// Setup the DAC, outputing on A0
// Setup the 20kHz timer that outputs the DAC waveform
// 
//
bool setup_soundWave()
{
  uint8_t timer_type = GPT_TIMER;
  int8_t tindex = FspTimer::get_available_timer(timer_type);

  wave1.setVolume(1.0);
  wave1.setFrequency(300);  //Hz

  setup_dac();

  if (tindex < 0){
    tindex = FspTimer::get_available_timer(timer_type, true);
  }
  if (tindex < 0){
    return false;
  }

  //FspTimer::force_use_of_pwm_reserved_timer();

  // 40kHz
  // This gives an interrupt period of 25uSec
  // Because the interrupt system is complex, and the sound interrupt
  // will always clash with the PWM and pulse-width interrupt if they are independant.
  // We do everything (Pulse, sampling and sound) from this single 40kHz interrupt
  //

  if(!soundWave.begin(TIMER_MODE_PERIODIC, timer_type, tindex, 40000, 0.0f, timer_DACOut_Interrupt)){
    return false;
  }  

  if (!soundWave.setup_overflow_irq()){
    return false;
  }

  if (!soundWave.open()){
    return false;
  }

  if (!soundWave.start()){
    return false;
  }

  startTune() ;    
  return true;
}




// adjust the sound according to signal
// ! Called at 100Hz !
//
void soundAlgorithm2(double signal)
{   

  double tempF = signal;  
  static float newVolume = 0;

  // gain or damping
  //
  signal *= 0.5;
  
  // change the tone
  //
  tempF = midTone + (midTone * signal  * 0.02);
  
  if(tempF < midTone)
  {
    tempF = midTone;
  }      
  
  if(tempF > (midTone * 2.0))
  {
    tempF = midTone * 2.0;
  }  
  
  wave1.setFrequency(tempF);  //Hz;   

  // volume
  //
  tempF = signal ; 
  
  if(tempF < 0)
  {
    tempF *= -1.0;
  }
  // value is always positive    

  // what is the change in volume ?
  //
  tempF = tempF-newVolume;


  // limit volume step
  //
  if (tempF > 4)
  {
    tempF = 4;
  }
  else if (tempF < -4 )
  {
    tempF = -4;
  }
  newVolume += tempF;
  
  volume = newVolume;
  volume /= 100.0;
  volume += 0.008;  

  wave1.setVolume(volume);    
}


// adjust the sound according to signal
// ! Called at 100Hz !
//
void soundAlgorithm3(double signal, float conduct_ratio)
{   

  double tempF = signal;  
  static float newVolume = 0;
  static float conductivity;
  

  // gain or damping
  //
  signal *= 0.5;
   
  
  tempF = (midTone * conductivity);
  
  if(conductivity  > conduct_ratio)
  {
    conductivity -= 0.004;  //  Track the conductivity, gradually up or down    
  }
  else
  {
    conductivity += 0.004;
  }

  tempF += midTone;
  
  if(tempF < midTone)
  {
    tempF = midTone;
  }      
  
  if(tempF > (midTone * 2.0))
  {
    tempF = midTone * 2.0;
  }  
  
  wave1.setFrequency(tempF);  //Hz;   

  // volume
  //
  tempF = signal ; 
  
  if(tempF < 0)
  {
    tempF *= -1.0;
  }
  // value is always positive    

  // what is the change in volume ?
  //
  tempF = tempF-newVolume;


  // limit volume step
  //
  if (tempF > 5)
  {
    tempF = 5;
  }
  else if (tempF < -5 )
  {
    tempF = -5;
  }
  newVolume += tempF;
  
  volume = newVolume;
  volume /= 100.0;
  volume += 0.008;  

  wave1.setVolume(volume);    
}



// convert number between 0.10 and 0.99 into a two digit sound
//
void queueNumber(float number)
{
  int tens;
  int remainder; 

  number *= 100.0;
  tens = (int) number;  
  remainder = tens % 10;
  tens /= 10; 
  if((tens < 10) && (tens > 0))
  {
    queueSound(tens);
  }
  if ((remainder < 10) && (remainder > 0))
  {
    queueSound(remainder);
  }
}

// add a wav file index to the list of wavs to be played (maximum 3)
// use the ENUM
void queueSound(int soundIndex)
{  
  // add sound to sound-queue
  //
  SOUND_QUEUE::queue[SOUND_QUEUE::loadWav] = soundIndex;  
  SOUND_QUEUE::loadWav++;  
  
  if (SOUND_QUEUE::loadWav >=  MAX_SOUND_QUEUE)
  {
    SOUND_QUEUE::loadWav = 0;
  }  
}

void startSounds()
{
  int index = SOUND_QUEUE::queue[0];

  SOUND_QUEUE::loadWav = 0; // ready for next load

  
  // if the first sound in the sound queue is valid. start playing it
  //
  if(index >= 0)  
  {
    voiceSampleIndex = soundSizes[index];   // the first raw wav sample 
    soundPointer =  soundPointers[index];
    VOICE_ACTIVE = true;
    VOICE_LEADIN = true;
  }

}



