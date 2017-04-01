class DecodeOTIO {
public:
    byte total_bits, state, MaxTemp , NbTempOk ;
    long T , Temp , tmpTemp, lastTemp;
    long LastReceived ; 
    byte data[16];
    byte pos;
public:
    enum { UNKNOWN, T0, T1, T2, T3, OK, DONE };

    void resetDecoder () {
          total_bits =  0;
          state = UNKNOWN;
          T = 0 ;
    }    
    //pMaxTemp : the number of successive received equal value to return the current temperature
    //OTIO send 20 frames with the 24 bits value */
    
    DecodeOTIO (byte pMaxTemp=5) { Temp = 0 ; NbTempOk=0;MaxTemp=pMaxTemp;tmpTemp=0;lastTemp=0;LastReceived=0;resetDecoder(); }

    /* p pulse time in 100 micros unit : return */
    /* 0 = short pulse = 2ms */
    /* 1 = long  pulse = 4ms */
    /* 2 = end   pulse = 8ms */
    byte getPulseBit(byte p)
    {
    if ( p < 25)
      return 0 ;
    else
    if ( p < 65)
      return 1 ;
    else 
      return 2 ;
    }
    
    /* return true if 24 bits have been receive between two 8ms pulse */
    /* result in T */
    bool nextOnePulse (word pWidth , byte data)   {
       //pulse lsb wil be 100 micros to save computation time
       byte width = pWidth / 100;
       /* 1er bit 1500 micros */ 
//       if ( (width>15)&& (data==1) )   state = T0 ;
       if ( (width>15)&& (data==1) )   state = T0 ;
      /* bit next 1000 micros  */
      if ( (width>10)&&(state == T0) )
      {
        total_bits++;
        /*  end of word and correct number of bit pulse shal be 8ms   */
//        if (( width > 60)&&( width < 100) )
        if (( width > 60) )
        {
          if (total_bits==25)
          {
            //update temperature
            tmpTemp = T ;
            resetDecoder ();
            return true;
          }
          else
          {
            resetDecoder ();
            return false;
          }
        } 
        else
        {
          /* add bit to temp */
          T = T << 1;
          T = T | getPulseBit(width);
        }
        if (total_bits>=25)
            resetDecoder ();
    }
    return false;   
  }

    /* return true if successive equal MaxTemp 24 bits words have been receive between two 8ms pulse */
    /* result in Temp */
    
  bool nextPulse (word width , byte data)   {
    //if one frame has been received
    if (nextOnePulse ( width ,  data) ){
       if (NbTempOk==0) lastTemp = tmpTemp ;
       //if same value increment counter or decrement
       if (lastTemp == tmpTemp ) NbTempOk ++ ; else NbTempOk -- ;
       //if max number of same frame has been received
       if (NbTempOk >= MaxTemp ){
            //set final value
            Temp = lastTemp;
            LastReceived = millis();
            NbTempOk = 0 ;
            buildDataBuffer();
            return true;
       } 
    }
    return false ;
  }  
  

/*
frame  0 = short pulse = 2ms 
       1 = long  pulse = 4ms 
       2 = end   pulse = 8ms 
    --IDENT-------    SIGN- -----TEMP---    -BAT---   --FRAC-    End Temp
    2 2 2 2 1 1 1 1   1     1 1 1 1 1 0 0   0 0 0 0   0 0 0 0 
BIT 3 2 1 0 9 8 7 6   5     4 3 2 1 0 9 8   7 6 5 4   3 2 1 0     
    1 0 0 1 0 1 0 1   1     0 0 0 1 0 1 1   0 0 0 0   0 0 1 0    2  -11.2
    1 0 0 1 0 1 0 1   1     0 0 0 0 0 0 0   0 0 0 0   0 0 1 0    2  -0.2                                                      
    1 0 0 1 0 1 0 1   0     0 0 0 0 0 0 1   0 0 0 0   0 0 1 0    2  01.2                                                      
    1 0 0 1 0 1 0 1   0     0 0 0 1 0 0 0   0 0 0 0   0 0 1 0    2  08.2
    1 0 0 1 0 1 0 1   0     0 0 0 1 0 1 0   0 0 0 0   0 1 0 1    2  10.5
*/
  /* temperature in Lsb = 0.1 degrec C */
  int getTemperature(){
    int  t = Temp ;
    byte frac = t & 0xF ;
    t = t >>8 ;
    byte signe = t &  0x80 ;
    /* partie entiere */
    t = t & 0x7F;
    /* partie frac */
    t = t*10  ;
    t = t  + frac ;
    if (signe)
      t = -t;
    return t;        
    
  }
//return identification 0x00 to 0xff
  byte getId(){
    return (Temp >> 16) ;        
  }
//return 1 if batterie OK  
  byte getBatteryOk(){
    return ( (Temp &  0x80 )!= 0 ) ;
  }
  
  char * ReportSerial(){
    static char buf[128];
    int t = getTemperature();;
    int frac = t % 10; if (frac<0)frac = -frac;

    sprintf(buf, "OTIO: %04X T:%d.%d ID:%02X %d", Temp, t / 10, frac, getId(), getBatteryOk());
    return buf;

  }

  const byte* getData() const {
    return data;
  }
  byte getBytes() {
    return pos;
  }

  void buildDataBuffer()
  {
    //Id 0x3C00

    data[0] = 0x3A;  //nible inverse + synchro A : A3C000  --> 3A 0c 00 //synchro
    data[1] = 0x0c;
    data[2] = 00;
    //rolling code ID
    data[3] = getId();
    //temp value 
    short int temperature = getTemperature();
    data[4] = temperature >>8 ;
    data[5] = temperature & 0xFF ;

    //baterie
    data[6] = getBatteryOk();
    pos = 7 ;

  }
};






/* 24 bit decodage 
   --IDENT-------    SIGN-   -TEMP---    -BAT---   --FRAC-     DEG
25 1 0 0 1 0 1 0 1   1 0 0   0 1 0 1 1   0 0 0 0   0 0 1 0 88  -11.2
25 1 0 0 1 0 1 0 1   1 0 0   0 1 0 0 1   0 0 0 0   0 0 1 1 88  -9.3
25 1 0 0 1 0 1 0 1   1 0 0   0 1 0 0 0   0 0 0 0   0 0 1 1 87  -8.3
25 1 0 0 1 0 1 0 1   1 0 0   0 0 1 1 1   0 0 0 0   0 0 1 1 87  -7.3
25 1 0 0 1 0 1 0 1   1 0 0   0 0 1 0 0   0 0 0 0   0 1 0 0 87  -4.4                                                      
25 1 0 0 1 0 1 0 1   1 0 0   0 0 0 1 1   0 0 0 0   0 1 0 1 88  -3.5                                                      
25 1 0 0 1 0 1 0 1   1 0 0   0 0 0 0 0   0 0 0 0   0 0 1 0 87  -0.2                                                      
25 1 0 0 1 0 1 0 1   0 0 0   0 0 0 0 0   0 0 0 0   0 1 0 1 87   0.5                                                      
25 1 0 0 1 0 1 0 1   0 0 0   0 0 0 0 1   0 0 0 0   0 0 1 0 88  01.2                                                      
25 1 0 0 1 0 1 0 1   0 0 0   0 0 0 0 1   0 0 0 0   1 0 0 0 87  01.8                                                      
25 1 0 0 1 0 1 0 1   0 0 0   0 0 0 1 0   0 0 0 0   0 1 0 0 87  02.4                                                      
25 1 0 0 1 0 1 0 1   0 0 0   0 0 0 1 0   0 0 0 0   0 0 1 0 87  02.2
25 1 0 0 1 0 1 0 1   0 0 0   0 0 0 1 1   0 0 0 0   0 1 0 0 86  03.4
25 1 0 0 1 0 1 0 1   0 0 0   0 0 0 1 1   0 0 0 0   1 0 0 1 87  03.9
25 1 0 0 1 0 1 0 1   0 0 0   0 0 1 0 0   0 0 0 0   0 1 0 0 87  04.4
25 1 0 0 1 0 1 0 1   0 0 0   0 0 1 0 1   0 0 0 0   0 0 0 0 89  05.0
25 1 0 0 1 0 1 0 1   0 0 0   0 0 1 0 1   0 0 0 0   1 0 0 1 87  05.9
25 1 0 0 1 0 1 0 1   0 0 0   0 0 1 1 0   0 0 0 0   0 1 0 0 87  06.4
25 1 0 0 1 0 1 0 1   0 0 0   0 0 1 1 0   0 0 0 0   1 0 0 0 88  06.8
25 1 0 0 1 0 1 0 1   0 0 0   0 0 1 1 1   0 0 0 0   0 1 0 1 88  07.5
25 1 0 0 1 0 1 0 1   0 0 0   0 0 1 1 1   0 0 0 0   1 0 0 1 86  07.9
25 1 0 0 1 0 1 0 1   0 0 0   0 1 0 0 0   0 0 0 0   0 0 1 0 88  08.2
25 1 0 0 1 0 1 0 1   0 0 0   0 1 0 0 0   0 0 0 0   0 1 0 1 87  08.5
25 1 0 0 1 0 1 0 1   0 0 0   0 1 0 0 0   0 0 0 0   1 0 0 1 87  08.9
25 1 0 0 1 0 1 0 1   0 0 0   0 1 0 0 1   0 0 0 0   0 0 1 0 87  09.2
25 1 0 0 1 0 1 0 1   0 0 0   0 1 0 0 1   0 0 0 0   0 1 0 1 86  09.5
25 1 0 0 1 0 1 0 1   0 0 0   0 1 0 1 0   0 0 0 0   0 1 0 1 87  10.5
25 1 0 0 1 0 1 0 1   0 0 0   0 1 0 1 1   0 0 0 0   0 1 1 0 87  11.6
25 1 0 0 1 0 1 0 1   0 0 0   0 1 1 0 0   0 0 0 0   0 0 0 0 87  12.0
25 1 0 0 1 0 1 0 1   0 0 0   0 1 1 0 0   0 0 0 0   0 0 1 1 87  12.3
25 1 0 0 1 0 1 0 1   0 0 0   0 1 1 0 0   0 0 0 0   0 1 1 0 87  12.6
25 1 0 0 1 0 1 0 1   0 0 0   0 1 1 0 1   0 0 0 0   0 0 1 1 86  13.7
25 1 0 0 1 0 1 0 1   0 0 0   0 1 1 0 1   0 0 0 0   0 1 1 0 87  13.6
25 1 0 0 1 0 1 0 1   0 0 0   0 1 1 0 1   0 0 0 0   1 0 0 1 87  13.9
25 1 0 0 1 0 1 0 1   0 0 0   0 1 1 1 0   0 0 0 0   0 0 1 0 86  14.2
25 1 0 0 1 0 1 0 1   0 0 0   0 1 1 1 1   0 0 0 0   0 0 0 1 88  15.1
25 1 0 0 1 0 1 0 1   0 0 0   0 1 1 1 1   0 0 0 0   0 1 0 0 88  15.4
25 1 0 0 1 0 1 0 1   0 0 0   1 0 0 0 0   0 0 0 0   0 0 0 1 87  16.1
25 1 0 0 1 0 1 0 1   0 0 0   1 0 0 0 0   0 0 0 0   0 1 1 1 87  16.7
25 1 0 0 1 0 1 0 1   0 0 0   1 0 0 1 0   0 0 0 0   0 0 0 0 88  18.0
25 1 0 0 1 0 1 0 1   0 0 0   1 0 0 1 1   0 0 0 0   0 0 0 1 87  19.1
25 1 0 0 1 0 1 0 1   0 0 0   1 0 1 0 1   0 0 0 0   0 0 0 0 87  21.0
*/


/* pulse in 100micros unit 
short : 2000..2200 micros
long  : 4000..4200 micros
end   :8700..8800    

   42 21 20 41 21 41 21 41 21 21 20 42 21 41 41 20 21 41 21 20 42 21 21 40 
87 41 21 20 42 20 42 20 42 21 21 20 42 20 42 41 21 20 42 20 21 41 21 20 40 
87 41 20 21 43 20 41 21 43 20 21 21 42 20 42 41 22 20 41 21 21 41 21 21 40 
87 41 21 20 41 19 42 20 42 20 22 20 42 19 42 41 21 19 42 20 21 41 22 20 39 
88 41 20 21 42 20 41 20 42 21 20 21 42 20 41 42 21 20 58 21 20 42 20 21 40 
87 41 20 21 41 20 41 22 41 20 21 21 41 21 41 42 20 20 41 22 20 42 21 21 40 
87 42 20 21 41 21 41 21 40 21 20 21 41 22 41 42 20 21 41 21 19 42 20 21 39 
87 42 20 21 41 21 41 21 41 21 20 21 41 21 40 42 21 21 41 21 20 42 20 21 40 
87 41 20 21 41 21 41 22 41 21 21 21 41 21 41 42 20 21 41 22 20 42 21 21 40 
87 42 20 21 41 21 41 21 40 21 20 21 41 22 41 42 20 21 41 21 19 42 20 21 39 
87 42 20 21 41 21 41 42 41 22 20 21 41 21 41 41 19 22 41 21 21 43 20 21 40 
87 42 20 21 41 21 40 21 41 21 20 22 41 21 41 42 20 21 40 21 20 42 20 22 40 
87 42 21 21 41 21 41 21 41 21 21 22 40 21 41 43 20 21 41 21 20 42 21 21 39 
87 42 20 21 41 22 41 21 41 22 20 21 41 21 41 41 19 22 41 21 21 43 20 21 40 
87 42 20 21 41 21 40 21 41 21 19 22 41 21 41 42 20 21 40 21 20 41 20 20 40 
87 42 21 21 41 21 41 21 41 21 21 21 40 21 41 43 20 21 41 21 20 42 21 21 56 
87 41 20 21 41 20 41 21 41 20 20 21 42 20 42 42 21 20 41 21 20 41 20 21 39 
87 40 21 20 41 21 42 20 41 20 21 20 41 21 42 41 20 21 42 20 20 40 21 20 40 
87 42 20 20 42 21 41 20 42 21 20 20 42 20 41 41 21 21 41 20 21 42 20 20 41 
87 41 20 21 43 20 41 21 42 20 20 21 43 20 42 41 21 20 41 21 21 41 20 21 40 
87 40 21 20 41 20 42 20 41 20 21 20 41 

*/
