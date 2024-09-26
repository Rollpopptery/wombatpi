/*
Pulse and Sample

wombatpi.net

Modified 14-Apr-2024


Notes:  
The Sample window is sensitive to all digital and analog IO the occurs during,
Thus is enclosed in a no-interrupts block. 
The audio output A0 feeds to high impedance (i.e > 4k) audio amp

*/

#include <arduino.h>
//#include "pwm.h"
#include "FspTimer.h"
#include "wombat_analog.h"
#include "target_sense.h"


FspTimer pulse_timer;




// coil pulse timer
// now using dac_audio interrupt
//


void setupSample() {

  // pulse output pin
  //
  pinMode(D3, OUTPUT);

  pinMode(analogPin, INPUT);  
  analogReference(AR_EXTERNAL);
  
  analogReadResolution(14);        // This code reads ADC result directly, so this setup not needed
 
  
  uint16_t value = analogRead(analogPin);  // Do a read to get everthing set-up
  // Update registers - do NOT use analogRead() after this

  *ADC140_ADCER = 0x06;              // 14 bit mode (already set), clear ACE bit 5
  
  //-------------------------------------------------------------
 
  *ADC140_ADCSR |= (0x01 << 15);     // Start a conversion
  
  adc_val_16 = 8;  
}

