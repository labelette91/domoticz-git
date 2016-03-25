#ifndef _RECORD
#define _RECORD

#include <stdio.h>
#include <string.h>


class TRecord {

public:
void init () ;
void put(int p);
std::string ToString() ;
int get();
bool empty();	
TRecord();
~TRecord();

#define NBPULSE 20000
int Pulse[NBPULSE];
int PWr;
int PRd;
	
};

#endif