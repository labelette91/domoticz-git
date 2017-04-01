/*
dummy declatation for wiringpi lib
 */

#include "stdafx.h"
#ifndef WITH_GPIO
#include <wiringPi.h>
#include <wiringPiSPI.h>

 int wiringPiFailure (int fatal, const char *message, ...) {return 0;}

// Core wiringPi functions

 struct wiringPiNodeStruct *wiringPiFindNode (int pin) {return 0;}
 struct wiringPiNodeStruct *wiringPiNewNode  (int pinBase, int numPins) {return 0;}

 int  wiringPiSetup       (void) {return 0;}
 int  wiringPiSetupSys    (void) {return 0;}
 int  wiringPiSetupGpio   (void) {return 0;}
 int  wiringPiSetupPhys   (void) {return 0;}

 void pinModeAlt          (int pin, int mode) {return ;}
 void pinMode             (int pin, int mode) {return ;}
 void pullUpDnControl     (int pin, int pud) {return ;}
 int  digitalRead         (int pin) {return 0;}
 void digitalWrite        (int pin, int value) {return ;}
 void pwmWrite            (int pin, int value) {return ;}
 int  analogRead          (int pin) {return 0;}
 void analogWrite         (int pin, int value) {return ;}

// PiFace specifics 
//	(Deprecated)

 int  wiringPiSetupPiFace (void) {return 0;}
 int  wiringPiSetupPiFaceForGpioProg (void) {return 0;}	// Don't use this - for gpio program only

// On-Board Raspberry Pi hardware specific stuff

 int  piBoardRev          (void) {return 0;}
 void piBoardId           (int *model, int *rev, int *mem, int *maker, int *overVolted) {return ;}
 int  wpiPinToGpio        (int wpiPin) {return 0;}
 int  physPinToGpio       (int physPin) {return 0;}
 void setPadDrive         (int group, int value) {return ;}
 int  getAlt              (int pin) {return 0;}
 void pwmToneWrite        (int pin, int freq) {return ;}
 void digitalWriteByte    (int value) {return ;}
 void pwmSetMode          (int mode) {return ;}
 void pwmSetRange         (unsigned int range) {return ;}
 void pwmSetClock         (int divisor) {return ;}
 void gpioClockSet        (int pin, int freq) {return ;}

// Interrupts
//	(Also Pi hardware specific)

 int  waitForInterrupt    (int pin, int mS) {return 0;}
 int  wiringPiISR         (int pin, int mode, void (*function)(void)) {return 0;}

// Threads

 int  piThreadCreate      (void *(*fn)(void *)) {return 0;}
 void piLock              (int key) {return ;}
 void piUnlock            (int key) {return ;}

// Schedulling priority

 int piHiPri (const int pri) {return 0;}

// Extras from arduino land

 void         delay             (unsigned int howLong) {return ;}
 void         delayMicroseconds (unsigned int howLong) {return ;}
 extern "C"  void  delayMicrosecondsHard(unsigned int howLong) ;
 void         delayMicrosecondsHard(unsigned int howLong) { return; }
 
 unsigned int millis            (void) {return 0;}
 unsigned int micros            (void) {return 0;}

 int wiringPiSPIDataRW(int channel, unsigned char *data, int len) { return 0; }
 int wiringPiSPISetupMode(int channel, int speed, int mode) { return 0; }
 int wiringPiSPISetup(int channel, int speed) { return 0; }

#endif