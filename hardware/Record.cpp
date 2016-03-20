#include "stdafx.h"

#include "Record.h"

TRecord::TRecord()
{
	init();
}
TRecord::	~TRecord()
{
}

void TRecord::init()
{
PWr=0;
PRd=0;

}
void TRecord::put(int p)
{
    Pulse[PWr++]=p;
    if (PWr >=  NBPULSE ) PWr=0;
}
int TRecord::get()
{
	int p=0 ;
	if (PWr!=PRd)
	{
    p=Pulse[PRd++];
    if (PRd >=  NBPULSE ) PRd=0;
  }
  return p;
}

std::string TRecord::ToString()
{
  int p=0;
  int n;
  std::string mes ;
  	
  char Mes[128*3];
    p=get();
    n=0;
		while(p!=0) 
		{
			unsigned char  b = p/100;
			b=b&0xf;
			sprintf(&Mes[n*2],"%1X",p/100);
			Mes[n*2+1]=' ';
    	p=get();
    	n++;
    	if (n>=64)
    		{
    			Mes[n*2]='\n' ;
    			Mes[n*2+1]=0;
    			mes += Mes;
    			n=0;	
    		}
		}
		Mes[n*2]=0;
	  if (n!=0) 
		{
			Mes[n * 2] = '\n';
			Mes[n * 2 + 1] = 0;
			mes += Mes;
		}
		return mes;
}

