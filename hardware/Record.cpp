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
bool TRecord::empty()
{
return(PWr==PRd);
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

/* std::string TRecord::ToString()
{
  int p=0;
  int n;
  std::string mes ;
  	
  char Mes[128*3];
    p=get();
    n=0;
		while(p!=0) 
		{
			int  b = p/100;
//			b=b&0xf;
			if (b < 10)
				Mes[n * 2] = '0' + b;
			else
			{
				b = b - 10;
				if (b < 26)
					Mes[n * 2] = 'A' + b ;
				else {
					b = b - 26 ;
					if (b < 26)
						Mes[n * 2] = 'a' + b;
					else
						Mes[n * 2] = '*';

				}

			}

//			sprintf(&Mes[n*2],"%1X",b);
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
			mes += Mes;
		}
		return mes;
}
*/

std::string TRecord::ToString()
{
	int p = 0;
	int n;
	std::string mes;

	char Mes[128 * 3];
	p = get();
	n = 0;
	int nbcar = 0;
	while (p != 0)
	{
		int  b = p / 100;

		nbcar += sprintf(&Mes[nbcar],"%d ",b);
		p = get();
		n++;
		if (n >= 64)
		{
			Mes[nbcar++] = '\n';
			Mes[nbcar++] = 0;
			mes += Mes;
			n = 0;
			nbcar = 0;
		}
	}
	if (n != 0)
	{
		mes += Mes;
	}
	return mes;
}