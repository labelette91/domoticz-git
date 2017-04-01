#ifndef _DecodeHomeEasy_
#define _DecodeHomeEasy_

#include "deftype.h"

#define NBBIT  32
#define PULSE_HIGH 400
#define PULSE_ONE  1300
#define PULSE_ZERO 300
#define PULSE_SYNCHRO 2000

#define TOLERANCE 400

#define HOMESWITCH_ID0 0x3B
#define HOMESWITCH_ID1 0x80

#define HOMESWITCH  (HOMESWITCH_ID0*256+ HOMESWITCH_ID1 )

//#define _debug

/* bit 1 : |-|____ : short Pulse One / long  Pulse Low  */
/* bit 0 : |-|_    : short Pulse One / short Pulse Low  */


#define TEST_PULSE(WIDTH,PULSE_LEN,TOL)((WIDTH> (PULSE_LEN-TOL))&&(WIDTH<(PULSE_LEN+TOL)))

class DecodeHomeEasy {
public:
    byte total_bits, state, MaxCode , NbCodeOk , lastBit , pos, data[25]; ;
    unsigned long CurCode , Code , tmpCode, lastCode;
    unsigned long LastReceived ; 

public:
    enum { UNKNOWN, T0, T1, T2, T3, OK, DONE };

    const byte* getData (byte& count) const ;
    const byte* getData () const ;
     byte getBytes ()  ;

    void resetDecoder () ;
    
    DecodeHomeEasy (byte pMaxCode=5) ;
    bool nextOnePulse (word pWidth , byte data)   ;
    
  bool nextPulse (word width , byte BitData)   ;
 };

#endif