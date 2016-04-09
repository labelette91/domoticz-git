#include "stdafx.h"
#include "deftype.h"
#include <wiringPi.h>
#include "hager.h"

//map delay 
#define DelayMicroseconds(VALUE)     delayMicrosecondsHard(VALUE);


//comand 
/*
byte cmdZ1eco   [] = {0x07,0x04,0xFA,0x00,0x02,0x20,0x8B,0x4E};
byte cmdZ1confor[] = {0x07,0x04,0xFA,0x00,0x02,0x20,0x88,0x51};
byte cmdZ2eco   [] = {0x07,0x04,0xFA,0x00,0x02,0x20,0x93,0x46};
byte cmdZ2confor[] = {0x07,0x04,0xFA,0x00,0x02,0x20,0x90,0x49};
byte cmdZ3eco   [] = {0x07,0x04,0xFA,0x00,0x02,0x20,0xA3,0x36};
byte cmdZ3confor[] = {0x07,0x04,0xFA,0x00,0x02,0x20,0xA0,0x39};
byte cmdZ1Hgel  [] = {0x07,0x04,0xFA,0x00,0x02,0x20,0xCC,0x0D};
byte cmdZ2Hgel  [] = {0x07,0x04,0xFA,0x00,0x02,0x20,0xD4,0x05};
byte cmdZ1config[] = {0x07,0x04,0xFA,0x00,0x02,0x05,0x01,0xF3};
byte cmdZ2config[] = {0x07,0x04,0xFA,0x00,0x02,0x05,0x02,0xF2};
byte cmdZ3config[] = {0x07,0x04,0xFA,0x00,0x02,0x05,0x04,0xF0};
     
byte cmdZ6eco[]    = {0x87,0x04,0xFA,0x00,0x02,0x20,0xA3,0xB6 };
byte cmdZ6confor[] = {0x87,0x04,0xFA,0x00,0x02,0x20,0xA0,0xB9 };
byte cmdZ6config[] = {0x87,0x04,0xFA,0x00,0x02,0x05,0x04,0x70 };

byte Addr2   [] = {0x07,0x04,0xF1,0x00,0x02 };
byte Addr3   [] = {0x07,0x04,0xF2,0x00,0x02 };

byte Z3eco   [] = {0x20,0xA3};
byte Z3confor[] = {0x20,0xA0};
byte Z3Hgel  [] = {0x20,0xE4};
byte Z3config[] = {0x05,0x04};

*/

//firt pulse = 1 length in dizaine de micro seconds 
byte Header[] PROGMEM= {
21 ,	//0 : 1 / 208
22 ,	//1 : 0 / 212
21 ,	//2 : 1 / 208
22 ,	//3 : 0 / 212
21 ,	//4 : 1 / 208
22 ,	//5 : 0 / 212
21 ,	//6 : 1 / 208
22 ,	//7 : 0 / 212
21 ,	//8 : 1 / 208
22 ,	//9 : 0 / 212
21 ,	//10 : 1 / 208
22 ,	//11 : 0 / 212
21 ,	//12 : 1 / 208
22 ,	//13 : 0 / 212
21 ,	//14 : 1 / 208
22 ,	//15 : 0 / 212
21 ,	//16 : 1 / 208
22 ,	//17 : 0 / 212
21 ,	//18 : 1 / 208
22 ,	//19 : 0 / 212
21 ,	//20 : 1 / 208
22 ,	//21 : 0 / 212
21 ,	//22 : 1 / 208
22 ,	//23 : 0 / 212
21 ,	//24 : 1 / 208
21 ,	//25 : 0 / 208
22 ,	//26 : 1 / 212
21 ,	//27 : 0 / 208
22 ,	//28 : 1 / 212
21 ,	//29 : 0 / 208
22 ,	//30 : 1 / 212
21 ,	//31 : 0 / 204
63 ,	//32 : 1 / 624
63 ,	//33 : 0 / 624
21 ,	//34 : 1 / 208
22 ,	//35 : 0 / 212
63 ,	//36 : 1 / 624
63 ,	//37 : 0 / 624
21 ,	//38 : 1 / 208----21
22 ,	//39 : 0 / 212
0 ,	//40 : 1 / 208--21

};


byte Addr1   [] = {0x07,0x04,0xFA,0x00,0x02 };

byte Z1eco   [] = {0x20,0x8B};
byte Z1confor[] = {0x20,0x88};
byte Z1Hgel  [] = {0x20,0xCC};
byte Z1config[] = {0x05,0x01};

byte HagerChk ;

byte HagerCmd[8];
byte IdCmd;


byte  pData;
byte  pLedPin; //pin for data input/output

#define HAGER_DELAY 210

void HagerSetPin(byte tx, byte pled)
{
    pData = tx ;
    pLedPin = pled ; 

}
//0 to 1 transition
//send 1
void HagerSendHigh()
{
digitalWrite(pData, LOW);
DelayMicroseconds(HAGER_DELAY)  ;
digitalWrite(pData, HIGH);
DelayMicroseconds(HAGER_DELAY)  ;
}
//1 to 0 transition
//send 0
void HagerSendLow()
{
digitalWrite(pData, HIGH);
DelayMicroseconds(HAGER_DELAY)  ;
digitalWrite(pData, LOW);
DelayMicroseconds(HAGER_DELAY)  ;
}
//send bit
void HagerSendBit(bool bit)
{
	if (bit)
		HagerSendHigh();
	else
		HagerSendLow();
}
//send byte 8 bits 
void HagerSendByte(byte  bite )
{
	HagerCmd[IdCmd++]=bite;
	HagerChk += bite;
	for (signed char i=7;i>=0;i--)
	{
		HagerSendBit ( bite & ( (byte)1 << i ) );
	}
}
//send array of byte 8 bits 
void HagerSendBytes(byte  * bite , byte nb )
{
	for (byte i=0;i<nb;i++)
	{
		HagerSendByte ( *bite++ ) ;
	}
}

//send Pulse = array duration in 10 micros
void HagerSendOne(byte * Pulse  )
{
  byte i=0;
  word delay;
  if (pLedPin) digitalWrite(pLedPin, HIGH);
  while ((*Pulse) != 0x00)
  {
    if (i%2==0) 
      //output = 1 
      digitalWrite(pData, HIGH);
    else
      //output = 0
      digitalWrite(pData, LOW);
    delay = (*Pulse++);
    delay = delay *10 ; 
    DelayMicroseconds(delay)  ;
    i++;
  }
  digitalWrite(pData, LOW);

  if (pLedPin) digitalWrite(pLedPin, LOW);
}


//cmnd = 0 : eco
//cmnd = 1 : conf
//cmnd = 2 : hors gel
//cmnd = 3 : arret
//cmnd = 4 : configuration t


void HagerSend ( byte * addr , byte cmnd )
{
  cli();
  HagerChk=0;
  IdCmd=0;
  HagerSendOne(Header );
  HagerSendBytes(addr , 5  ) ;
  if (cmnd==CMD_ECO)
		HagerSendBytes(Z1eco , 2  ) ;
  else if (cmnd==CMD_CONFOR)
		HagerSendBytes(Z1confor , 2  ) ;
  else if (cmnd==CMD_HGEL)
		HagerSendBytes(Z1Hgel , 2  ) ;
  else if (cmnd==CMD_ARRET)
		HagerSendBytes(Z1Hgel , 2  ) ;
  else if (cmnd==CMD_CONFIG)
		HagerSendBytes(Z1config , 2  ) ;

	//compute checksum
	HagerChk = ~HagerChk + 1 ;
		
	HagerSendByte(HagerChk);
  sei();

}


void HagerSends(byte id4, byte cmnd)
{

  //set adresse
  Addr1[2] = 0xF0 + id4;
  for (byte i = 0; i < 5; i++) {
    HagerSend(Addr1, cmnd);
    delay(10);
  }
}


