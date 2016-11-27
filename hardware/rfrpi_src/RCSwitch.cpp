/* ==========================================================================
 * RCSwitch.h
 * --------------------------------------------------------------------------
 * RF433 demonstrator for rfrpi Raspberry PI shield
 * see : http://www.disk91.com/?p=1323
 * --------------------------------------------------------------------------
 * See License on next header
 * --------------------------------------------------------------------------
 *
 *  Created on: 23 Feb. 2014
 *   Author: disk91 - Paul Pinault (c) 2014
 * --------------------------------------------------------------------------
 * This Class is managing the RF433 reception, it is based on RCSwitch Arduino library
 * Adapted for the RFRPI card. Author and license are indicated in the next header
 *
 * This Class is the hardware interface and the layer 1 decoder based on the
 * different known decoder.
 * By default, only Oregon Scientic, OOk and Dio are activated, you can modify
 * code to activate others, depending on your sensors. Activating too much decoders
 * will consume cpu time and can create decodeur collisions
 *
 * --------------------------------------------------------------------------
 */
/* ---------------------------------------------------------------------------
  RCSwitch - Arduino libary for remote control outlet switches
  Copyright (c) 2011 Suat �zg�r.  All right reserved.

  Contributors:
  - Andre Koehler / info(at)tomate-online(dot)de
  - Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
  
  Project home: http://code.google.com/p/rc-switch/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  -------------------------------------------------------------------------------
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>

#include "RCSwitch.h"
#include "RcOok.h"
#include "version.h"

char RCSwitch::OokReceivedCode[RCSWITCH_MAX_MESS_SIZE];
bool RCSwitch::OokAvailableCode;

OregonDecoderV2 * orscV2;
OregonDecoderV3 * orscV3;
RCSwitch_ * rcswp1;
DIO		  * dio;
#ifdef __ALLDECODER
RCSwitch_  * rcswp2;
CrestaDecoder * cres;
KakuDecoder * kaku;
XrfDecoder * xrf;
HezDecoder * hez;
VisonicDecoder * viso;
EMxDecoder * emx;
KSxDecoder * ksx;
FSxDecoder * fsx;
#endif


/* =================================================
 * Construct RCSwitch
 */
RCSwitch::RCSwitch(int rxpin, int txpin, int _rxepin, int _txepin) {

	RCSwitch::OokAvailableCode = false;
	RCSwitch::OokReceivedCode[0] = '\0';

	orscV2 = new OregonDecoderV2(this);
	orscV3 = new OregonDecoderV3(this);
	rcswp1 = new RCSwitch_(1,this);
	dio = new DIO(this);

#ifdef __ALLDECODER
	// @TODO - Ajouter les autres ...
	rcswp2.configure(2,this);
#endif

#if RFRPI_RPI_VERSION == 1
	if (rxpin != -1 && _rxepin != -1) {
		this->nReceiverEnablePin = _rxepin;
		pinMode(this->nReceiverEnablePin,OUTPUT);
		digitalWrite(this->nReceiverEnablePin,LOW);
		pinMode(rxpin,INPUT);
		this->enableReceive(rxpin);
	} else this->nReceiverInterrupt = -1;
#else
	if (rxpin != -1 && _rxepin != -1) {
       this->nReceiverEnablePin = _rxepin;
	   pinMode(this->nReceiverEnablePin,OUTPUT);
	   digitalWrite(this->nReceiverEnablePin,LOW);
   	   this->enableReceive(rxpin);
   	   this->rxEnable = true;
       pthread_create(&this->myThread,NULL, receiveLoop, this);
	} else {
	   this->nReceiverInterrupt = -1;
	   this->rxEnable = false;
	}
#endif

	if (txpin != -1 && _txepin != -1) {
		this->nTransmitterEnablePin = _txepin;
		pinMode(this->nTransmitterEnablePin,OUTPUT);
		digitalWrite(this->nTransmitterEnablePin,LOW);
		this->enableTransmit(txpin);
	} else this->nTransmitterPin = -1;

}

RCSwitch::~RCSwitch() {
#if RFRPI_RPI_VERSION == 2
	pthread_cancel(this->myThread);
	pthread_join(this->myThread, NULL);
	this->myThread = pthread_self();
#endif

	delete orscV2;
	delete orscV3;
	delete rcswp1;
	delete dio;
#ifdef __ALLDECODER
	delete rcswp2;
    delete cres;
    delete kaku;
    delete xrf;
    delete hez;
    delete viso;
    delete emx;
    delete ksx;
    delete fsx;
#endif
}

/**
 * Enable transmissions
 *
 * @param nTransmitterPin    Arduino Pin to which the sender is connected to
 */
void RCSwitch::enableTransmit(int nTransmitterPin) {
  this->nTransmitterPin = nTransmitterPin;
  pinMode(this->nTransmitterPin, OUTPUT);
  digitalWrite(this->nTransmitterPin, LOW);
}

/**
  * Disable transmissions
  */
void RCSwitch::disableTransmit() {
  this->nTransmitterPin = -1;
}

/**
 * Enable receiving data
 */
void RCSwitch::enableReceive(int interrupt) {
  this->nReceiverInterrupt = interrupt;
  this->enableReceive();
}

void RCSwitch::enableReceive() {
#if RFRPI_RPI_VERSION == 1
  if (this->nReceiverInterrupt != -1) {
    wiringPiISR(this->nReceiverInterrupt, INT_EDGE_BOTH, &handleInterrupt);
  }
#else
  this->rxEnable = true;
#endif
  if (this->nReceiverEnablePin != -1 && this->nTransmitterEnablePin != -1) {
	  digitalWrite(this->nTransmitterEnablePin,LOW);
	  digitalWrite(this->nReceiverEnablePin,HIGH);
          // 10 ms break to ensure configuation has setup
	  usleep(10000);
  }
}

/**
 * Disable receiving data
 */
void RCSwitch::disableReceive() {
#if RFRPI_RPI_VERSION == 1
  if (this->nReceiverInterrupt != -1) {
    wiringPiISR(this->nReceiverInterrupt, INT_EDGE_BOTH, &handleNoInterrupt);
  }
#else
  this->rxEnable = false;
#endif
  if (this->nReceiverEnablePin != -1 && this->nTransmitterEnablePin != -1) {
	  digitalWrite(this->nTransmitterEnablePin,LOW);
	  digitalWrite(this->nReceiverEnablePin,LOW);
          // 10 ms break to ensure configuation has setup
	  usleep(10000);
  }
}

/* ======================================================
 * Antenna switch management
 * ------------------------------------------------------
 */
void RCSwitch::switch2transmit() {
#if RFRPI_RPI_VERSION == 1
  if (this->nReceiverInterrupt != -1) {
    wiringPiISR(this->nReceiverInterrupt, INT_EDGE_BOTH, &handleNoInterrupt);
  }
#else
  this->rxEnable = false;
#endif
  if (this->nReceiverEnablePin != -1 && this->nTransmitterEnablePin != -1) {
	  digitalWrite(this->nReceiverEnablePin,LOW);
	  digitalWrite(this->nTransmitterEnablePin,HIGH);
          // 10 ms break to ensure configuation has setup
	  usleep(10000);
  }
}

void RCSwitch::switch2receive() {
#if RFRPI_RPI_VERSION == 1
  if (this->nReceiverInterrupt != -1) {
    wiringPiISR(this->nReceiverInterrupt, INT_EDGE_BOTH, &handleInterrupt);
  }
#else
  this->rxEnable = true;
#endif
  if (this->nReceiverEnablePin != -1 && this->nTransmitterEnablePin != -1) {
	  digitalWrite(this->nTransmitterEnablePin,LOW);
	  digitalWrite(this->nReceiverEnablePin,HIGH);
          // 10 ms break to ensure configuation has setup
	  usleep(10000);
  }
}

// ==============================================
// Set to true when a code has been decode by the
// OoK module
bool RCSwitch::OokAvailable() {
	return RCSwitch::OokAvailableCode;
}

// ==============================================
// Return the received code decoded by Ook engine
// if available and true, otherwith return false
// can be used w/o OokAvailable
//
// The decoded value is stored in the v this string
// must have a size equal to RCSWITCH_MAX_MESS_SIZE
//
// Reset available flag (this allow new capture from
// interrupt (locked otherwize to avoid reentrance

bool RCSwitch::getOokCode(char * _dest) {
	if ( RCSwitch::OokAvailableCode ) {
		strcpy(_dest,RCSwitch::OokReceivedCode);
		RCSwitch::OokAvailableCode = false;
		return true;
	} else return false;
}

// =============================================
// reset available (autorize new capture)
void RCSwitch::OokResetAvailable() {
	RCSwitch::OokAvailableCode = false;
}


// =============================================
// Process loop for kernel managed version
void * RCSwitch::receiveLoop(void * _param) {

    RCSwitch * myRCSwitch = (RCSwitch *) _param;
    FILE * fp;
    fp = fopen("/dev/rfrpi", "r");
    if ( fp == NULL ) {
       printf("[ERROR] /dev/rfrpi device not found - kernel driver must be started !!\n");
       exit(1);
    }

    char buffer[2048];
    char * bstart = buffer;
    int cValues = 0;
    int reste = 0;
    while (1) {
       int count = fread(bstart,1,2048,fp);
       count += reste;

       if ( count > 0 ) {
         /*
         printf("V : [");
         for ( int k = 0 ; k < count ; k ++  ) printf("%d ",buffer[k]);
         printf("\n");
         */
         int start=0;
         int end = 0;
         while ( end < count ) {
            while ( end < count && buffer[end] != '\n' && buffer[end] != 0 ) end++;
            if ( end < count ) {
                buffer[end] = 0;
                unsigned int duration = atoi(&buffer[start]);
                word p = (unsigned short int)duration;

                if ( !RCSwitch::OokAvailableCode && duration < 32000 ) {		// avoid reentrance -- wait until data is read
            	  if (orscV2->nextPulse(p)) 	{ orscV2->sprint("OSV2 ",RCSwitch::OokReceivedCode); orscV2->resetDecoder(); RCSwitch::OokAvailableCode = true; }
            	  if (orscV3->nextPulse(p)) 	{ orscV3->sprint("OSV3 ",RCSwitch::OokReceivedCode); orscV3->resetDecoder(); RCSwitch::OokAvailableCode = true; }
	              if (rcswp1->nextPulse(p)) 	{ rcswp1->sprint("RCSW ",RCSwitch::OokReceivedCode); rcswp1->resetDecoder(); RCSwitch::OokAvailableCode = true; }
            	  if (dio->nextPulse(p)) 		{ dio->sprint("DIO_ ",RCSwitch::OokReceivedCode); dio->resetDecoder(); RCSwitch::OokAvailableCode = true; }

#ifdef __ALLDECODER
            	  if (rcswp2.nextPulse(p)) 	{ RCSwitch::OokAvailableCode = true; rcswp2.sprint("ALR2 ",RCSwitch::OokReceivedCode); rcswp2.resetDecoder(); }
            	  if (cres.nextPulse(p)) 	{ cres.print("CRES"); cres->resetDecoder(); }
            	  if (kaku.nextPulse(p)) 	{ kaku.print("KAKU"); kaku->resetDecoder(); }
            	  if (xrf.nextPulse(p))  	{ xrf.print("XRF"); xrf->resetDecoder(); }
            	  if (hez.nextPulse(p)) 	{ hez.print("HEZ"); hez->resetDecoder(); }
            	  if (viso.nextPulse(p)) 	{ viso.print("VISO"); viso->resetDecoder(); }
            	  if (emx.nextPulse(p)) 	{ emx.print("EMX"); emx->resetDecoder(); }
            	  if (ksx.nextPulse(p)) 	{ ksx.print("KSX"); ksx->resetDecoder(); }
            	  if (fsx.nextPulse(p)) 	{ fsx.print("FSX"); fsx->resetDecoder(); }
#endif
                } else if (duration >=32000 ) { // too long wait ... timeout, previously we had noise ...
                  orscV2->resetDecoder();
                  orscV3->resetDecoder();
                  rcswp1->resetDecoder();
                  dio->resetDecoder();
                }
                end++;
                start = end;
            }
            if ( end >= count ) {
                for ( int i = start ; i < end ; i ++ ) {
                   buffer[i-start] = buffer[i];
                   //printf("K : %d ...",buffer[i]);
                }
                if ( end - start > 0)
                   printf("]\n");
                bstart = &buffer[end-start];
                reste = end - start;
            }
         }
       }
       usleep(10000); // 10 ms sleep then ready the device again
    }
    return NULL;

}


// ==============================================
// Interrupt Handler to manage the different protocols
void RCSwitch::handleInterrupt() {

  static unsigned int duration;
  static unsigned int changeCount;
  static unsigned long lastTime;
  static unsigned int repeatCount;
  long time = micros();
  duration = time - lastTime;
  lastTime = time;
  word p = (unsigned short int) duration;
  //if ( p != duration ) printf("*** OUPS *** %d // %d \n",duration,p);

  // Avoid re-entry
  if ( !OokAvailableCode && duration < 32000 ) {		// avoid reentrance -- wait until data is read
	  if (orscV2->nextPulse(p)) 	{ orscV2->sprint("OSV2 ",RCSwitch::OokReceivedCode); orscV2->resetDecoder(); RCSwitch::OokAvailableCode = true; }
	  if (orscV3->nextPulse(p)) 	{ orscV3->sprint("OSV3 ",RCSwitch::OokReceivedCode); orscV3->resetDecoder(); RCSwitch::OokAvailableCode = true; }
	  if (rcswp1->nextPulse(p)) 	{ rcswp1->sprint("RCSW ",RCSwitch::OokReceivedCode); rcswp1->resetDecoder(); RCSwitch::OokAvailableCode = true; }
	  if (dio->nextPulse(p)) 		{ dio->sprint("DIO_ ",RCSwitch::OokReceivedCode); dio->resetDecoder(); RCSwitch::OokAvailableCode = true; }

#ifdef __ALLDECODER
	  if (rcswp2.nextPulse(p)) 	{ RCSwitch::OokAvailableCode = true; rcswp2.sprint("ALR2 ",RCSwitch::OokReceivedCode); rcswp2.resetDecoder(); }
	  if (cres.nextPulse(p)) 	{ cres.print("CRES"); cres.resetDecoder(); }
	  if (kaku.nextPulse(p)) 	{ kaku.print("KAKU"); kaku.resetDecoder(); }
	  if (xrf.nextPulse(p))  	{ xrf.print("XRF"); xrf.resetDecoder(); }
	  if (hez.nextPulse(p)) 	{ hez.print("HEZ"); hez.resetDecoder(); }
	  if (viso.nextPulse(p)) 	{ viso.print("VISO"); viso.resetDecoder(); }
	  if (emx.nextPulse(p)) 	{ emx.print("EMX"); emx.resetDecoder(); }
	  if (ksx.nextPulse(p)) 	{ ksx.print("KSX"); ksx.resetDecoder(); }
	  if (fsx.nextPulse(p)) 	{ fsx.print("FSX"); fsx.resetDecoder(); }
#endif
  } else if (duration >=32000 ) { // too long wait ... timeout, previously we had noise ...
      orscV2->resetDecoder();
      orscV3->resetDecoder();
      rcswp1->resetDecoder();
      dio->resetDecoder();
  }
}

// ============================================
// Not clear how to disable interrupt so, the more
// simple way is to change the functin for doing nothing.
void RCSwitch::handleNoInterrupt() {
   // do nothing on interrupt
}



// =======================================================
// Transmit pulse
// -------------------------------------------------------
// Transmit is a High level timing followed by a low level
// timing un micro-second
// The time constraint is critical and at any time this function
// is subject to be stopped by the scheduler and this can modify
// the timing in execution. For this reason, the procedure
// count time already spent on each state to ensure it is the
// nearest possible from the expected time.
// The best precision we can expect is 5us as it is the basic unit
// used in this procedure. 5us delay is done on looping so this
// procedure keep the proc used all the time.
// Before calling this function the switch2transmis function
// will stop reception to avoid break/stop of transmission
void RCSwitch::transmit(int nHighPulses, int nLowPulses) {
    long startTime = 0;
    long cTime = 0;

    boolean disabled_Receive = false;
    int nReceiverInterrupt_backup = nReceiverInterrupt;
    if (this->nTransmitterPin != -1) {

        digitalWrite(this->nTransmitterPin, HIGH);
        startTime = micros();
        do {
           delayMicroseconds(5);
           cTime = micros();
        } while ( (cTime - startTime) < nHighPulses );


        digitalWrite(this->nTransmitterPin, LOW);
        startTime = micros();
        do {
           delayMicroseconds(5);
           cTime = micros();
        } while ( (cTime - startTime) < nLowPulses );
    }
}



