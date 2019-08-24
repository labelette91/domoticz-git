
#if (_MSC_VER < 1300)
   typedef signed char       int8_t;
   typedef signed short      int16_t;
   typedef signed int        int32_t;
   typedef unsigned char     uint8_t;
   typedef unsigned short    uint16_t;
   typedef unsigned int      uint32_t;
#else
   typedef signed __int8     int8_t;
   typedef signed __int16    int16_t;
   typedef signed __int32    int32_t;
   typedef unsigned __int8   uint8_t;
   typedef unsigned __int16  uint16_t;
   typedef unsigned __int32  uint32_t;
#endif
typedef signed __int64       int64_t;
typedef unsigned __int64     uint64_t;


const uint8_t maskArray[8] = { 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };

uint16_t BitIdxToByte(const uint16_t &bitIdx)
{
	if ((bitIdx % 8) == 0)
		return bitIdx / 8;
	return (bitIdx + (8 - (bitIdx % 8))) / 8;
}

//return the value at bit offset offset length : size
//as described in eep profile 
uint32_t GetRawValue(uint8_t * data ,  uint16_t offset, uint8_t size)
{
  uint32_t value = 0 ;
	if (size > 32)
		return value ;

	uint16_t idx = (uint16_t)(offset / 8);
	uint16_t idxe = BitIdxToByte(offset + size);
	uint16_t len = idxe - idx;
	uint64_t tmpLongValue=0;

	uint8_t bits = (offset % 8);
	uint8_t bite = 0;
	if (((offset + size) % 8) != 0)
		bite = (8 - ((offset + size) % 8));

	uint8_t mask = maskArray[bits];

	switch (len)
	{
		case 1:
			value = (data[idx] & mask);
			break;
		case 2:
			value = (data[idx] & mask) << 8 | data[idx + 1];
			break;
		case 3:
			value = (data[idx] & mask) << 16 | data[idx + 1] << 8 | data[idx + 2];
			break;
		case 4:
			value = (data[idx] & mask) << 24 | data[idx + 1] << 16 | data[idx + 2] << 8 | data[idx + 3];
			break;
		case 5:
			tmpLongValue = (uint64_t)(data[idx] & mask) << 32 | (uint64_t)(data[idx + 1]) << 24 | (uint64_t)(data[idx + 2]) << 16 | (uint64_t)(data[idx + 3]) << 8 | (uint64_t)(data[idx + 4]);
			break;
	}

	if (len == 5)
	{
		value = tmpLongValue >> bite;
	}
	else
	{
		value >>= bite;
	}

	return value;
}


//copy the value at bit offset offset length : size
//as described in eep profile 
//return true if ok
bool SetRawValue(uint8_t * data , uint32_t value, uint16_t offset, uint8_t size)
{
	if (size == 0)
		return false;
	if (size >= 32 )
		return false;

	if (((offset + size) / 8) > MAX_DATA_PAYLOAD)
		return false;

	uint8_t idx = (uint8_t)(offset / 8);
	uint16_t idxe = BitIdxToByte((offset + size));
	uint8_t len = idxe - idx;

	uint8_t bits = (offset % 8);
	uint8_t bite = 0;
	if (((offset + size) % 8) != 0)
		bite = (8 - ((offset + size) % 8));

	uint8_t mask = maskArray[bits];

	uint64_t tmpLongValue=0;
    //left shift the not used btis are zero
    if (len == 5)
    {
    	tmpLongValue = value;
    	tmpLongValue <<= bite;
    }
	value <<= bite;	//left shift the not used btis are zero
	switch (len)
	{
		case 1:
		{
			//first set to zero
			data[idx] &= ~(mask & (0xFF << bite));
			data[idx] |= (value & mask);
			break;
		}
		case 2:
		{
			data[idx] &= ~mask;
			data[idx] |= (value >> 8) & mask;
			data[idx + 1] &= ~(0xFF << bite);
			data[idx + 1] |= value;
			break;
		}
		case 3:
		{
			data[idx] &= ~mask;
			data[idx] |= (value >> 16) & mask;
			data[idx + 1] = 0;
			data[idx + 1] |= value >> 8;
			data[idx + 2] &= ~(0xFF << bite);
			data[idx + 2] |= value;
			break;
		}
		case 4:
		{
			data[idx] &= ~mask;
			data[idx] |= (value >> 24) & mask;
			data[idx + 1] = 0;
			data[idx + 1] |= value >> 16;
			data[idx + 2] = 0;
			data[idx + 2] |= value >> 8;
			data[idx + 3] &= ~(0xFF << bite);
			data[idx + 3] |= value;
			break;
		}
        case 5:
            {
                data[idx] &= (~mask);
                data[idx] |=((tmpLongValue >> 32) & mask);
                data[idx + 1] = 0;
                data[idx + 1] |= (tmpLongValue >> 24);
                data[idx + 2] = 0;
                data[idx + 2] |= (tmpLongValue >> 16);
                data[idx + 3] = 0;
                data[idx + 3] |= (tmpLongValue >> 8);
                data[idx + 4] &= (~(0xFF << bite));
                data[idx + 4] |= tmpLongValue;
                break;
            }
	}
	return true;
}

T_DATAFIELD* GetOffsetFromName( char * OffsetName , T_DATAFIELD * OffsetDes )
{
  uint32_t offsetInd = 0 ;
  while (OffsetDes[offsetInd].Size != 0 )
  {
    if (strstr(OffsetDes[offsetInd].ShortCut,OffsetName ) != 0 )
      return &OffsetDes[offsetInd] ;
    offsetInd++;
  }
  return &OffsetDes[offsetInd] ;

}

bool SetRawValue(uint8_t * data , uint32_t value, T_DATAFIELD* offset )
{
  return SetRawValue( data , value, offset->Offset,offset->Size ) ;

}

uint32_t GetRawValue(uint8_t * data ,   T_DATAFIELD* offset )
{
  return GetRawValue( data ,  offset->Offset,offset->Size ) ;
}

uint32_t GetRawValue(uint8_t * data ,  T_DATAFIELD* offset , uint32_t offsetIndex )
{
  return GetRawValue( data ,  offset[offsetIndex].Offset,offset[offsetIndex].Size ) ;
}


bool SetRawValue(uint8_t * data , uint32_t value, char *  OffsetName , T_DATAFIELD * OffsetDes )
{
  T_DATAFIELD* offset =  GetOffsetFromName(  OffsetName ,  OffsetDes ) ;
  return SetRawValue( data , value, offset->Offset, offset->Size ) ;

}

uint32_t GetRawValue(uint8_t * data ,  char *  OffsetName , T_DATAFIELD * OffsetDes )
{
  T_DATAFIELD* offset =  GetOffsetFromName(  OffsetName ,  OffsetDes ) ;
  return GetRawValue( data ,  offset->Offset, offset->Size ) ;
}


#include <stdarg.h>
//return the number of byte of data payload
//0 if rror
uint32_t SetRawValuesNb(uint8_t * data , T_DATAFIELD * OffsetDes ,int NbParameter , va_list value )
{

   for ( int i=0;i<NbParameter;i++)
   {
      if  ( OffsetDes->Size == 0 )
        return 0 ; //erreur

      uint32_t par = va_arg(value,int);       /*   va_arg() donne le paramètre courant    */
      SetRawValue( data, par  , OffsetDes ) ;
      OffsetDes++;
   }

   //test if all variable are sets
   if (OffsetDes->Size != 0)
	   return 0; //erreur
   //last bit offser
   OffsetDes--;
   uint32_t total_bits  = OffsetDes->Offset + OffsetDes->Size;
   uint32_t total_bytes = (total_bits + 7) / 8;

   return total_bytes ;
}


uint32_t SetRawValuesNb(uint8_t * data, T_DATAFIELD * OffsetDes, int NbParameter, ...)
{
	va_list value;

	/* Initialize the va_list structure */
	va_start(value, NbParameter);
	uint32_t total_bytes = SetRawValuesNb(data, OffsetDes, NbParameter, value);
	va_end(value);

	return total_bytes;
}



//return the number of byte of data payload
//0 if error

uint32_t SetRawValues(uint8_t * data, T_DATAFIELD * OffsetDes,  va_list value)
{

	while (OffsetDes->Size != 0)
	{

		int par = va_arg(value, int);       /*   va_arg() donne le paramètre courant    */
		//not enough argument
		if (par == END_ARG_DATA)
			return 0;
		SetRawValue(data, par, OffsetDes);
		OffsetDes++;
	}

	int par = va_arg(value, int);       
	if (par != END_ARG_DATA)
		return 0;

    //last bit offset
	OffsetDes--;
	uint32_t total_bits = OffsetDes->Offset + OffsetDes->Size;
	uint32_t total_bytes = (total_bits + 7) / 8;

	return total_bytes;
}


uint32_t SetRawValues(uint8_t * data, T_DATAFIELD * OffsetDes,  ...)
{
	va_list value;

	/* Initialize the va_list structure */
	va_start(value, OffsetDes);
	uint32_t total_bytes = SetRawValues(data, OffsetDes, value);
	va_end(value);

	return total_bytes;
}



T_DATAFIELD D2_05_00_Cmd_1 [] = {
{  1  , 7 , "POS " , "" },
{  9  , 7 , "ANG " , "" },
{  17 , 3 , "REPO" , "" },
{  21 , 3 , "LOCK" , "" },
{  24 , 4 , "CHN " , "" },
{  28 , 4 , "CMD " , "" },
{  0  , 0 , "    " , "" },


};


