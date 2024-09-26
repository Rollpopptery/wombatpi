
// wombatpi.net


#ifndef _WOMBAT_H
#define _WOMBAT_H


#define USB_SERIAL_ENABLED (true)

// Set the coil type here
//
#include "coil_20_1364.h"
COIL_20_1364 theCoil;



// On the UNO R4, Serial out pins is 'Serial1'
// The USB serial is 'Serial'

bool WIFI_SERIAL_ENABLED = false;   // Serial1   not used


// communications mode (serial port data modes) described in wombat_coms
//
// Only one mode of operation is valid , mode a
volatile enum {a, S} mode = a;

volatile enum {DEFAULT} targetMode = DEFAULT;



#endif
