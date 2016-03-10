//file simulate Arduino Serial Print 
#ifndef Print_h
#define Print_h

#include <stdio.h> // for int
#include <string.h> // for int

#include "deftype.h"


#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class Print
{
  public:
  
	static int PRINT (long mes , int base , bool lf)
	{
			switch (base) {
					case BIN : printf("%b",mes) ;
					break;
					case OCT:  printf("%o",mes) ;
					break;
					case DEC : printf("%d",mes) ;
					break;
					case HEX : printf("%X",mes) ;
					break;
					default  : printf("%d",mes) ;
					break;
			}
			if (lf)printf("\n") ;
			return 1;
	}

		
		static int write(uint8_t mes ) {printf("%02X",mes) ; };

    static int write(const char *buffer, int size) {
      for (int i=0;i<size;i++) write(buffer[i]);
    }

    static int write(const char *str) {
      if (str == NULL) return 0;
      return write( str, strlen(str));
    }
    
    static int print(const char mes [] )                     {return printf("%s",mes) ; };
    static int print(char mes )                              {return printf("%c",mes) ; };
    static int print(unsigned char mes , int base = DEC)     {return PRINT(mes,base,false) ; };
    static int print(int mes , int base = DEC)               {return PRINT(mes,base,false) ; };
    static int print(unsigned int mes, int base = DEC)       {return PRINT(mes,base,false) ; };
    static int print(long mes , int base = DEC)              {return PRINT(mes,base,false) ; };
    static int print(unsigned long mes , int base = DEC)     {return PRINT(mes,base,false) ; };
    static int print(double mes , int base = 2)              {return printf("%f",mes) ; };
    static int println(const char mes[])                     {return printf("%s\n",mes) ; };        
    static int println(char mes )                            {return printf("%c\n",mes) ; };        
    static int println(unsigned char mes , int base = DEC)   {return PRINT(mes,base,true) ; };   
    static int println(int mes , int base = DEC)             {return PRINT(mes,base,true) ; };   
    static int println(unsigned int mes , int base = DEC)    {return PRINT(mes,base,true) ; };   
    static int println(long mes , int base = DEC)            {return PRINT(mes,base,true) ; };   
    static int println(unsigned long mes , int base  = DEC)  {return PRINT(mes,base,true) ; };   
    static int println(double mes , int base = 2)            {return printf("%f\n",mes) ; };        
    static int println(void)																 {return printf("\n") ; };        
};

extern Print Serial ;

#endif