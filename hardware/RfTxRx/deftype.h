#ifndef DefType_H
#define DefType_H

typedef unsigned char boolean;
typedef unsigned char byte;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned int size_t   ;
typedef unsigned short word;

#define PROGMEM

#define cli() scheduler_realtime()
#define sei() scheduler_standard() 

extern "C" void delayMicrosecondsHard(unsigned int howLong);
extern void scheduler_realtime();
extern void scheduler_standard();

#endif
