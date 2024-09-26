#ifndef _TARGET_SENSE_H
#define _TARGET_SENSE_H


//------------------------------------
// target_sense.h
//
// 
//
// Modified 07-June-2024
//
//------------------------------------

// Points on the sample array and where they are in time.
// Fixed by the sample speed of the micro. Never change these indexes
//
#define INDEX_12uSEC (4)
#define INDEX_15uSEC (5)
#define INDEX_18uSEC (6)
#define INDEX_21uSEC (7)
#define INDEX_24uSEC (8)
#define INDEX_27uSEC (9)
#define INDEX_30uSEC (10)
#define INDEX_33uSEC (11)
#define INDEX_36uSEC (12)
#define INDEX_39uSEC (13)
#define INDEX_42uSEC (14)
#define INDEX_45uSEC (15)
#define INDEX_48uSEC (16)
#define INDEX_51uSEC (17)
#define INDEX_54uSEC (18)
#define INDEX_57uSEC (19)
#define INDEX_60uSEC (20)
#define INDEX_63uSEC (21)
#define INDEX_66uSEC (22)
#define INDEX_69uSEC (23)
#define INDEX_72uSEC (24)
#define INDEX_75uSEC (25)
#define INDEX_78uSEC (26)
#define INDEX_81uSEC (27)
#define INDEX_84uSEC (28)
#define INDEX_87uSEC (29)
#define INDEX_90uSEC (30)
#define INDEX_93uSEC (31)
#define INDEX_96uSEC (32)
#define INDEX_99uSEC (33)
#define INDEX_102uSEC (34)
#define INDEX_105uSEC (35)



#define R1 (0)
#define R2 (1)
#define R3 (2)
#define R4 (3)

// Audio Tones
//

// middle C
#define freq_c1 (262)
#define freq_f0 (174)
#define freq_f1 (349)
#define freq_e0 (164)
#define freq_e1 (330)
#define freq_g1 (392)
#define freq_c2 (523)

double midTone = freq_e0;  // Hz


void normalise(double arr[], int sz);
bool isTarget_Set1(double sig_curve[]);
bool isIron_Set1(double sig_curve[], bool printOut );
void targetID_Set1(double sig_curve[], bool printOut);

//
enum TARGETID{OK_BIG, OK_SMALL, OK_MIDDLE, Fe, NO_TARGET};
#define TARGET_TYPES (5)

class TARGET_SENSE{

  public:
    static TARGETID targetID;  

    // change the tone by this much depending on target
    //
    static float Tones[TARGET_TYPES]; 
};

TARGETID TARGET_SENSE::targetID;


// What frequency when particular target ID (unused with voice ID)
//
// OK_BIG, OK_SMALL, OK_MIDDLE, Fe, NO_TARGET
//
float TARGET_SENSE::Tones[TARGET_TYPES] = {450,200,174,140,174};



//------ 20cm coil ------------------------------------------------------------
//#include "coil_20cm.h"

//----- 13cm (5 inch) coil -------------------------------------------------
//#include "coil_highspeed1.h"


//------ 20cm 1000uH coil ------------------------------------------------------------
//#include "coil_20cm_1000uH.h"


#endif
// _TARGET_SENSE_H