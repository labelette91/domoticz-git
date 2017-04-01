//file simulated arduino SPI.cpp
#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include "Print.h"
#include <wiringPiSPI.h>

#include "deftype.h"

#define noInterrupts()
#define interrupts()

#ifndef LSBFIRST
#define LSBFIRST 0
#endif
#ifndef MSBFIRST
#define MSBFIRST 1
#endif
#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06
#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C
#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR

class SPIClass {
private:
	int spiChannel ;
	int spiFd ;
public:
  // Initialize the SPI library
	bool  Setup (int pspiChannel , int speed)
{
	spiChannel = pspiChannel;
  if ((spiFd = wiringPiSPISetup (spiChannel, speed)) < 0)
  {
    return true  ;
   
  }
	else
		return false ;
}


	void begin()
  {
  };

  // If SPI is used from within an interrupt, this function registers
  // that interrupt with the SPI library, so beginTransaction() can
  // prevent conflicts.  The input interruptNumber is the number used
  // with attachInterrupt.  If SPI is used from a different interrupt
  // (eg, a timer), interruptNumber should be 255.
  void usingInterrupt(uint8_t interruptNumber);
  // And this does the opposite.
  void notUsingInterrupt(uint8_t interruptNumber);
  // Note: the usingInterrupt and notUsingInterrupt functions should
  // not to be called from ISR context or inside a transaction.
  // For details see:
  // https://github.com/arduino/Arduino/pull/2381
  // https://github.com/arduino/Arduino/pull/2449

  // Before using SPI.transfer() or asserting chip select pins,
  // this function is used to gain exclusive access to the SPI bus
  // and configure the correct settings.
  void beginTransaction() {
  }

  // Write to the SPI bus (MOSI pin) and also receive (MISO pin)
  uint8_t transfer(uint8_t data) {
    return data;
  }
  uint16_t transfer16(uint16_t data) {
    union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } in, out;
    in.val = data;
    return out.val;
  }
  bool transfer(byte *buf, size_t count) {
		if (wiringPiSPIDataRW (spiChannel, buf, 2) == -1)
		  return true;
		else
		  return false;

  }
  // After performing a group of transfers and releasing the chip select
  // signal, this function allows others to access the SPI bus
  void endTransaction(void) {

  }

  // Disable the SPI bus
  void end()
  {
//		close (spiFd) ;
  };

  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  void setBitOrder(uint8_t bitOrder) {
  };
  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  void setDataMode(uint8_t dataMode) {
  };
  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  void setClockDivider(uint8_t clockDiv) {
  };
  // These undocumented functions should not be used.  SPI.transfer()
  // polls the hardware flag which is automatically cleared as the
  // AVR responds to SPI's interrupt
  void attachInterrupt() {  };
  void detachInterrupt() {  };

};

extern SPIClass SPI;


#define SS 0
#define RF69_IRQ_PIN 0 
#define RF69_IRQ_NUM 0 
#define SPCR _SPCR
#define SPSR _SPSR


#endif
