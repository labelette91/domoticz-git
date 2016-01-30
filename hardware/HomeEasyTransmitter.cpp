/*
 * HE300 Automatic Protocol Transmitter 'Simulator'
 * David Edmundson 21/11/2009
 *
 * This class allows use of the homeeasy automatic protocol to be used in any Arduino program
 *
 * Based heavily on demo code by
 * Martyn Henderson 02/10/2009  http://martgadget.blogspot.com
 *
 * The code has been moved into a class, for easy re-usage and has been optimised 
 * in regards to calculating and storing the transmitter address.
 *
 * You dont need to learn the Arduino into the socket, because it can 
 * pretend to be a transmitter you already have.
 *
 * Use the Automatic protocol reciever code above to find out
 * your transmitter address, and reciepient ID. 
 *
 * Original code from Martyn
 * *Only* tested with one HE300 transmitter (HE Address 272946) with its
 * slider switch set to 1 which appears to be 0 in fact.
 * Seems to work with a switch socket (HE302S) and can turn it on and off 
 * without the ardunio being learned into the socket.
 *
 * Edited code works with a HE305 transmitter, connecting to a HEXXX light fitting. The reciepient code for these 
 * appears to be 0b1010. I based my timings based on actual timings received from /my/ transmitter.
 * These seem to match the numbers used by Barnaby and Peter but not from Martyn. Adjust if needed.
 *
 * Cheers to Barnaby and Peter, because without the code above
 * that shows the receiver output this wouldnt have been possible!
 *
 * If you make something cool using HomeEasy and Arduino, please 
 * post back here with a link to your code so we can all enjoy.
 *
 */
// DelayMicroseconds     : delay avec suspend
// DelayMicrosecondsHard : delay polling timer  
 
#if defined(ARDUINO) && ARDUINO >= 100
#include <stdio.h>
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <timer.h>
#define DelayMicroseconds(VALUE)     delayMicroseconds(VALUE);
#define DelayMicrosecondsHard(VALUE) delayMicroseconds(VALUE);
#else
#include <wiringPi.h>
typedef unsigned char boolean;
typedef unsigned char byte;
#define cli() scheduler_realtime()
#define sei() scheduler_standard() 
#define DelayMicroseconds(VALUE)     delayMicroseconds(VALUE);
#define DelayMicrosecondsHard(VALUE) delayMicrosecondsHard(VALUE);
extern "C" void delayMicrosecondsHard (unsigned int howLong);
extern void scheduler_realtime() ;
extern void scheduler_standard() ;

#endif
#include "HomeEasyTransmitter.h"

//temps en µs
#define LATCH1_HIGH  270
#define LATCH1_LOW   9900
#define LATCH2_HIGH  270
#define LATCH2_LOW   2675


#define BIT_PULSE_HIGH  275
#define BIT_0_PULSE_LOW 275
#define BIT_1_PULSE_LOW 1225

//#define DelayMicroseconds(VALUE) delayMicroseconds(VALUE);Serial.println(VALUE); 
//#define DelayMicroseconds(VALUE) DelayMicros(VALUE);



  void rfm69_wait_t_data(void)
{
  /* t_data = 250ns = 4 insn at 16MHz */
#if defined(ARDUINO) && ARDUINO >= 100
  __asm__ __volatile__ ("nop");
  __asm__ __volatile__ ("nop");
  __asm__ __volatile__ ("nop");
  __asm__ __volatile__ ("nop");
#endif
}

void HomeEasyTransmitter::rfm69_set_data(byte state)
{
    if (clkPin>=0)
        rfm69_set_data_with_clk(state) ;
    else
        rfm69_set_data_with_clk( state);
}
void HomeEasyTransmitter::rfm69_set_data_without_clk(byte state)
{
  digitalWrite(txPin, state);
  if (ledPin) digitalWrite(ledPin, state);
}

void HomeEasyTransmitter::rfm69_set_data_with_clk(byte state)
{
  /* data is sampled on the rising edge of dclk/dio1 */

  digitalWrite(clkPin, LOW);
  digitalWrite(txPin, state);
  if (ledPin) digitalWrite(ledPin, state);
  rfm69_wait_t_data();
  digitalWrite(clkPin, HIGH);
  /* assume at least t_data from here */
}


HomeEasyTransmitter::HomeEasyTransmitter(short dataPin, short pclkPin , byte pLed )
{
  txPin  = dataPin;
  clkPin = pclkPin ;
  ledPin = pLed ;
}

//withou clock management
HomeEasyTransmitter::HomeEasyTransmitter(short dataPin,  byte pLed )
{
  txPin  = dataPin;
  clkPin = -1 ;
  ledPin = pLed ;
}

//sends either an on/off signal to the main switch
//always seems to work best if we send it twice..

void HomeEasyTransmitter::setSwitch(bool on, unsigned long transmitterId, short recipient)
{
  transmit(on, transmitterId, recipient);
  delay(10);
  transmit(on, transmitterId, recipient);
  delay(10);
  transmit(on, transmitterId, recipient);
  delay(10);
  transmit(on, transmitterId, recipient);
  delay(10);

}


//sends either an on/off signal to the main switch
void HomeEasyTransmitter::transmit(bool blnOn,unsigned long transmitterId, short recipient)
{
  int i;

  cli();
	initPin();

  // Do the latch sequence.. 
  rfm69_set_data( HIGH);
  DelayMicrosecondsHard(LATCH1_HIGH);     
  rfm69_set_data( LOW);
  DelayMicroseconds(LATCH1_LOW);     // low for 9900 for latch 1
  
  rfm69_set_data( HIGH);
  DelayMicrosecondsHard(LATCH2_HIGH);     
    rfm69_set_data( LOW);
  DelayMicroseconds(LATCH2_LOW);     // low for 2675 for latch 1


  // Send HE Device Address..
  // This is a 26 bit code. 
  // Start at MSB and iterate through to the lowest
  for(i=25; i>=0; i--)
  {
    //The typecasting seems a bit overkill, but 26 bits needs a long and the arduino compiler seems to love trying to 
    //convert everything to an standard int.
    //creates bitmask of only relevant bit. Check and send a 1 or 0 as applicable.
    bool bitToSend = (unsigned long)(transmitterId & ((unsigned long)1 << i)) != 0;
    sendPair(bitToSend);

  }

  // Send 26th bit - group 1/0
  sendPair(false);

  // Send 27th bit - on/off 1/0
  sendPair(blnOn);

  // last 4 bits - recipient   -- button 1 on the HE300 set to 
  // slider position I in this example:
  for(i=3; i>=0; i--)
  {
    bool bitToSend = ( byte)(recipient & ((byte )1 << i)) != 0;
    sendPair(bitToSend);
  }

/*    sendPair(true);
    sendPair(false);
    sendPair(true);
    sendPair(false);*/

  rfm69_set_data( HIGH);   // high again (shut up)
  DelayMicrosecondsHard(LATCH2_HIGH);      // wait a moment
  rfm69_set_data( LOW);    // low again for 2675 - latch 2.
  DelayMicroseconds(LATCH2_LOW);      // wait a moment
  sei();

}

void HomeEasyTransmitter::sendBit(bool b)
{
  if (b)
  {
    rfm69_set_data( HIGH);
    DelayMicrosecondsHard(BIT_PULSE_HIGH);
    rfm69_set_data( LOW);
    DelayMicroseconds(BIT_1_PULSE_LOW);
  }
  else
  {
    rfm69_set_data( HIGH);
    DelayMicrosecondsHard(BIT_PULSE_HIGH);
    rfm69_set_data( LOW);
    DelayMicrosecondsHard(BIT_0_PULSE_LOW);
  }
}

void HomeEasyTransmitter::sendPair(bool b)
{
  // Send the Manchester Encoded data 01 or 10, never 11 or 00
  if(b)
  {
    sendBit(true);
    sendBit(false);
  }
  else
  {
    sendBit(false);
    sendBit(true);
  }
}


 void HomeEasyTransmitter::initPin()
    {
    pinMode(txPin, OUTPUT);      // transmitter pin.
    digitalWrite(txPin, LOW);    //no tx 

    if (clkPin>=0)
		{
    	pinMode(clkPin, OUTPUT);      
    	digitalWrite(clkPin, LOW);
		}
    }