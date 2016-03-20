
#include "fifo.h"

bool TFifo::Empty()
{
    return (WrIndex==RdIndex);
}
bool TFifo::Full()
{
    return (GetNext(WrIndex)==RdIndex);
}

void TFifo::Clear ()
{
  memset(Value,sizeof(Value),0) ;
  WrIndex=0;
  RdIndex=0;
}
TFifo::TFifo ()
{
  Size = NBRECORD;
  Clear();
}
TFifo::~TFifo ()
{
  
}
byte TFifo::GetNext(int Index)
{
  if (Index>=(Size-1) )
    return 0;
  else
    return Index+1;
}
void TFifo::Put(byte pSize , byte * pData )
{
  if (!Full())
  {
      Value[WrIndex].Size= pSize ;
      memcpy(Value[WrIndex].Data,pData,RECORD_ZIZE);
      WrIndex = GetNext(WrIndex);
  }
}
byte *  TFifo::Get(byte & pSize )
{
byte * data=0;
    pSize=0;
    if (!Empty())
    {
        //return last recorded value
        pSize = Value[RdIndex].Size ; 
        data = Value[RdIndex].Data;
        RdIndex = GetNext(RdIndex);
    }
    return data;
}

