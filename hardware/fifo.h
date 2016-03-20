#ifndef fifo_
#define fifo_

#include <stdio.h>
#include <string.h>

#include "deftype.h"

#define NBRECORD     20
#define RECORD_ZIZE   50

struct T_RECORD{
        byte Size ;
        byte Data[RECORD_ZIZE];
};

class TFifo {
public:
  TFifo ();
  ~TFifo ();

//store value and return last
void Put(byte pSize , byte * pData );
void Clear();
byte *  Get(byte & pSize );
bool Empty();
bool Full();


private:
        T_RECORD Value[NBRECORD];
  byte Size; //size of buffer'
  byte WrIndex; //current record index
  byte RdIndex; //current record index

  byte GetNext(int Index);

};

#endif