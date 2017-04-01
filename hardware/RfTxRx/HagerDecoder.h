//minimum pulse in micros
#define PULSE_MIN 100
//maximum pulse in micros
#define PULSE_MAX 700
//short pulse duration 
#define PULSE_SHORT 300
//preamble size in pulse
#define PREAMBLE  32
// numeber of message bits
#define MESSAGE_BIT  65

//100
#define HORS_GEL  0x4 
//000
#define CONFOR    0x0
//011
#define ECO       0x3
//101
#define ARRET     0x5
 
//mode = bit 0..2 du  data[6]
//retunr HORS_GEL/...ARRET
byte GetMode(const byte* data)
{
	return (data[6]&0x7) ;
}

/*bit data[6]
7					6				5				4				3				2				1				0
Type Programme		Zone 3	Zone 2	Zone 1	------Mode ------

*/

//programmation auto
#define PROGRAM			0
//derogation temporaire	
#define DEROGATION 	2	
//permanent
#define PERMANENT		3			

int getProgramType(const byte* data)
{
	//bit 7-6
	int pg = (data[6]& 0xc0) ;
	pg = pg >> 6 ;
	return pg;
}

//Zone = de  data[6] bit 3  = Zone 1 bit 4  = Zone 2 bit 5  = Zone 3
byte GetZone(const byte* data)
{
	byte Zone = data[6] ;
  if (Zone & (1<<3)) return 1 ;
  if (Zone & (1<<4)) return 2 ;
  if (Zone & (1<<5)) return 3 ;
  return 1;	
}



class HagerDecoder : public DecodeOOK {
public:
    HagerDecoder() {}
    
    // add one bit to the packet data buffer
    virtual void gotBit (char value) {
        //ignore first bit
        if (total_bits>0)
          data[pos] = (data[pos] << 1) | (value ? 0x1 : 00);
        total_bits++;
        pos = (total_bits-1)>>3;

        if (pos >= sizeof data) {
            resetDecoder();
            return;
        }
        state = OK;
    }
    
    virtual int decode (word width) {
        if (PULSE_MIN <= width && width < PULSE_MAX) {
            byte w = width >= PULSE_SHORT;
            switch (state) {
                case UNKNOWN: //wait preamble
                    if (w != 0) { // Long pulse : , start bit : wait 
                        if ( flip >= PREAMBLE )
                          state = T1;
                        flip = 0;
                    } else if (w == 0 ) { // Short pulse
                        ++flip;
                    } 
                    break;
                case T1: //wait 2 long
                if (w != 0) {
                      flip++;
                      if (flip>=1){
                        state = T2;
                        flip = 0;
                      }
                } else   // Reset decoder
                    return -1;
                break;
                case T2: //wait //2 short
                if (w == 0) {
                      flip++;
                      if (flip>=2){
                        state = T3;
                        flip = 0;
                      }
                } else   // Reset decoder
                    return -1;
                break;
                case T3: //wait //2 long
                if (w != 0) {
                      flip++;
                      if (flip>=2){
                        state = T0;
                        flip = 0;
                      }
                } else   // Reset decoder
                    return -1;
                break;
                
                case OK:
                    if (w == 0) {
                        // Short pulse
                        state = T0;
                    } else {
                        // Long pulse
                        manchester(1);
                        state = OK;
                    }
                    break;
                case T0:
                    if (w == 0) { // Second short pulse
                        manchester(0);
                    } else   // Reset decoder
                      return -1;
                    break;
            }
        } else {
            return -1;
        }
        return total_bits == MESSAGE_BIT ? 1: 0;
    }

    byte Sum()
    {
      byte s = 0;
     
      for(byte i = 0; i<total_bits>>3;i++)
      {
        s += data[i];
      }
      return (s);
    }

    bool IsValid() 
    {
      return ( Sum()==0);
    }

    void reportSerial(  ) {
    Serial.print(millis() / 1000);
    Serial.print(" Hager");
    Serial.print(' ');

    for (byte i = 0; i < pos; i++) {
        Serial.print(data[i] >> 4, HEX);
        Serial.print(data[i] & 0x0F, HEX);
        if (i==3) Serial.print(' ');
    }

    Serial.print(" addr:");
    for (byte i = 0; i < 5; i++) {
        Serial.print(data[i] >> 4, HEX);
        Serial.print(data[i] & 0x0F, HEX);
    }
    Serial.print(" cmd:");
    Serial.print(data[5] >> 4, HEX);
    Serial.print(data[5] & 0x0F, HEX);
    Serial.print(" data:");
    Serial.print(data[6] >> 4, HEX);
    Serial.print(data[6] & 0x0F, HEX);
    
    Serial.print(" ");
    byte i = 8 ;
    do  {
        i--;
    	  if (data[6]& (1<<i)) 
        	Serial.print('1');
        else
        	Serial.print('0');
				if ( (i==6)||(i==3) )Serial.print(" ");

    }while(i!=0);
   	
    Serial.print(" Z");
    Serial.print(GetZone(data)) ;

    if (GetMode(data) == HORS_GEL ) Serial.print(" HORS_GEL");
    if (GetMode(data) == CONFOR   ) Serial.print(" CONFOR");
    if (GetMode(data) == ECO      ) Serial.print(" ECO");
    if (GetMode(data) == ARRET    ) Serial.print(" ARRET");

    if (getProgramType(data) == PROGRAM    ) Serial.print(" PROGRAM");
    if (getProgramType(data) == DEROGATION ) Serial.print(" DEROGATION");
    if (getProgramType(data) == PERMANENT  ) Serial.print(" PERMANENT");

    
    Serial.println();
}
    
};
