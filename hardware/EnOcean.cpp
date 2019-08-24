#include "stdafx.h"
#include "EnOcean.h"
#include "../main/Logger.h"
#include "../main/Helper.h"
#include "../main/RFXtrx.h"
#include "../main/SQLHelper.h"

#include <string>

#define S_RPS_T21 0x20
#define S_RPS_T21_SHIFT 5
#define S_RPS_NU  0x10
#define S_RPS_NU_SHIFT 4
#define S_RPS_RPC 0x0F
#define S_RPS_RPC_SHIFT 0


CEnOcean::CEnOcean() {
	m_id_base = 0;
	m_Seq = 0 ;

};
//return true if base adress reading
bool CEnOcean::IsRunning()
{
	return (m_id_base != 0);
}
//id offset of device
//src : source rocker
unsigned int CEnOcean::GetAdress(int unitid) {
	return(m_id_base + unitid);
}

unsigned int CEnOcean::GetOffsetAdress(int unitid) {
	return ( unitid- m_id_base) ;
}

uint64_t CEnOcean::CreateDevice(const int HardwareID, const char* ID, const int  unitCode, const unsigned char devType, const unsigned char subType, const unsigned char signallevel, const unsigned char batterylevel, const int nValue, const char* sValue, std::string &devname, int SwitchType , const std::string & deviceoptions)
{
	uint64_t ulID = 0;
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query("SELECT ID,Name, Used, SwitchType, nValue, sValue, LastUpdate, Options, Log FROM DeviceStatus WHERE (HardwareID=%d AND DeviceID='%q' AND Unit=%d AND Type=%d AND SubType=%d)", HardwareID, ID, unitCode, devType, subType);
	if (result.size() == 0)
	{
		//Insert

		if (!m_sql.m_bAcceptNewHardware)
		{
			devname = "Ignored";
			return -1; //We do not allow new devices
		}

		if (devname != "")
			devname = "Unknown" + std::string(ID);
		m_sql.safe_query(
			"INSERT INTO DeviceStatus (HardwareID, DeviceID, Unit, Type, SubType, SignalLevel, BatteryLevel, nValue, sValue,Name,used,SwitchType) "
			"VALUES ('%d','%q','%d','%d','%d','%d','%d','%d','%q','%q',1,'%d' )",
			HardwareID,
			ID, unitCode, devType, subType,
			signallevel, batterylevel,
			nValue, sValue, devname.c_str(),
			SwitchType);

		//Get new ID
		result = m_sql.safe_query(
			"SELECT ID FROM DeviceStatus WHERE (HardwareID=%d AND DeviceID='%q' AND Unit=%d AND Type=%d AND SubType=%d)",
			HardwareID, ID, unitCode, devType, subType);
		if (result.size() == 0)
		{
			_log.Log(LOG_ERROR, "Serious database error, problem getting ID from DeviceStatus!");
			return -1;
		}
		std::stringstream s_str(result[0][0]);
		s_str >> ulID;

		//Set device options
		//deviceoptions.append("SelectorStyle:0;LevelNames:Off|Level1|Level2|Level3");
		if (!deviceoptions.empty()) 
			m_sql.SetDeviceOptions(ulID , m_sql.BuildDeviceOptions(deviceoptions, false));

	}
	return ulID;
}

//return the unit that correspond to the offset from controler base adress
int CEnOcean::getUnitFromDeviceId(unsigned int devIDx )
{
	return getUnitFromDeviceId(DeviceIDToString(devIDx) );
}
int CEnOcean::getUnitFromDeviceId(std::string devIDx  )
{
	std::vector<std::vector<std::string> > result;

	ToSensorsId(devIDx);

	//get Dev UnitCode 

	result = m_sql.safe_query("SELECT Address FROM EnoceanSensors WHERE (DeviceID='%s')  ", devIDx.c_str() );
	if (result.size() > 0) {
	return  atoi(result[0][0].c_str());
	}
	else
	return 0 ;
}


//return the sender adress fro enOcean deviceId 0x12345678
unsigned int CEnOcean::getSenderAdressFromDeviceId(std::string devIDx)
{
	int unitId = getUnitFromDeviceId(devIDx);
	unsigned int SenderAdress = GetAdress(unitId);
	return SenderAdress;
}

unsigned int CEnOcean::getSenderAdressFromDeviceId(unsigned int devIDx)
{
	return getSenderAdressFromDeviceId(DeviceIDToString(devIDx));
}
//DeviceId : ID of device in EnoceanSensor
//offsetID : offset of device from controler base adress 0..127
void CEnOcean::UpdateBaseAddress(std::string DeviceId , int offsetID ) {
	m_sql.safe_query("UPDATE EnoceanSensors SET %s=%d   WHERE (DeviceID = '%s' )", "Address" , offsetID , DeviceId.c_str() );

}

//find a base adress for enOcean device DeviceId : 0x12345678 
int CEnOcean::UpdateDeviceAddress(std::string DeviceId) {
	std::vector<std::vector<std::string> > result;
	bool  UsedUnitId[MAX_BASE_ADDRESS + 1];

	memset(UsedUnitId, 0, sizeof(UsedUnitId));
	//convert to 8 char
	ToSensorsId(DeviceId);

	//search if a same device ID already allocated exist
	result = m_sql.safe_query("SELECT Address FROM EnoceanSensors WHERE (DeviceId='%s') and (HardwareId=%d)  ", DeviceId.c_str(), m_HwdID);
	for (unsigned int i = 0; i < result.size(); i++)
	{
		//take the same
		int unitId = atoi(result[i][0].c_str());
		if (unitId != 0) {
			//UpdateBaseAddress(DeviceId, unitId );
			return unitId ;
		}
	}

	//get list of enocean sensor device
	result = m_sql.safe_query("SELECT Address FROM EnoceanSensors WHERE (HardwareId=%d)  ",  m_HwdID );
	//get all BaseId allready affected to switch device
	for (unsigned int i = 0; i < result.size(); i++)
	{
		int unitId = atoi(result[i][0].c_str());
		unitId = unitId % MAX_BASE_ADDRESS ;//robustess
		//ID already used
		UsedUnitId[unitId] = true;
	}

	//get list of device enocean created
	result = m_sql.safe_query("SELECT DeviceId FROM DeviceStatus  WHERE (HardwareId=%d)  ", m_HwdID);
	//get all BaseId allready affected to switch device
	for (unsigned int i = 0; i < result.size(); i++)
	{
		int unitId = atoi(result[i][0].c_str());
		//if it is a gateWay range adress
		if (CheckIsGatewayAdress(unitId))
		{
			unitId = GetOffsetAdress(unitId) % MAX_BASE_ADDRESS;//robustess
			//ID already used
			UsedUnitId[unitId] = true;
		}
	}

	//find not used addess
	for (int i = 1; i < MAX_BASE_ADDRESS; i++)
	{
		if (UsedUnitId[i] == false) {
			UpdateBaseAddress(DeviceId, i);
			return i ;
		}
	}
	//device full 
	return 0;

}

int CEnOcean::UpdateDeviceAddress(unsigned int  DeviceId) 
	{
		return (UpdateDeviceAddress(DeviceIDToString(DeviceId)) );
	}
//test if deviceId exist in in database
//return 0 if not found
int CEnOcean::DeviceExist( unsigned int Deviceid)
{
return DeviceExist((char *)DeviceIDToString(Deviceid).c_str());
}

int CEnOcean::DeviceExist(char * szDeviceID)
{

	std::vector<std::vector<std::string> > result;

	result = m_sql.safe_query("SELECT ID  FROM EnoceanSensors WHERE (DeviceID=='%q')", szDeviceID);
	//not found
	if (result.size() < 1)
		return 0;
	else
		return atoi(result[0][0].c_str());


}

//create sensor in database
void CEnOcean::CreateSensors(char * szDeviceID, int rorg , int manufacturer, int profile, int ttype)
{
	m_sql.safe_query("INSERT INTO EnoceanSensors (HardwareID, DeviceID, Manufacturer, Rorg,Profile, [Type]) VALUES (%d,'%q',%d,%d,%d,%d)", m_HwdID, szDeviceID, manufacturer, rorg,profile, ttype);

}
void CEnOcean::CreateSensors(unsigned int DeviceID, int rorg, int manufacturer, int profile, int ttype)
{
	CreateSensors((char *)DeviceIDToString(DeviceID).c_str(), rorg, manufacturer, profile, ttype);
}


//update profile  sensor in database
void CEnOcean::UpdateProfileSensors(char * szDeviceID, int rorg,  int profile, int ttype)
{
	m_sql.safe_query("UPDATE EnoceanSensors SET  Rorg=%d , Profile=%d , Type=%d  WHERE (DeviceID = '%s' )", rorg, profile, ttype, szDeviceID);

}



void CEnOcean::AddSensors(unsigned int DeviceID, int manufacturer, int profile, int ttype)
{

	if (!DeviceExist(DeviceID))
	{
		// If not found, add it to the database
		CreateSensors(DeviceID, 0,manufacturer, profile, ttype);
		_log.Log(LOG_NORM, "EnOcean: Sender_ID 0x%08X inserted in the database", DeviceID);
	}
	else
		_log.Log(LOG_NORM, "EnOcean: Sender_ID 0x%08X already in the database", DeviceID);
}

void CEnOcean::AddSensors(unsigned int DeviceID, int manufacturer, int profile, int ttype , int OffsetAddr )
{
	AddSensors(DeviceID, manufacturer, profile, ttype);

	UpdateBaseAddress(DeviceIDToString(DeviceID).c_str(),  OffsetAddr );
}

std::string CEnOcean::DeviceIDToString(unsigned int DeviceID)
{
	char szDeviceID[20];
	DeviceIDIntToChar(DeviceID, szDeviceID);
	return szDeviceID;
}

void ToSensorsId(std::string &DeviceId)
{
//	while (DeviceId.size() < 8)
//		DeviceId = '0' + DeviceId;
}

//convert device ID id from  buffer[] to unsigned int
unsigned int DeviceArrayToInt(unsigned char m_buffer[])
{
	unsigned int id = (m_buffer[0] << 24) + (m_buffer[1] << 16) + (m_buffer[2] << 8) + m_buffer[3];
	return id;
}

//convert device ID id from   unsigned int to buffer[]  
void  DeviceIntToArray(unsigned int sID, unsigned char buf[])
{

buf[0] = (sID >> 24) & 0xff;
buf[1] = (sID >> 16) & 0xff;
buf[2] = (sID >> 8) & 0xff;
buf[3] = sID & 0xff;
}

void DeviceIDIntToChar(unsigned int DeviceID ,  char szDeviceID[])
{
	sprintf(szDeviceID, "%7X", (unsigned int)DeviceID);

}
std::string  DeviceIDIntToChar(unsigned int DeviceID)
{
	char szDeviceID[16];
	sprintf(szDeviceID, "%7X", (unsigned int)DeviceID);
	return szDeviceID;
}


//convert divice ID string to long
unsigned int DeviceIdCharToInt(std::string &DeviceID) {
	unsigned int ID;
	std::stringstream s_strid;
	s_strid << std::hex << DeviceID;
	s_strid >> ID;
	return ID;
}

bool CEnOcean::getProfile(std::string szDeviceID, int &Manufacturer, int &Rorg, int &Profile, int &Type)
{
	Rorg = Profile = Type = Manufacturer = 0;

	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query("SELECT  Rorg, Profile, [Type], Manufacturer FROM EnoceanSensors WHERE (HardwareID==%d) AND (DeviceID=='%q')", m_HwdID, szDeviceID.c_str());
	if (result.size() == 1)
	{
		// hardware device was already teached-in
		Rorg         = atoi(result[0][0].c_str());
		Profile      = atoi(result[0][1].c_str());
		Type         = atoi(result[0][2].c_str());
		Manufacturer = atoi(result[0][3].c_str());
		return true;
	}
	return false;
}
bool CEnOcean::getProfile(unsigned int DeviceID, int &Manufacturer, int &Rorg, int &Profile, int &Type)
{
	return getProfile(DeviceIDToString(DeviceID), Manufacturer, Rorg, Profile, Type);
}

//---------------------------------------------------------------------------
// TypFnAToB    : Convertit une chaîne hexadécimale ascii en un tableau binaire
// Input  Arg.  : Chaîne hexadécimale ascii
//                Adresse du tableau
//                Adresse de rangement de la longueur du tableau
// Output       : true  -> Conversion réussie
//                false -> Echec
// Remark       : Le tableau de destination doit être suffisement dimensionné
//                La conversion s'arrête sur tout caractère non hexa
//---------------------------------------------------------------------------

int HexToBin(char c)
{
	int h = (unsigned char)(c - '0');
	if (h>9)
		h = (unsigned char)(h - 7);
	if (h>15)
		h = (unsigned char)(h - 32);
	if (h>15)
		return 0;
	return h;

}
bool TypFnAToB(const char * st, unsigned char bin[], int  *trame_len)
{
	std::string x;
	int  h, l;
	int i = 0;
	int index = 0;
	/* -------------- */
	*trame_len = 0;

	while (st[index] != 0)
	{

		if (st[index] == 'b') {
			index++;
			h = 0;
			while ((st[index] == '0') || (st[index] == '1'))
			{
				h = h << 1;
				if ((st[index] == '1'))
					h = h + 1;
				index++;
			}

			bin[i++] = (unsigned char)(h);
		}

		else
		{
			h = HexToBin(st[index]);

			index++;
			if (st[index] != ' ')
			{

				l = HexToBin(st[index]);
				bin[i++] = (unsigned char)((h << 4) + l);
			}
			else
				bin[i++] = (unsigned char)(h);
			index++;
		}

		if (st[index] == ' ')
			index++;
	}
	*trame_len = i;
	return true;
};



void CEnOcean::parse_PACKET_REMOTE_MAN_COMMAND( unsigned char m_buffer[] , int m_DataSize, int m_OptionalDataSize )
{
{
	//get function
	int fct = m_buffer[0] * 256 + m_buffer[1];
	//ping response
	if (fct == 0x606)
	{
		// ping
		//	55 00 0F 07 01 2B         C5 80 00 7F F0 06 00 00 00 00 00 00 00 00 8F        03 01 A6 54 28 FF 00 83
		//response
		//	55 00 08 0A 07 C6         06 06 07 FF D2 01 12 2D                             05 01 33 BE 01 A6 54 28 2D 00 34
		int profile = m_buffer[4] * 65536 + (int)m_buffer[5] * 256 + (int)m_buffer[6];
		unsigned int senderId = DeviceArrayToInt(&m_buffer[12]);
		addSensorProfile(senderId, profile);

		_log.Log(LOG_NORM, "EnOcean: Ping SenderId: %08X Profile:%06X ", senderId, profile);
	}
	//product id  response
	else if (fct == 0x827)
	{
		//get product id  cmd 227 
		//55 00 0F 07 01 2B         C5 80 00 7F F2 27 00 00 00 00 00 00 00 00 8F        03 FF FF FF FF FF 00             55
		//reponse  manu 46 procuct ref 00010003
		//55 00 0A 0A 07 10         08 27 07 FF 00 46 00 01 00 03                       FF FF FF FF 01 A6 54 28 2C 00     B3
		int manuf = m_buffer[5];
		unsigned int senderId = DeviceArrayToInt(&m_buffer[14]);
		_log.Log(LOG_NORM, "EnOcean: getProductId SenderId: %08X Manufacturer:%s  ", senderId, Get_EnoceanManufacturer(manuf));
		addSensorManuf(senderId, manuf);
		ping(senderId);
	}
	//get link table medatadate cmd 0210 : taille current / max  table
	else if (fct == 0x810)
	{
		//get link table medatadate cmd 0210 : taille current/ max  table
		//55 00 0F 07 01 2B         C5 40 00 7F F2 10 00 00 00 00 00 00 00 00 8F        03 01 A6 54 28 FF 00  AD
		//response curren size 03 max x18=24
		//55 00 09 0A 07 AD         08 10 07 FF 50 00 00 03 18                          FF FF FF FF 01 A6 54 28 2D 00 08
		int currentSize = m_buffer[7];
		int maxSize = m_buffer[8];
		unsigned int senderId = DeviceArrayToInt(&m_buffer[13]);
		_log.Log(LOG_NORM, "EnOcean: getLink table medatadata SenderId: %08X Size:%d Max:%d ", senderId, currentSize, maxSize);
		setLinkTableMedadata(senderId, currentSize, maxSize);
	}
	//get all link table
	else if (fct == 0x811)
	{
		//get all link table
		//55 00 0F 07 01 2B         C5 40 01 FF F2 11 00 00 17 00 00 00 00 00 8F        03 01 A6 54 28 FF 00 56
		//response
		//55 00 20 0A 07 D4         08 11 07 FF  00 00 FF 99 DF 01 D5 00 01  00 01 FF 99 DF 02 F6 02 01   00 02 FF 99 DF 02 F6 02 01 01       FF FF FF FF 01 A6 54 28 2E 00 FB
		//55 00 20 0A 07 D4         08 11 07 FF  00 03 00 00 00 00 FF FF FF  00 04 00 00 00 00 FF FF FF   00 05 00 00 00 00 FF FF FF 00       FF FF FF FF 01 A6 54 28 2E 00 8F
		//55 00 20 0A 07 D4         08 11 07 FF  00 06 00 00 00 00 FF FF FF  00 07 00 00 00 00 FF FF FF   00 08 00 00 00 00 FF FF FF 00       FF FF FF FF 01 A6 54 28 2E 00 DE
		//55 00 20 0A 07 D4         08 11 07 FF  00 09 00 00 00 00 FF FF FF  00 0A 00 00 00 00 FF FF FF   00 0B 00 00 00 00 FF FF FF 00       FF FF FF FF 01 A6 54 28 2E 00 F4
		//55 00 20 0A 07 D4         08 11 07 FF  00 0C 00 00 00 00 FF FF FF  00 0D 00 00 00 00 FF FF FF   00 0E 00 00 00 00 FF FF FF 00       FF FF FF FF 01 A6 54 28 2E 00 E1
		//55 00 20 0A 07 D4         08 11 07 FF  00 0F 00 00 00 00 FF FF FF  00 10 00 00 00 00 FF FF FF   00 11 00 00 00 00 FF FF FF 00       FF FF FF FF 01 A6 54 28 2E 00 BC
		//55 00 20 0A 07 D4         08 11 07 FF  00 15 00 00 00 00 FF FF FF  00 16 00 00 00 00 FF FF FF   00 17 00 00 00 00 FF FF FF 00       FF FF FF FF 01 A6 54 28 2E 00 66

		unsigned int senderId = DeviceArrayToInt(&m_buffer[m_DataSize + 4]);
		int nb = m_DataSize - 5;
		nb /= 9;
		for (int i = 0; i < nb; i++) {

			int  offs = m_buffer[5 + i * 9];
			uint entryId = DeviceArrayToInt(&m_buffer[6 + i * 9]);
			uint entryProfile = DeviceArrayToInt(&m_buffer[10 + i * 9]);
			int  channel = m_buffer[13 + i * 9];
			entryProfile /= 256;
			addLinkTable(senderId, offs, entryProfile, entryId, channel);
			if (entryId > 0)
				_log.Log(LOG_NORM, "EnOcean: SenderId: %08X Link table entry %02d EntryId: %08X Profile %06X Channel:%d", senderId, offs, entryId, entryProfile, channel);

			printSensors();
		}
	}
	//query function
	else if (fct == 0x607)
	{
		//query function
		//55 00 0F 07 01 2B	C5 80 00 7F F0 07 00 00 00 00 00 00 00 00 8F 				03 01 A6 54 28 FF 00     8D  opt 7
		//55 00 34 0A 07 DD 06 07 07 FF 02 24 07 FF 02 27 07 FF 02 20 07 FF 02 10 07 FF 02 11 07 FF 02 12 07 FF 02 30 07 FF 02 31 07 FF 02 32 07 FF 02 33 07 FF 02 26 07 FF 00 00 00 00      FF FF FF FF 01 A6 54 28 2C 00     2E opt 10
		unsigned int senderId = DeviceArrayToInt(&m_buffer[m_DataSize + 4]);
		int nb = m_DataSize - 4;
		nb /= 4;
		for (int i = 0; i < nb; i++) {

			int  function = m_buffer[4 + i * 4] * 256 + m_buffer[5 + i * 4];
			if (function)
				_log.Log(LOG_NORM, "EnOcean: SenderId: %08X Function :%0X  ", senderId, function);
		}

	}
	}

}

//send F6 01 01 : rocker button for teach in
void CEnOcean::SendRpsTeachIn(unsigned int sID)
{
	unsigned char buff[16];

	//F6 10 ff 99 df 01 30
	//F6 00 ff 99 df 01 20

	buff[0] = RORG_RPS;
	buff[1] = 0x10;// rocker 0 press
	buff[2] = (sID >> 24) & 0xff;		// Sender ID
	buff[3] = (sID >> 16) & 0xff;
	buff[4] = (sID >> 8) & 0xff;
	buff[5] = sID & 0xff;
	buff[6] = 0x30;
	sendFrameQueue(PACKET_RADIO, buff, 7, NULL, 0);

	//Next command is send a bit later (button release)
	buff[1] = 0;				// no button press
	buff[6] = S_RPS_T21;	// release button			// b5=T21, b4=NU, b3-b0= RepeaterCount

	sendFrameQueue(PACKET_RADIO, buff, 7, NULL, 0);

}

//send data              1BS D5 00 FF 99 DF 01 00
//opt                                             03 FF FF FF FF FF 00
//esp3     55 00 07 07 01 7A D5 00 FF 99 DF 01 00 03 FF FF FF FF FF 00 64
void CEnOcean::Send1BSTeachIn(unsigned int sID)
{
	unsigned char buff[16];

	buff[0] = RORG_1BS;
	buff[1] = 0;
	buff[2] = (sID >> 24) & 0xff;		// Sender ID
	buff[3] = (sID >> 16) & 0xff;
	buff[4] = (sID >> 8) & 0xff;
	buff[5] = sID & 0xff;
	buff[6] = 0;

	//optionnal data
	unsigned char opt[16];
	opt[0] = 0x03; //subtel
	opt[1] = 0xff;
	opt[2] = 0xff;
	opt[3] = 0xff;
	opt[4] = 0xff;
	opt[5] = 0xff;
	opt[6] = 00;//RSI 

	sendFrameQueue(PACKET_RADIO, buff, 7, opt, 7);

}
// A5 02 00 00 00 FF 99 DF 01 30
void CEnOcean::Send4BSTeachIn(unsigned int sID)
{
	if (isOpen()) {

		unsigned char buf[100];
		buf[0] = RORG_4BS;
		buf[1] = 0x2;
		buf[2] = 0;
		buf[3] = 0;
		buf[4] = 0x0; // DB0.3=0 -> teach in with no EEP

		buf[5] = (sID >> 24) & 0xff;
		buf[6] = (sID >> 16) & 0xff;
		buf[7] = (sID >> 8) & 0xff;
		buf[8] = sID & 0xff;

		buf[9] = 0x30; // status

		sendFrame(PACKET_RADIO, buf, 10, NULL, 0);
	}
}

void CEnOcean::sendVld(unsigned int sID, int channel, int value)
{
	unsigned char buff[16];

	buff[0] = RORG_VLD; //vld
	buff[1] = 0x01;
	buff[2] = channel;
	buff[3] = value;
	buff[4] = (sID >> 24) & 0xff;		// Sender ID
	buff[5] = (sID >> 16) & 0xff;
	buff[6] = (sID >> 8) & 0xff;
	buff[7] = sID & 0xff;
	buff[8] = 0; //status

	//optionnal data
	unsigned char opt[16];
	opt[0] = 0x03; //subtel
	opt[1] = 0xff;
	opt[2] = 0xff;
	opt[3] = 0xff;
	opt[4] = 0xff;
	opt[5] = 0xff;
	opt[6] = 00;//RSI 

	//D2 01 00 00 FF 99 DF 01 00
	//03 FF FF FF FF FF 00

	sendFrameQueue(PACKET_RADIO, buff, 9, opt, 7);
}

//send a VLD datagramm with payload : data to device Id sID
void CEnOcean::sendVld(unsigned int sID, unsigned char *data , int DataLen )
{
	unsigned char buffer[64];

	if (DataLen > MAX_DATA_PAYLOAD)
		return;

	unsigned char *buff = buffer ;
	*buff++ = RORG_VLD; //vld
	for (int i = 0; i < DataLen; i++)
		*buff++ = *data++;

	*buff++ = (sID >> 24) & 0xff;		// Sender ID
	*buff++ = (sID >> 16) & 0xff;
	*buff++ = (sID >> 8) & 0xff;
	*buff++ = sID & 0xff;
	*buff++ = 0; //status

				 //optionnal data
	unsigned char opt[16];
	opt[0] = 0x03; //subtel
	opt[1] = 0xff;
	opt[2] = 0xff;
	opt[3] = 0xff;
	opt[4] = 0xff;
	opt[5] = 0xff;
	opt[6] = 00;//RSI 

				//D2 01 00 00 FF 99 DF 01 00
				//03 FF FF FF FF FF 00

	sendFrameQueue(PACKET_RADIO, buffer, 6+DataLen, opt, 7);
}

extern uint32_t SetRawValues(uint8_t * data, T_DATAFIELD * OffsetDes, va_list value);

//send a VLD datagramm of the command described by descriptor OffsetDes detailed in EEP profile.to device Id sID

// the argument are variable length
//it shall correspond to each offset Data detailed in EnOcean_Equipment_Profiles_EEP_v2.x.x_public.
//the list  parameter shall end with value END_ARG_DATA
//return is the size of the daya payload in byte 
// or 0 : if an error occured : not enough parameters passed
//example :
//sendVld(unitBaseAddr, D20500_CMD_2, channel, 2, END_ARG_DATA);
// send a stop command = 2 to channel 9 for EEP : D2-05-00 ; Blinds control

//sendVld(unitBaseAddr, D20500_CMD_1, 100, 127, 0, 0, 0 , 1, END_ARG_DATA);
// send a got position and angle command = 1 to 
// POS=100 % 
// ANG=127 : dont change 
// REPO=0 goto ditectly 
// LOCK=0  dont change  , 
// channel 0
// CMD = 1 goto command for EEP : D2-05-00 ; Blinds control


uint32_t CEnOcean::sendVld(unsigned int unitBaseAddr, T_DATAFIELD * OffsetDes,  ...)
{
	uint8_t  data[MAX_DATA_PAYLOAD+2];
	va_list value;

	/* Initialize the va_list structure */
	va_start(value, OffsetDes);

	memset(data, 0, sizeof(data));

	uint32_t DataSize = SetRawValues(data, OffsetDes,  value);
	if (DataSize)
		sendVld(unitBaseAddr, data, DataSize);

	va_end(value);

	return  DataSize;

}


void CEnOcean::setRorg(unsigned char * buff)
{
	buff[0] = RORG_SYS_EX;

	m_Seq++;
	if (m_Seq > 3) m_Seq = 1;
	buff[1] = m_Seq << 6;       //SEQ 40/80/C0

}

void setDestination(unsigned char * opt, unsigned int destID)
{
	//optionnal data
	opt[0] = 0x03; //subtel
	DeviceIntToArray(destID, &opt[1]);
	opt[5] = 0xff;
	opt[6] = 00;//RSI 

}
void CEnOcean::remoteLearning(unsigned int destID, bool StartLearning, int channel)
{
	unsigned char buff[16];
	unsigned char opt[16];

	memset(buff, 0, sizeof(buff));
	setRorg(buff);

	buff[2] = 0x01;			//data len = 2
	buff[3] = 0x7F;		//mamanufacturer 7FF
	buff[4] = 0xF2;			//function 220
	buff[5] = 0x20;

	//payload 4 bytes
	if (StartLearning)buff[6] = 0; else buff[6] = 0x80;
	buff[7] = channel;

	buff[14] = 0x8F; //status

	//optionnal data
	setDestination(opt, destID);

	_log.Log(LOG_TRACE, "EnOcean: send remoteLearning");
	sendFrameQueue(PACKET_RADIO, buff, 15, opt, 7);
}

void CEnOcean::unlock(unsigned int destID, unsigned int code)
{
	unsigned char buff[16];
	unsigned char opt[16];

	memset(buff, 0, sizeof(buff));
	setRorg(buff);

	buff[2] = 0x02;			//data len = 4
	buff[3] = 0x7F;			//mamanufacturer 7FF
	buff[4] = 0xF0;
	buff[5] = 0x01;			//function 001
	buff[14] = 0x8F; //status

	DeviceIntToArray(code, &buff[6]);

	//optionnal data
	setDestination(opt, destID);

	_log.Log(LOG_TRACE, "EnOcean: send unlock");
	sendFrameQueue(PACKET_RADIO, buff, 15, opt, 7);
}

void CEnOcean::lock(unsigned int destID, unsigned int code)
{
	unsigned char buff[16];
	unsigned char opt[16];

	memset(buff, 0, sizeof(buff));
	setRorg(buff);

	buff[2] = 0x02;			//data len = 4
	buff[3] = 0x7F;			//mamanufacturer 7FF
	buff[4] = 0xF0;
	buff[5] = 0x02;			//function 002
	buff[14] = 0x8F; //status

	DeviceIntToArray(code, &buff[6]);

	//optionnal data
	setDestination(opt, destID);

	_log.Log(LOG_TRACE, "EnOcean: send lock");
	sendFrameQueue(PACKET_RADIO, buff, 15, opt, 7);

}

void CEnOcean::setcode(unsigned int destID, unsigned int code)
{
	unsigned char buff[16];
	unsigned char opt[16];

	memset(buff, 0, sizeof(buff));
	setRorg(buff);

	buff[2] = 0x02;			//data len = 4
	buff[3] = 0x7F;			//mamanufacturer 7FF
	buff[4] = 0xF0;
	buff[5] = 0x03;			//function 003
	buff[14] = 0x8F; //status

	DeviceIntToArray(code, &buff[6]);

	//optionnal data
	setDestination(opt, destID);

	_log.Log(LOG_TRACE, "EnOcean: send setcode %08X , %d", destID, code);
	sendFrameQueue(PACKET_RADIO, buff, 15, opt, 7);

}

void CEnOcean::ping(unsigned int destID)
{
	unsigned char buff[16];
	unsigned char opt[16];

	memset(buff, 0, sizeof(buff));
	setRorg(buff);

	buff[2] = 0x00;			//data len = 0
	buff[3] = 0x7F;			//mamanufacturer 7FF
	buff[4] = 0xF0;
	buff[5] = 0x06;			//function 006
	buff[14] = 0x8F; //status

	//optionnal data
	setDestination(opt, destID);

	_log.Log(LOG_TRACE, "EnOcean: send ping %08X ", destID);
	sendFrameQueue(PACKET_RADIO, buff, 15, opt, 7);

}

void CEnOcean::action(unsigned int destID)
{
	unsigned char buff[16];
	unsigned char opt[16];

	memset(buff, 0, sizeof(buff));
	setRorg(buff);

	buff[2] = 0x00;			//data len = 0
	buff[3] = 0x7F;			//mamanufacturer 7FF
	buff[4] = 0xF0;
	buff[5] = 0x05;			//function 005
	buff[14] = 0x8F;		//status

					 //optionnal data
	setDestination(opt, destID);

	sendFrameQueue(PACKET_RADIO, buff, 15, opt, 7);
	_log.Log(LOG_TRACE, "EnOcean: send action %08X ", destID);

}

void CEnOcean::getProductId()
{
	unsigned char buff[16];
	unsigned char opt[16];

	memset(buff, 0, sizeof(buff));
	setRorg(buff);

	buff[2] = 0x00;			//data len = 0
	buff[3] = 0x7F;			//mamanufacturer 7FF
	buff[4] = 0xF2;
	buff[5] = 0x27;			//function 227
	buff[14] = 0x8F;		//status

							//optionnal data
	setDestination(opt, 0xFFFFFFFF);

	_log.Log(LOG_TRACE, "EnOcean: send getProductId");
	sendFrameQueue(PACKET_RADIO, buff, 15, opt, 7);

}

void CEnOcean::addSensorManuf(uint SensorId, uint Manuf)
{
	m_sensors[SensorId].Manufacturer = Manuf;
}

void CEnOcean::addSensorProfile(uint SensorId, uint Profile)
{
	m_sensors[SensorId].Profile = Profile;
}

void CEnOcean::getLinkTableMedadata(uint destID)
{
	unsigned char buff[16];
	unsigned char opt[16];

	memset(buff, 0, sizeof(buff));
	setRorg(buff);

	buff[2] = 0x00;			//data len = 0
	buff[3] = 0x7F;			//mamanufacturer 7FF
	buff[4] = 0xF2;
	buff[5] = 0x10;			//function 210
	buff[14] = 0x8F; //status

					 //optionnal data
	setDestination(opt, destID);

	_log.Log(LOG_TRACE, "EnOcean: send getLinkTableMedadata %08X ", destID);
	sendFrameQueue(PACKET_RADIO, buff, 15, opt, 7);

}

void CEnOcean::setLinkTableMedadata(uint SensorId, int csize, int MaxSize)
{
	m_sensors[SensorId].CurrentSize = csize;
	m_sensors[SensorId].MaxSize = MaxSize;
}

void CEnOcean::getProductFunction(uint destID)
{
	unsigned char buff[16];
	unsigned char opt[16];
	//C5 80 00 7F F0 07 00 00 00 00 00 00 00 00 8F

	memset(buff, 0, sizeof(buff));
	setRorg(buff);

	buff[2] = 0x00;			//data len = 0
	buff[3] = 0x7F;			//mamanufacturer 7FF
	buff[4] = 0xF0;
	buff[5] = 0x07;			//function 007
	buff[14] = 0x8F; //status

					 //optionnal data
	setDestination(opt, destID);

	_log.Log(LOG_TRACE, "EnOcean: send getProductFunction %08X ", destID);
	sendFrameQueue(PACKET_RADIO, buff, 15, opt, 7);

}


void CEnOcean::getallLinkTable(uint SensorId, int begin, int end)
{
	unsigned char buff[16];
	unsigned char opt[16];

	memset(buff, 0, sizeof(buff));
	setRorg(buff);

	buff[2] = 0x01;			//data len = 3
	buff[3] = 0xFF;			//mamanufacturer 7FF
	buff[4] = 0xF2;
	buff[5] = 0x11;			//function 211

	buff[7] = begin;		//end offset table 
	buff[8] = end;		//end offset table 

	buff[14] = 0x8F; //status

					 //optionnal data
	setDestination(opt, SensorId);

	_log.Log(LOG_TRACE, "EnOcean: send getallLinkTable %08X ", SensorId);
	sendFrameQueue(PACKET_RADIO, buff, 15, opt, 7);

}

void CEnOcean::addLinkTable(uint DeviceId, int entry, int profile, uint sensorId, int channel)
{
	if (entry < SIZE_LINK_TABLE) {
		m_sensors[DeviceId].LinkTable[entry].Profile = profile;
		m_sensors[DeviceId].LinkTable[entry].SenderId = sensorId;
		m_sensors[DeviceId].LinkTable[entry].Channel = channel;
	}

}

void CEnOcean::printSensors()
{
	for (T_SENSOR_MAP::iterator itt = m_sensors.begin(); itt != m_sensors.end(); itt++)
	{

		_log.Log(LOG_NORM, "DeviceId:%08X  Profile:%0X Manufacturer:%d CurrentSize:%d MaxSize:%d", itt->first, itt->second.Profile, itt->second.Manufacturer, itt->second.CurrentSize, itt->second.MaxSize);
		for (int i = 0; i < itt->second.CurrentSize; i++)
			_log.Log(LOG_NORM, "                  Entry:%d Id:%08X Profile:%X Channel:%d", i, itt->second.LinkTable[i].SenderId, itt->second.LinkTable[i].Profile, itt->second.LinkTable[i].Channel);

	}

}


void CEnOcean::TeachIn(std::string& sidx)
{
	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query("SELECT DeviceID,Unit  FROM DeviceStatus WHERE (ID='%s')  ", sidx.c_str());
	if (result.size() > 0)
	{
		std::string deviceId = result[0][0];

		int channel = atoi(result[0][1].c_str());
		//get sender adress from db
		unsigned int SenderAdress = DeviceIdCharToInt(deviceId);

		_log.Log(LOG_NORM, "EnOcean: send remoteLearning to device %s channel:%d", deviceId.c_str(), channel);

		unlock(SenderAdress, 1);
		remoteLearning(SenderAdress, true, channel - 1);

	}

}

std::string IntToString(int val, int nbDigit)
{
	char fmt[16];
	char intStr[32];
	sprintf(fmt, "%%0%dX", nbDigit);
	sprintf(intStr, fmt, val);
	return intStr;
}

void CEnOcean::GetNodeList(http::server::WebEmSession & session, const http::server::request& req, Json::Value &root)
{
	int nbParam = req.parameters.size() - 3;
	root["status"] = "OK";
	root["title"] = "EnOceanNodes";

	std::vector<std::vector<std::string> > result;
	result = m_sql.safe_query("SELECT D.Name, D.Type, d.SubType, d.SwitchType, d.Unit, E.DeviceId, E.Rorg, E.Profile, E.Type, E.Manufacturer, E.Address FROM DeviceStatus as d LEFT OUTER JOIN EnoceanSensors as e ON(instr(E.DeviceID, D.DeviceId) <> 0)  WHERE (D.HardwareID==%d) ", m_HwdID);

	if (result.size() > 0)
	{
		std::vector<std::vector<std::string> >::const_iterator itt;
		int ii = 0;
		for (itt = result.begin(); itt != result.end(); ++itt)
		{
			std::vector<std::string> sd = *itt;

			//					unsigned int homeID = boost::lexical_cast<unsigned int>(sd[1]);
			{

				root["result"][ii]["Name"] = sd[00];
				root["result"][ii]["Type"] = sd[01];
				root["result"][ii]["SubType"] = sd[02];
				root["result"][ii]["SwitchType"] = sd[03];
				root["result"][ii]["TypeName"] = RFX_Type_SubType_Desc(atoi(sd[01].c_str()), atoi(sd[02].c_str()));
				root["result"][ii]["Unit"] = sd[04];
				root["result"][ii]["DeviceID"] = sd[05];
				int rorg = atoi(sd[6].c_str());
				int func = atoi(sd[7].c_str());
				int type = atoi(sd[8].c_str());

				root["result"][ii]["Profile"] = IntToString(rorg, 2) + "-" + IntToString(func, 2) + "-" + IntToString(type, 2);
				root["result"][ii]["Manufacturer"] = sd[9];
				root["result"][ii]["Manufacturer_name"] = Get_EnoceanManufacturer(atoi(sd[9].c_str()));
				
				root["result"][ii]["BaseAddress"] = GetAdress(atoi(sd[10].c_str()));
				root["result"][ii]["EnoTypeName"] = Get_Enocean4BSType(rorg, func, type);


				char szDate[80] = "";
				//struct tm loctime;
				//						localtime_r(&pNode->LastSeen, &loctime);
				//strftime(szDate, 80, "%Y-%m-%d %X", &loctime);

				root["result"][ii]["LastUpdate"] = szDate;

				ii++;
			}
		}
	}
}



std::string string_format(const char * fmt, ...) {
	va_list ap;
	char buf[1024];
	va_start(ap, fmt);
	int n = vsnprintf((char *)buf, sizeof(buf), fmt, ap);

	return buf;
}

void CEnOcean::GetLinkTable(http::server::WebEmSession & session, const http::server::request& req, Json::Value &root)
{
	int nbParam = req.parameters.size() - 3;
	root["status"] = "OK";
	root["title"] = "EnOceanLinkTable";
	std::string DeviceIds = http::server::request::findValue(&req, "sensorid");
	unsigned int  DeviceId = DeviceIdCharToInt(DeviceIds);

	addLinkTable(0x1a65428, 0, 0xD0500, 0xABCDEF, 1);

	{
		for (int entry = 0; entry < SIZE_LINK_TABLE; entry++)
		{
				root["result"][entry]["Profile"]  = string_format("%06X", m_sensors[DeviceId].LinkTable[entry].Profile) ;
				root["result"][entry]["SenderId"] = string_format("%7X", m_sensors[DeviceId].LinkTable[entry].SenderId);
				root["result"][entry]["Channel"] = string_format("%d", m_sensors[DeviceId].LinkTable[entry].Channel);
			
		}
	}
}




void CEnOcean::SetCode(http::server::WebEmSession & session, const http::server::request & req, Json::Value & root)
{
	for (unsigned int i = 0; i < req.parameters.size() - 3; i++) {
		setcode(DeviceIdCharToInt(http::server::request::findValue(&req, std::to_string(i).c_str())), 1);
	}

}


std::string GetLighting2StringId(unsigned int id )
{
	char szTmp[300];

	sprintf(szTmp, "%7X", id );
	return szTmp;
}

//return true if adress is in range 
bool CEnOcean::CheckIsGatewayAdress(unsigned int deviceid)
{
	if ((deviceid > m_id_base) && (deviceid < m_id_base + MAX_BASE_ADDRESS))
		return true;
	else
		return false;

}

